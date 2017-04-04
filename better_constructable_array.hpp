#pragma once
#include <cstddef>
#include <type_traits>

namespace mutils{

	template<typename T, std::size_t s, bool b, typename... Ctrarg>
	struct _array;

	template<typename>
	struct empty_array {
		static empty_array rest;
	};
	template<typename T>
	empty_array<T> empty_array<T>::rest;
	
	template<typename T, std::size_t s, typename... Ctrarg>
	using array = std::conditional_t<(s > 0), _array<T,s,(s > 1),Ctrarg...>, empty_array<T> >;
	
	template<typename T, std::size_t s, typename... Ctrarg>
	struct _array<T,s,true,Ctrarg...>{

		T one;
		array<T,s-1,Ctrarg...> rest;

		template<typename... T2>
		_array(Ctrarg ... t1, T2 && ...t2)
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
		empty_array<T> rest;
		
		_array(Ctrarg ...t2)
			:one(std::forward<Ctrarg>(t2)...)
			{}
		
		inline auto& operator[](std::size_t i){
			if (i == 0) return one;
			else assert(false && "error: index out of bounds");
			struct dead_code{}; throw dead_code{};
		}
	};
};
