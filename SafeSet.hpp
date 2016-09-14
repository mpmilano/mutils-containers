#pragma once
#include <set>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>

namespace mutils{

	template<typename T>
	constexpr void discard(const T&){}

	template<typename Set, typename... Args>
	auto normalized_emplace(Set &s, Args && ... args)
		-> decltype(s.emplace(std::forward<Args>(args)...))
	{
		return s.emplace(std::forward<Args>(args)...);
	}

	template<typename List, typename... Args>
	auto normalized_emplace(List &l, Args && ... args)
		-> decltype(l.emplace_back(std::forward<Args>(args)...))
	{
		return l.emplace_back(std::forward<Args>(args)...);
	}

	template<typename Set, typename... Args>
	auto normalized_insert(Set &s, Args && ... args)
		-> decltype(s.insert(std::forward<Args>(args)...))
	{
		return s.insert(std::forward<Args>(args)...);
	}

	template<typename List, typename... Args>
	auto normalized_insert(List &l, Args && ... args)
		-> decltype(l.push_back(std::forward<Args>(args)...))
	{
		return l.push_back(std::forward<Args>(args)...);
	}

	template<typename List, typename... Args>
	auto normalized_insert_front(List &l, Args && ... args)
		-> decltype(l.push_front(std::forward<Args>(args)...)){
		return l.push_front(std::forward<Args>(args)...);
	}

	template<typename Set, typename... Args>
	auto normalized_insert_front(Set &s, Args && ... args)
		-> decltype(s.insert(std::forward<Args>(args)...)){
		return normalized_insert(s,std::forward<Args>(args)...);
	}
	
	template<typename T,
			 class Collection = std::list<T> >
	struct MonotoneSafeSet {
		static_assert(std::is_same<typename Collection::value_type,T>::value,
					  "Error: Collection is not a collection of T.");

		const std::size_t max_size;
		
		MonotoneSafeSet(std::size_t max_size):
			max_size(max_size){}
		
		Collection impl;
		std::mutex m;
		std::condition_variable cv;
		using lock = std::unique_lock<std::mutex>;
		template<typename... Args>
		T* emplace(Args && ... args){
			struct at_scope_end{
				std::condition_variable &cv;
				~at_scope_end(){ cv.notify_all();}
			};
			at_scope_end ase{cv};
			{
				lock l{m}; discard(l);
				if (size() > max_size) return nullptr;
				impl.emplace_back(std::forward<Args>(args)...);
				auto ret = &impl.back();
				return ret;
			}
		}

		bool empty() const{
			return impl.empty();
		}

		auto size() const{
			return impl.size();
		}

		bool add(T t){
			{
				if (size() > max_size) return false;
				lock l{m}; discard(l);
				normalized_insert(impl,t);
			}
			cv.notify_all();
			return true;
		}

		bool add_front(T t){
			{
				if (size() > max_size) return false;
				lock l{m}; discard(l);
				normalized_insert_front(impl,t);
			}
			cv.notify_all();
			return true;
		}
		Collection iterable_copy(){
			return impl;
		}

		Collection& iterable_reference(){
			return impl;
		}

		void remove(const T &t){
			lock l{m}; discard(l);
			impl.erase(std::find(impl.begin(), impl.end(),t));
		}
	
	};

	template<typename T,
			 class Collection>
	struct InterimSafeSet : public MonotoneSafeSet<T,Collection>{

		InterimSafeSet(std::size_t max_size)
			:MonotoneSafeSet<T,Collection>(max_size){}

		struct EmptyException : public std::exception{
			const char* what() noexcept {
				return "empty safe set encountered";
			}
		};
		using lock = typename MonotoneSafeSet<T>::lock;

	protected:
		auto pop_common(){
			auto r = std::move(*this->impl.begin());
			this->impl.erase(this->impl.begin());
			return r;
		}
	public:

		template<typename... Args>
		T emplace_or_pop(Args && ... args){
			lock l{this->m}; discard(l);
			if (this->impl.size() == 0)
				normalized_emplace(this->impl,std::forward<Args>(args)...);
			return pop_common();
		}

		template<typename... Args>
		T build_or_pop(const std::function<Args ()>& ... args){
			lock l{this->m}; discard(l);
			if (this->impl.size() == 0)
				normalized_emplace(this->impl,args()...);
			return pop_common();
		}
	};

	template<typename T, class Collection = std::list<T> >
	struct SafeSet : public InterimSafeSet<T,Collection> {

		SafeSet(std::size_t max_size = std::numeric_limits<std::size_t>::max())
			:InterimSafeSet<T,Collection>(max_size){}
		
		using lock = typename MonotoneSafeSet<T>::lock;
		T pop(){
			lock l{this->m}; discard(l);
			if (this->impl.size() == 0) {
				this->cv.wait(l,[&](){return this->impl.size() > 0;});
			}
			return this->pop_common();
		}
	};

	template<typename T, class Collection>
	struct SafeSet<T*,Collection> : public InterimSafeSet<T*,Collection>{
		static_assert(std::is_same<typename Collection::value_type,T*>::value,
					  "Error: Collection is not a collection of T*.");

		SafeSet(std::size_t max_size = std::numeric_limits<std::size_t>::max())
			:InterimSafeSet<T*,Collection>(max_size){}
		
		using lock = typename MonotoneSafeSet<T*>::lock;
		T* pop(){
			lock l{this->m}; discard(l);
			if (this->impl.size() == 0) return nullptr;
			return this->pop_common();
		}
	};
}
