#pragma once
#include <unordered_map>
#include <shared_mutex>

namespace mutils{

	template<typename... A>
	class SimpleConcurrentMap{

		std::unordered_map<A...> map;
		mutable std::shared_mutex mut;
		using slock_t = std::shared_lock<std::shared_mutex>;
		using ulock_t = std::unique_lock<std::shared_mutex>;
	public:		
		template<typename... T>
		const auto& at(T&& ... t) const {
			slock_t l{mut};
			return map.at(std::forward<T>(t)...);
		}
		
		template<typename... T>
		auto& operator[](T&& ... t){
			ulock_t l{mut};
			return map.operator[](std::forward<T>(t)...);
		}

		template<typename... T>
		auto count(T&& ...t) const {
			return map.count(std::forward<T>(t)...);
		}
	};
}
