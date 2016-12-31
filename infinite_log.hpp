#pragma once
#include <atomic>

template<typename T>
class InfiniteLog {

	using T_p = std::unique_ptr<T>;
	
	struct Log{
		const T_p data[];
		std::atomic<T_p*> current;
		const std::size_t size;
		Log(std::size_t size)
			:data(new T_p[size]),
			 current(data),
			 size(size)
			{
				assert(size > 0);
			}
		
		Log(std::size_t old_size, T_p* old_begin)
			:Log(old_size*2)
			{
				memcpy(current,old_begin,old_size);
				current += old_size;
			}
	};
	
	std::atomic<std::unique_ptr<Log> > inner_log;
	
	
public:	
	T& last(std::size_t n = 0) const {
		const T_p& candidate = *(inner_log.load().current.load() - n);
		assert(candidate);
		return *candidate;
	}
	
	T& append() {
		if (full()){
			bool b = false;
			while (full() && !b){
				auto &inner_p = inner_log.load();
				auto *inner = *inner_p;
				std::unique_ptr new_log{new Log(inner.size,inner.data)};
				b = inner_log.compare_exchange_strong(inner_p,new_log);
			}
		}
		//do a normal append
	}
	
};
