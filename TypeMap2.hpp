#pragma once

namespace mutils{


	namespace type_map{
	
		template<typename Type, typename Value>
		struct type_map_entry{
			Value v;
		};

		template<typename...> struct find_entry;
		
		template<> struct find_entry<>{
			static void * get_entry(){
				return nullptr;
			}
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
	};
}
