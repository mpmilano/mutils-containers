#pragma once

namespace mutils{

	namespace kind_map {
		template<typename Type, template<class> class Value>
		struct kind_map_entry{
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
		};
	};
}
