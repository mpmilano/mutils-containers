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

		using lock = std::unique_lock<std::mutex>;

		/**
		   the backing struct, and a reference count.  pointers have 
		   many references to these.
		 */
		struct allocated_chunk{
			std::mutex chunk_lock;
			std::array<char,buf_size> data;
			std::atomic<std::size_t> use_count{0};
		};
		
		/**
		   represents a pointer into an allocated chunk.  is a smart pointer (so no manual memory management).
		   is not copyable.  Initially empty (size() == 0)
		 */
		struct pointer{
		private:
			allocated_chunk *parent;
		public:
			char* payload;
		private:
			char* payload_end;
			bool has_split{false};

			pointer(allocated_chunk *parent, char* payload, char* payload_end);
			pointer(const pointer&) = delete;
		public:
			pointer(pointer&& o);
			pointer& operator=(pointer&& o);
			~pointer();


			pointer grow();
			
			pointer grow_to_fit(std::size_t new_size){
				if (size() < new_size) {
					auto ret = grow();
					assert(!payload);
					assert(ret.size() >= new_size);
					return ret;
				}
				else return std::move(*this);
			}
			
			std::size_t size() const;


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
		static pointer allocate();
		
	};
	
	//implementations follow
	
	template<std::size_t buf_size>
	BufferGenerator<buf_size>::pointer::pointer(allocated_chunk *parent, char* payload, char* payload_end)
		:parent(parent),payload(payload),payload_end(payload_end)
	{
		++parent->use_count;
	}
	
	template<std::size_t buf_size>
	BufferGenerator<buf_size>::pointer::pointer(pointer&& o)
		:parent(o.parent),
		 payload(o.payload),
		 payload_end(o.payload_end),
		 has_split(o.has_split){
		o.parent = nullptr;
		o.payload = nullptr;
	}

	template<std::size_t buf_size>
	typename BufferGenerator<buf_size>::pointer&
	BufferGenerator<buf_size>::pointer::operator=(pointer&& o)
	{
		parent = o.parent;
		o.parent = nullptr;
		payload = o.payload;
		o.payload = nullptr;
		payload_end = o.payload_end;
		o.payload_end = nullptr;
		has_split = o.has_split;
		return *this;
	}
	
	template<std::size_t buf_size>
	BufferGenerator<buf_size>::pointer::~pointer(){
		if (parent && (--parent->use_count == 0)){
			delete parent;
		}
	}
	
	template<std::size_t buf_size>
	typename BufferGenerator<buf_size>::pointer BufferGenerator<buf_size>::pointer::grow(){
		auto ret = allocate();
		memcpy(ret.payload,payload,size());
		ret.payload_end = ret.payload + buf_size;
		pointer{std::move(*this)}; // to invalidate this pointer
		assert(!payload);
		return ret;
	}
	template<std::size_t buf_size>
	std::size_t BufferGenerator<buf_size>::pointer::size() const {
		assert(payload_end - payload <= (int)buf_size);
		assert(payload_end);
		assert(payload);
		return payload_end - payload;
	}

	template<std::size_t buf_size>
	typename BufferGenerator<buf_size>::pointer BufferGenerator<buf_size>::pointer::split(std::size_t offset){
		assert(payload_end - payload <= (int)buf_size);
		assert(offset < size());
		auto ret = pointer{parent,payload+offset,payload_end};
		ret.has_split = has_split;
		payload_end = payload + offset;
		has_split = true;
		assert(payload_end - payload <= (int)buf_size);
		return ret;
	}

	template<std::size_t buf_size>
	typename BufferGenerator<buf_size>::pointer BufferGenerator<buf_size>::allocate(){
		auto* new_chunk = new allocated_chunk();
		return pointer{new_chunk,new_chunk->data.data(),new_chunk->data.data() + buf_size};
	}

}
