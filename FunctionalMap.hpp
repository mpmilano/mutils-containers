#pragma once
#include <memory>
#include "args-finder.hpp"

namespace mutils{

	template<typename key, typename value>
	struct _mapnode_super {
		const int height;
		_mapnode_super(const _mapnode_super&) = delete;
		_mapnode_super(int height):height(height){}
		virtual bool operator<(const _mapnode_super&) const = 0;
		
		bool operator==(const _mapnode_super& other) const {
			return !((*this) < other || other < (*this));
		}
	};
	
	template<typename key, typename value>
	struct _mapnode_empty : public _mapnode_super<key,value> {
		_mapnode_empty():_mapnode_super<key,value>{0}{}
		bool operator<(const _mapnode_super<key,value>& other) const {
			return (other.height > 0 ? true : false);
		}
	};
	
	template<typename key, typename value>
	struct _mapnode_nonempty;
	
	template<typename key, typename value>
	struct _mapnode {
	private:
		const std::shared_ptr<const _mapnode_super<key,value> > this_p;
	public:

		_mapnode(const _mapnode &l,const key &k,const value &v,const _mapnode &r, const int height);
		_mapnode(std::shared_ptr<const _mapnode_empty<key,value> >);
		_mapnode(std::shared_ptr<const _mapnode_nonempty<key,value> >);
		_mapnode();
		_mapnode(const _mapnode&) = default;

		template<typename Ret>
		Ret match(const std::function<Ret (std::shared_ptr<const _mapnode_empty<key,value> >)> &f,
				  const std::function<Ret (std::shared_ptr<const _mapnode_nonempty<key,value> >)> &g) const;

		bool operator==(const _mapnode &other) const {
			return (*this_p) == (*other.this_p);
		}

		bool operator<(const _mapnode &other) const {
			return (*this_p) < (*other.this_p);
		}

		bool operator>(const _mapnode &other) const {
			return other < (*this);
		}

		int height() const {
			return this_p->height;
		}
	};
	
	template<typename key, typename value>
	struct _mapnode_nonempty : public _mapnode_super<key,value> {
		const _mapnode<key,value> l;
		const key k;
		const value v;
		const _mapnode<key,value> r;
		_mapnode_nonempty(const decltype(l)& l, const decltype(k) &k, const decltype(v) &v, const decltype(r) &r, const int height)
			:_mapnode_super<key,value>(height),l(l),k(k),v(v),r(r){}

		bool operator<(const _mapnode_super<key,value>& other) const {
			return (other.height == 0 ? false : k < ((_mapnode_nonempty*)&other)->k );
		}
	};

	template<typename key, typename value>
	template<typename Ret>
	Ret _mapnode<key,value>::match(const std::function<Ret (std::shared_ptr<const _mapnode_empty<key,value> >)> &f,
									  const std::function<Ret (std::shared_ptr<const _mapnode_nonempty<key,value> >)> &g) const {
		if (auto ptr = std::dynamic_pointer_cast<const _mapnode_empty<key,value> >(this_p))
			return f(ptr); 
		else if (auto ptr =
				 std::dynamic_pointer_cast<const _mapnode_nonempty<key,value> >(this_p))
			return g(ptr); 
		else assert(false && "fell through");
	}

	template<typename key, typename value>
	_mapnode<key,value>::_mapnode(std::shared_ptr<const _mapnode_empty<key,value> > p)
		:this_p{p} {}

	template<typename key, typename value>
	_mapnode<key,value>::_mapnode(std::shared_ptr<const _mapnode_nonempty<key,value> > p)
		:this_p{p} {}


	template<typename key, typename value>
	_mapnode<key,value>::_mapnode():this_p{new _mapnode_empty<key,value>()} {}

	template<typename key, typename value>
	_mapnode<key,value>::_mapnode(const _mapnode<key,value> &l,const key& k,const value &v,const _mapnode<key,value> &r, const int height)
		:this_p(new _mapnode_nonempty<key,value>(l,k,v,r,height)){}
	
	template<typename Key, typename Value>
	struct map{
		using key = Key;
		using value = Value;
		using mapnode = _mapnode<Key,Value>;
		using empty = _mapnode_empty<Key,Value>;
		using nonempty = _mapnode_nonempty<Key,Value>;
		
		static auto create(const mapnode& l, const key& x, const value &d, const mapnode& r){
			int hl = l.height();
			int hr = r.height();
			return mapnode{l,x,d,r, hl >= hr ? hl + 1 : hr + 1};
		}

		static auto singleton(const key& x, const value &d){
			return mapnode{mapnode{},x,d,mapnode{},1};
		}

		static auto balance(const mapnode &l, const key &x, const value &d, const mapnode &r){
			static const std::function<mapnode (std::shared_ptr<const empty>)> invalid_arg =
				[](const auto&) -> mapnode {assert(false && "invalid arg!");};
			
			int hl = l.height();
			int hr = r.height();
			if (hl > hr + 2){
				return l.template match<mapnode>(
					/*empty*/ invalid_arg,
					/*nonempty*/[&](std::shared_ptr<const nonempty> l){
							if (l->l.height() >= l->r.height()){
								return create(l->l,l->k,l->v,create(l->r,x,d,r));
							}
							else {
								return l->r.template match<mapnode>(
									invalid_arg,
									[&](std::shared_ptr<const nonempty> lr){
										return create (
											create (l->l, l->k, l->v, lr->l),
											lr->k,
											lr->v,
											create (lr->r, x, d, r));
									}
									);
							}
						}
					);
			}
			else if (hr > hl + 2){
				return r.template match<mapnode>(invalid_arg,
							   [&](std::shared_ptr<const nonempty> r){
								   if (r->r.height() >= r->l.height()) 
									   return create (create (l, x, d, r->l),
													  r->k, r->v,r->r);
								   else {
									   return r->l.template match<mapnode>(invalid_arg,
														[&](std::shared_ptr<const nonempty> rl){
															return create (create (l, x, d, rl->l), rl->k, rl->v, create(rl->r, r->k, r->v, r->r));
														}
										   );
							 }
						}
					);
			}
			else {
				return mapnode{l,x,d,r, (hl >= hr ? hl + 1 : hr + 1)};
			}
		}

		static bool is_empty(const mapnode& mn){
			return mn.template match<bool>([](std::shared_ptr<const empty>){return true;},
							[](std::shared_ptr<const nonempty>){return false;}
				);
		}

		static constexpr bool compare(const key& l, const key& r){
			return (l < r ? -1 : (l > r ? 1 : 0));
			
		}
		
		static mapnode add(const key& x, const value& data, const mapnode& __mn){
			return __mn.template match<mapnode> (
				[&](std::shared_ptr<const empty>){return mapnode{mapnode{},x,data,mapnode{},1};},
				[&](std::shared_ptr<const nonempty> m){
					const int c = compare(x,m->k);
					if (c == 0) {
						return (m->v == data ? mapnode{m} : mapnode{m->l,x,data,m->r,m->height});
					}
					else if (c < 0) {
						const auto ll = add (x,data,m->l);
						return (m->l == ll ? mapnode{m} : balance(ll, m->k, m->v, m->r));
					}
					else {
						const auto rr = add (x, data, m->r);
						return (m->r == rr ? mapnode{m} : balance(m->l,m->k,m->v,rr));
					}
				});
		}

		static const value& find (const key&x, const mapnode& __mn){
			return __mn.template match<const value&>(
				[&] (std::shared_ptr<const empty>)-> const value& {assert(false && "not found!");},
				[&] (std::shared_ptr<const nonempty> m)-> const value& {
					const auto c = compare(x,m->k);
					return (c == 0 ? m->v : find(x, (c < 0 ? m->l : m->r)));
				}
				);
		}

		static bool mem(const key&x, const mapnode &__mn){
			return __mn.template match<bool>(
				[](std::shared_ptr<const empty>){return false;},
				[&](std::shared_ptr<const nonempty> m){
					const auto c = compare(x,m->k);
					return ( c == 0 ? m->v : find(x,(c < 0 ? m->l : m->r )));
				}
				);
		}
	};

}
