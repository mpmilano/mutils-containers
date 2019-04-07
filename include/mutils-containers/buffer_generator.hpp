#pragma once
#include <array>
#include <cassert>
#include <atomic>
#include <string.h>
#include <mutex>

#ifndef NDEBUG
#include <iostream>
#endif

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
			allocated_chunk *backing_buffer;
		public:
			//invariant: payload is within the allocated chunk
			char* payload;
		private:
			char* payload_end;

			pointer(allocated_chunk *backing_buffer, char* payload, char* payload_end);
			pointer(const pointer&) = delete;
		public:
			pointer(pointer&& o);
			pointer& operator=(pointer&& o);
			~pointer();

			//This is a destructive operation!
			pointer grow();

			//This is a destructive operation!
			pointer grow_to_fit(std::size_t new_size){
				if (size() < new_size) {
					auto ret = grow();
					assert(!payload);
					assert(ret.size() >= new_size);
					return ret;
				}
				else return std::move(*this);
			}

			//payload_end - payload
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
	BufferGenerator<buf_size>::pointer::pointer(allocated_chunk *backing_buffer, char* payload, char* payload_end)
		:backing_buffer(backing_buffer),payload(payload),payload_end(payload_end)
	{
		++backing_buffer->use_count;
	}
	
	template<std::size_t buf_size>
	BufferGenerator<buf_size>::pointer::pointer(pointer&& o)
		:backing_buffer(o.backing_buffer),
		 payload(o.payload),
		 payload_end(o.payload_end)
	{
		o.backing_buffer = nullptr;
		o.payload = nullptr;
		o.payload_end = nullptr;
	}

	template<std::size_t buf_size>
	typename BufferGenerator<buf_size>::pointer&
	BufferGenerator<buf_size>::pointer::operator=(pointer&& o)
	{
		backing_buffer = o.backing_buffer;
		o.backing_buffer = nullptr;
		payload = o.payload;
		o.payload = nullptr;
		payload_end = o.payload_end;
		o.payload_end = nullptr;
		return *this;
	}
	
	template<std::size_t buf_size>
	BufferGenerator<buf_size>::pointer::~pointer(){
		if (backing_buffer){
			//We don't need to acquire a lock;
			//the only way to increase the use count
			//is via a call to split(), which can only
			//be called on a valid object; that valid object
			//will keep the use_count at at least 1.
			//so if reach 0, we're deleting the last pointer
			//off of which we could call split.
			if (--backing_buffer->use_count == 0){
				delete backing_buffer;
			}
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
#ifndef NDEBUG
		if (offset > size()){
			std::cout << offset << std::endl;
			std::cout << size() << std::endl;
		}
#endif
		assert(offset <= size());
		auto ret = pointer{backing_buffer,payload+offset,payload_end};
		payload_end = payload + offset;
		assert(payload_end - payload <= (int)buf_size);
		return ret;
	}

	template<std::size_t buf_size>
	typename BufferGenerator<buf_size>::pointer BufferGenerator<buf_size>::allocate(){
		auto* new_chunk = new allocated_chunk();
		return pointer{new_chunk,new_chunk->data.data(),new_chunk->data.data() + buf_size};
	}

}
