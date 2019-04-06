#include "mutils-containers/MultiTypeMap.hpp"
#include <string>
#include <iostream>

using namespace mutils;

int main(){
	MultiTypeMap<int,char,double,std::string> m;
	m.template mut<double>(0).reset(new double(3.45));
	m.template mut<std::string>(1).reset(new std::string("hello!"));
	std::cout << *m.template at<std::string>(1);
}
