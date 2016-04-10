#pragma once
#include <type_traits>
#include <tuple>
#include "HeteroMap.hpp"

template<typename T, std::size_t rank>
struct FibTree;

template<typename T>
struct FibTree<T,0> {
	T elem;
};

template<typename T, std::size_t rank>
struct FibTree : public FibTree<T,rank-1> {
	FibTree<T,rank - 1> newest;
	T elem;
};

template<typename T>
struct FibHeap {
	HeteroMap<int/* n, FibTree<T,n> */> trees; 
	//invariant; trees maps its keys (n) to FibTrees of rank n.
};


/* junkyard

template<std::size_t rank, typename T, typename ignore = std::enable_if_t<(rank == 0)> >
constexpr auto FibTree_upto_n(){
	return std::tuple<>();
}

template<std::size_t rank, typename T, typename ignore = void, typename ignore2 = std::enable_if_t<(rank > 0)> >
constexpr auto FibTree_upto_n(){
	return std::tuple_cat(std::make_tuple(FibTree<T,rank-1>{}),FibTree_upto_n<rank-1,T>());
}

*/
