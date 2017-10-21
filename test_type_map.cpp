#include "TypeMap2.hpp"
#include "KindMap.hpp"
#include "MultiTypeMap2.hpp"
#include <list>
#include <cassert>
#include <iostream>
#include <typeinfo>

using namespace mutils;

template<typename T> using list = std::list<T>;

int main(){
	TypeMap<std::list<int>, int, double, char> tm;
	tm.template get<double>().push_back(3);
	//tm.template get<long>();
	tm.for_each([](auto *type_p, auto num){assert(type_p == nullptr); std::cout << num.front() << std::endl;});
		
	KindMap<list,int,double,char> km;
	km.template get<double>().push_back(3.6);
	//km.template get<long>();

	km.for_each([](auto *type_p, auto num){assert(type_p == nullptr); std::cout << num.front() << std::endl;});

	MultiTypeMap<int,long,double,char> map;
	map.template mut<double>(3) = 3.5;
	const auto& cmap = map;
	assert(cmap.template at<double>(3) == 3.5);
	
}
