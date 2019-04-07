#include "mutils-containers/buffer_generator.hpp"
#include <cassert>
#include <iostream>

using namespace mutils;

int main(){
	BufferGenerator<1024> bufs;
	auto first_ptr = bufs.allocate().grow_to_fit(1024);
	std::cout << "first_ptr.size() " << first_ptr.size() << std::endl;
	std::cout << "first_ptr.payload " << (void*) first_ptr.payload << std::endl;
	assert(first_ptr.size() == 1024);
	auto second_ptr = first_ptr.split(512);
	first_ptr.payload[512] = 32;
	first_ptr.payload[256] = 45;
	assert(second_ptr.payload[0] == 32);
	auto third_ptr = first_ptr.split(256);
	assert(first_ptr.size() == 256);
	assert(first_ptr.payload);
	first_ptr.payload[256] = 46;
	assert(third_ptr.payload[0] == 46);
	auto fourth_ptr = first_ptr.split(0);
	assert(fourth_ptr.payload[0] == 46);
	assert(fourth_ptr.size() == 0);
	assert(first_ptr.size() == 256);
	assert(second_ptr.size() == 512);
	assert(third_ptr.size() == 256);
}
