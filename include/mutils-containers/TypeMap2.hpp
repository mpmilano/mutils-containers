#pragma once
#include <type_traits>

namespace mutils{


	namespace type_map{
	
		template<typename Type, typename Value>
		struct type_map_entry{
			Value v;
			using type = Type;
		};

		template<typename...> struct find_entry;
		
		template<> struct find_entry<>{
			static void get_entry(){}
		};
		
		template<typename Type, typename Value, typename... Other>
		struct find_entry<type_map_entry<Type,Value>, Other... > : public find_entry<Other...>{
			static Value& get_entry(type_map_entry<Type,Value>& o, Type const * const){
				return o.v;
			}
			
			using find_entry<Other...>::get_entry;
		};
	}
	template<typename Value, typename... Types>
	struct TypeMap : public type_map::type_map_entry<Types,Value>...{
			
		template<typename T>
		auto& get(){
			T *key{nullptr};
			return type_map::find_entry<type_map::type_map_entry<Types, Value>...>::get_entry(*this,key);
		};

		template<typename F> void for_each(const F& fun){
			auto mapped_fun = [&](auto* type_map_entry){
				typename std::decay_t<decltype(*type_map_entry)>::type* key{nullptr};
				fun(key,type_map_entry->v);
				return true;
			};
			auto lst = {true,true,mapped_fun((type_map::type_map_entry<Types,Value>*) this)...};
			(void)lst;
		}
		
	};
}
