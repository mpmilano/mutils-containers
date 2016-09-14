#pragma once
#include <array>
#include <cassert>
#include "string.h"

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
			BufferGenerator &gen;
		public:
			char* payload;
		private:
			char* payload_end{payload};
			bool has_split{false};

			pointer(BufferGenerator& gen, allocated_chunk *parent, char* payload);
			pointer(const pointer&) = delete;
		public:
			pointer(pointer&& o);
			~pointer();

			/**
			   Extend the size this pointer referrs to, creating a new
			   underlying buffer if necessary.  Basically realloc()
			 */
			pointer grow(std::size_t offset);
			
			std::size_t size() const;

			/**
			   create a new pointer pointing to the end of this allocated region.
			 */
			pointer split();

			/**
			   split this allocated region in two at point offset
			 */
			pointer split(std::size_t offset);
			friend class BufferGenerator;
		};

		/**
		   Return a pointer in which no space is used, 
		   which is backed by a newly-allocated buffer
		 */
		pointer allocate();
		
	};
	
	//implementations follow
	
	template<std::size_t buf_size>
	BufferGenerator<buf_size>::pointer::pointer(BufferGenerator& gen, allocated_chunk *parent, char* payload)
		:parent(parent),gen(gen),payload(payload)
	{++parent->use_count;}
	
	template<std::size_t buf_size>
	BufferGenerator<buf_size>::pointer::pointer(pointer&& o)
		:parent(o.parent),
		 gen(o.gen),
		 payload(o.payload),
		 payload_end(o.payload_end),
		 has_split(o.has_split){
		o.parent = nullptr;
		o.payload = nullptr;
	}
	
	template<std::size_t buf_size>
	BufferGenerator<buf_size>::pointer::~pointer(){
		if (parent && (--parent->use_count == 0)){
			delete parent;
		}
	}
	
	template<std::size_t buf_size>
	typename BufferGenerator<buf_size>::pointer BufferGenerator<buf_size>::pointer::grow(std::size_t offset){
		assert(offset + size() <= buf_size);
		if (!has_split &&
			//there's enough space left in this buffer
			(long)offset <= (parent->data.data() + buf_size - payload_end)){
			payload_end += offset;
			return std::move(*this);
		}
		else {
			auto ret = gen.allocate();
			memcpy(ret.payload,payload,size());
			ret.payload_end = ret.payload + size();
			return ret;
		}
	}
	template<std::size_t buf_size>
	std::size_t BufferGenerator<buf_size>::pointer::size() const {
		return payload_end - payload;
	}

	template<std::size_t buf_size>
	typename BufferGenerator<buf_size>::pointer BufferGenerator<buf_size>::pointer::split(){
		has_split = true;
		return pointer{gen,parent,payload_end};
	}

	template<std::size_t buf_size>
	typename BufferGenerator<buf_size>::pointer BufferGenerator<buf_size>::pointer::split(std::size_t offset){
		assert(offset < size());
		auto ret = pointer{gen,parent,payload+offset};
		ret.payload_end = payload_end;
		ret.has_split = has_split;
		payload_end = payload + offset;
		has_split = true;
		return ret;
	}

	template<std::size_t buf_size>
	typename BufferGenerator<buf_size>::pointer BufferGenerator<buf_size>::allocate(){
		auto* new_chunk = new allocated_chunk();
		return pointer{*this,new_chunk,new_chunk->data.data()};
	}

}
