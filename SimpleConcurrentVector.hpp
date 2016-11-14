#pragma once
#include <vector>
#include <shared_mutex>

namespace mutils{

	template<typename... A>
	class SimpleConcurrentVector{

		std::vector<A...> vector;
		mutable std::shared_mutex mut;
		using slock_t = std::shared_lock<std::shared_mutex>;
		using ulock_t = std::unique_lock<std::shared_mutex>;
	public:


		template<typename... T>
		SimpleConcurrentVector(T&& ... t)
			:vector(std::forward<T>(t)...)
			{}
		
		template<typename... T>
		const auto& at(T&& ... t) const {
			slock_t l{mut};
			return vector.at(std::forward<T>(t)...);
		}
		
		template<typename... T>
		auto& operator[](T&& ... t){
			slock_t l{mut};
			return vector.operator[](std::forward<T>(t)...);
		}

		template<typename... T>
		auto size(T&& ...t) const {
			return vector.size(std::forward<T>(t)...);
		}

		template<typename... T>
		auto emplace_back(T&& ...t)  {
			ulock_t l{mut};
			return vector.emplace_back(std::forward<T>(t)...);
		}

		void extend(typename std::vector<A...>::size_type count) {
			ulock_t l{mut};
			if (vector.size() < count) vector.resize(count);
		}
	};
}
