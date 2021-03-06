#pragma once
#include <map>
#include <set>
#include "mutils/mutils.hpp"

namespace mutils {

	template<typename T>
	using unique_ptr_alias = std::unique_ptr<T> ;
	
	template<typename Key, template<typename> class ptr = unique_ptr_alias>
	struct HeteroMap {
	private:
		

		using count = std::function<int (Key)>;
		using destructor = std::function<void ()>;
		template<typename T>
		using submap = std::map<Key, ptr<std::decay_t<T> > >;
		
		std::map<type_id, std::tuple<void*, count, destructor,std::string> > sub_maps;
		std::map<Key,type_id> member_set; //for membership queries, when we do not know the type
		
		template<typename T>
		submap<std::decay_t<T> >* get_submap(){
			static const auto tname = "this has been disabled"; //type_name_fast<std::decay_t<T> >();
			auto tid = get_type_id<std::decay_t<T> >();
			if (sub_maps.count(tid) == 0){
				submap<std::decay_t<T> >* newmap = new submap<std::decay_t<T> >{};
				sub_maps.emplace(
					tid,
					std::tuple<void*,count,destructor,std::string>{
						newmap,
							[newmap](Key k){return newmap->count(k);},
							[newmap](){delete newmap;},
								tname
								});
			}
			return (submap<std::decay_t<T> >*) std::get<0>(sub_maps[tid]);
		}
		
		template<typename T>
		const submap<std::decay_t<T> > * get_submap() const {
			return (submap<T>*) std::get<0>(sub_maps.at(get_type_id<std::decay_t<T> >()));
		}
	public:
		
		template<typename T>
		auto& at(Key i) const {
			auto tid = get_type_id<std::decay_t<T> >();
			assert([&]() -> bool {
					if (sub_maps.count(tid) == 0){
						if (member_set.count(i) != 0){
							//this would indicate that there's a disagreement between the type mapping in
							//member_set and the static type_id assignment
							assert(get_type_id<std::decay_t<T> >() == get_type_id<std::decay_t<T> >());
							assert(sub_maps.count(member_set.at(i)) > 0);
							std::cerr << "we have type ("
									  << std::get<3>(sub_maps.at(member_set.at(i)))
									  << ") for this key; you asked for ("
									  << type_name<std::decay_t<T> >() << ")" << std::endl;
							assert(false && "HeteroMap failure");
						}
					}
					return true;
				}());
			//this is just a normal failure of at() when there's no element to retrieve.
			if (sub_maps.count(tid) == 0) std::cout << "missing key: " << i << std::endl;
			assert(sub_maps.count(tid) > 0);
			return get_submap<std::decay_t<T> >()->at(i);
		}
		
		template<typename T>
		auto& mut(Key i){
			member_set.emplace(i,get_type_id<std::decay_t<T> >());
			return (*get_submap<std::decay_t<T> >())[i];
		}
		
		bool contains(Key i) const {
			bool ret = member_set.count(i) > 0;
			assert(!ret || (ret && std::get<1>(sub_maps.at(member_set.at(i)))(i) > 0));
			return ret;
		}
		
		virtual ~HeteroMap(){
			for (auto& p : sub_maps){
				std::get<2>(p.second)();
			}
		}
	};
}

