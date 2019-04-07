#pragma once
#include "HeteroMap.hpp"

namespace mutils {

	template<typename Key,typename... Values>
	struct MultiTypeMap{
	private:
		HeteroMap<Key> map;
		template<typename T>
		constexpr static bool type_allowed(){
			bool results[] = {false,false,std::is_same<T,Values>::value...};
			for (bool b : results) if (b) return true;
			return false;
		}
		
	public:
		/**
			 Retrieve a constant unique_ptr corresponding to this key.  May be null, in which
			 case the map is not considered to contain this key.
		 */
		template<typename T>
		const std::unique_ptr<T>& at(const Key &i) const {
			static_assert(type_allowed<T>(),
										"Error: attempt to access type not present in this map");
			return map.template at<T>(i);
		}

		/**
			 Retrieve a unique_pointer to the value currently stored at i, or null if no stored
			 value exists.  
			 Call unique_ptr's "reset" method to insert (or overwrite) a new value for this key
		 */
		template<typename T>
		std::unique_ptr<T>& mut(const Key& i){
			static_assert(type_allowed<T>(),
										"Error: attempt to access type not present in this map");
			return map.template mut<T>(i);
		}
		
		bool contains(Key i) const {
			return map.contains(i);
		}

	};
}
