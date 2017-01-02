#pragma once
#include <atomic>

enum class backingLog {vector, list};

template<typename T,backingLog>
class InfiniteLog;

template<typename T>
class InfiniteLog<T,backingLog::list> {
	struct cons_cell {
		T elem;
		std::unique_ptr<cons_cell> prev;
	};

	using cons_p = std::unique_ptr<cons_cell>;

	std::atomic<cons_p> current;

	T& append(T elem){
		cons_p _current;
		while (!_current){
			_current = current.exchange(cons_p{});
			if (_current){
				current.store(cons_p{new cons_cell(std::move(elem),std::move(_current))});
				break;
			}
		}
	}
};

template<typename T>
class InfiniteLog<T,backingLog::vector> {

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
