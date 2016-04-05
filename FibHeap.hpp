#pragma once
#include <type_traits>
#include <tuple>

template<typename T, std::size_t rank>
struct FibHeap;

template<typename T>
struct FibHeap<T,0> {
	T elem;
};

template<typename T, std::size_t rank>
struct FibHeap : public FibHeap<T,rank-1> {
	FibHeap<T,rank - 1> newest;
	T elem;
};


/* junkyard

template<std::size_t rank, typename T, typename ignore = std::enable_if_t<(rank == 0)> >
constexpr auto FibHeap_upto_n(){
	return std::tuple<>();
}

template<std::size_t rank, typename T, typename ignore = void, typename ignore2 = std::enable_if_t<(rank > 0)> >
constexpr auto FibHeap_upto_n(){
	return std::tuple_cat(std::make_tuple(FibHeap<T,rank-1>{}),FibHeap_upto_n<rank-1,T>());
}

*/
