#pragma once
#include <set>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>

namespace mutils{

	template<typename T>
	constexpr void discard(const T&){}

	template<typename T>
	struct MonotoneSafeSet {
		std::set<T> impl;
		std::mutex m;
		std::condition_variable cv;
		using lock = std::unique_lock<std::mutex>;
		template<typename... Args>
		T& emplace(Args && ... args){
			lock l{m}; discard(l);
			impl.emplace_back(std::forward<Args>(args)...);
			return impl.back();
		}

		bool empty() const{
			return impl.empty();
		}

		auto size() const{
			return impl.size();
		}

		void add(T t){
			{
				lock l{m}; discard(l);
				impl.insert(t);
			}
			cv.notify_all();
		}

		std::set<T> iterable_copy(){
			return impl;
		}

		std::set<T>& iterable_reference(){
			return impl;
		}

		void remove(const T &t){
			lock l{m}; discard(l);
			impl.erase(std::find(impl.begin(), impl.end(),t));
		}
	
	};

	template<typename T>
	struct InterimSafeSet : MonotoneSafeSet<T>{

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
			if (this->impl.size() == 0) this->impl.emplace(std::forward<Args>(args)...);
			return pop_common();
		}

		template<typename... Args>
		T build_or_pop(const std::function<Args ()>& ... args){
			lock l{this->m}; discard(l);
			if (this->impl.size() == 0) this->impl.emplace(args()...);
			return pop_common();
		}
	};

	template<typename T>
	struct SafeSet : InterimSafeSet<T> {
		using lock = typename MonotoneSafeSet<T>::lock;
		T pop(){
			lock l{this->m}; discard(l);
			if (this->impl.size() == 0) {
				this->cv.wait(l,[&](){return this->impl.size() > 0;});
			}
			return this->pop_common();
		}
	};

	template<typename T>
	struct SafeSet<T*> : InterimSafeSet<T*>{
		using lock = typename MonotoneSafeSet<T>::lock;
		T* pop(){
			lock l{this->m}; discard(l);
			if (this->impl.size() == 0) return nullptr;
			auto r = this->impl.front();
			this->impl.pop_front();
			return r;
		}	
	};
}
