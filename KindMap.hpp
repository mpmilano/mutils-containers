#pragma once
#include <type_traits>
#include <vector>

namespace mutils{

	namespace kind_map {
		template<typename Type, template<class> class Value>
		struct kind_map_entry{
			using type = Type;
			using value_type = Value<Type>;
			Value<Type> v;
		};
		
		template<typename...> struct find_entry;
		
		template<> struct find_entry<>{
			static void * get_entry(){
				return nullptr;
			}
		};
		
		template<typename Type, template<class> class Value, typename... Other>
		struct find_entry<kind_map_entry<Type,Value>, Other... > : public find_entry<Other...>{
			static Value<Type>& get_entry(kind_map_entry<Type,Value>& o, Type const * const){
				return o.v;
			}
			
			using find_entry<Other...>::get_entry;
		};
		
	}
	template<template<class> class Value, typename... Types>
	struct KindMap : public kind_map::kind_map_entry<Types,Value>...{

		template<typename T>
		auto& get(){
			T *key{nullptr};
			return kind_map::find_entry<kind_map::kind_map_entry<Types, Value>...>::get_entry(*this,key);
		}

		template<typename F> void for_each(const F& fun){
			auto mapped_fun = [&](auto* kind_map_entry){
				typename std::decay_t<decltype(*kind_map_entry)>::type* key{nullptr};
				return fun(key,kind_map_entry->v);
				return true;
			};
			auto lst = {true,true,mapped_fun((kind_map::kind_map_entry<Types,Value>*) this)...};
			assert(std::vector<bool>{lst}.size() > 0);
		}
	};
}
