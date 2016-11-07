#pragma once
#include <cstddef>
#include <type_traits>

namespace mutils{

	template<typename T, std::size_t s, bool b, typename... Ctrarg>
	struct _array;

	template<typename T, std::size_t s, typename... Ctrarg>
	using array = _array<T,s,(s > 1),Ctrarg...>;
	
	template<typename T, std::size_t s, typename... Ctrarg>
	struct _array<T,s,true,Ctrarg...>{

		T one;
		array<T,s-1,Ctrarg...> rest;

		template<typename... T2>
		_array(Ctrarg && ... t1, T2 && ...t2)
			:one(std::forward<Ctrarg>(t1)...),
			 rest(std::forward<T2>(t2)...){}
		
		inline auto& operator[](std::size_t i){
			if (i == 0) return one;
			else return rest[i-1];
		}
	};

	template<typename T, std::size_t s, typename... Ctrarg>
	struct _array<T,s,false, Ctrarg...>{

		T one;
		_array(Ctrarg && ...t2)
			:one(std::forward<Ctrarg>(t2)...)
			{}
		
		inline auto& operator[](std::size_t i){
			if (i == 0) return one;
			else assert(false && "error: index out of bounds");
			struct dead_code{}; throw dead_code{};
		}
	};
};
