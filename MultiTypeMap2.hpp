#pragma once
#include <map>

namespace mutils{


	namespace mtype_map{
	
		template<typename Type, typename Key>
		struct map_entry{
			std::map<Key,Type> map;
		};

		template<typename...> struct find_entry;
		
		template<> struct find_entry<>{
			static void * get_entry(){
				return nullptr;
			}
		};
		
		template<typename Type, typename Key, typename... Other>
		struct find_entry<map_entry<Type,Key>, Other... > : public find_entry<Other...>{
			static std::map<Key,Type>& get_entry(map_entry<Type,Key>& o, Type const * const){
				return o.map;
			}
			
			static const std::map<Key,Type>& get_entry(const map_entry<Type,Key>& o, Type const * const){
				return o.map;
			}
			
			using find_entry<Other...>::get_entry;
		};
	}
	template<typename Key, typename... Types>
	struct MultiTypeMap : public mtype_map::map_entry<Types,Key>...{
			
		template<typename T>
		auto& mut(const Key& k){
			T *select{nullptr};
			return mtype_map::find_entry<mtype_map::map_entry<Types, Key>...>::get_entry(*this,select)[k];
		}

		template<typename T>
		const auto& at(const Key& k) const {
			T *select{nullptr};
			return mtype_map::find_entry<mtype_map::map_entry<Types, Key>...>::get_entry(*this,select).at(k);
		}
	};
}
