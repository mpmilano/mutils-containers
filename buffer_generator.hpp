#pragma once

namespace mutils{

	//Idea: BufferGenerator is used to allocate large,
	//fixed-size buffers into which many pointers deliniate
	//separate, contiguous regions.
	template<std::size_t buf_size>
	class BufferGenerator{
	public:

		/**
		   the backing struct, and a reference count.  pointers have 
		   many references to these.
		 */
		struct allocated_chunk{
			std::array<char,buf_size> data;
			std::size_t use_count{0};
		};
		
		/**
		   represents a pointer into an allocated chunk.  is a smart pointer (so no manual memory management).
		   is not copyable.  Initially empty (size() == 0)
		 */
		struct pointer{
		private:
			allocated_chunk *parent;
			char* payload;
			char* payload_end{payload};
			bool split{false};

			pointer(allocated_chunk *parent, char* payload);
			pointer(const pointer&) = delete;
			pointer(pointer&& o);
		public:
			~pointer();

			/**
			   Extend the size this pointer referrs to, creating a new
			   underlying buffer if necessary.  Basically realloc()
			 */
			pointer grow(int offset);
			
			std::size_t size() const;

			/**
			   create a new pointer pointing to the end of this allocated region.
			 */
			pointer split();

			/**
			   split this allocated region in two at point offset
			 */
			pointer split(int offset);
		};

		/**
		   Return a pointer in which no space is used, 
		   which is backed by a newly-allocated buffer
		 */
		pointer allocate();
		
	};
	
	//implementations follow
	
	template<std::size_t buf_size>
	BufferGenerator::pointer::pointer(allocated_chunk *parent, char* payload)
		:parent(parent),payload(payload)
	{++parent->use_count;}
	
	template<std::size_t buf_size>
	BufferGenerator::pointer::pointer(pointer&& o)
		:parent(o.parent),
		 payload(o.payload){
		o.parent = nullptr;
		o.payload = nullptr;
	}
	
	template<std::size_t buf_size>
	BufferGenerator::pointer::~pointer(){
		if (parent && --parent->use_count){
			delete &parent;
		}
	}
	
	template<std::size_t buf_size>
	BufferGenerator::pointer BufferGenerator::pointer::grow(int offset){
		assert(offset + size() < buf_size);
		if (!split &&
			//there's enough space left in this buffer
			offset < (parent->data.data() + buf_size - payload_end)){
			payload_end += offset;
			return std::move(*this);
		}
		else {
			auto ret = allocate();
			memcpy(ret.payload,payload,size());
			ret.payload_end = ret.payload + size();
			return ret;
		}
	}
	template<std::size_t buf_size>
	std::size_t BufferGenerator::pointer::size() const {
		return payload_end - payload;
	}

	template<std::size_t buf_size>
	BufferGenerator::pointer BufferGenerator::pointer::split(){
		split = true;
		return pointer{parent,payload_end};
	}

	template<std::size_t buf_size>
	BufferGenerator::pointer BufferGenerator::pointer::split(int offset){
		assert(offset < size());
		auto ret = pointer{parent,payload+offset};
		ret.payload_end = payload_end;
		ret.split = split;
		payload_end = payload + offset;
		split = true;
		return ret;
	}

	template<std::size_t buf_size>
	BufferGenerator::pointer BufferGenerator::allocate(){
		auto* new_chunk = new allocated_chunk();
		return pointer{*new_chunk,new_chunk->data.data()};
	}

}
