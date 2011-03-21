/*
 * Copyright (c) 2009, anirul
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the CERN nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY anirul ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL anirul BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
#ifndef BDB_BASIC_DB_DEFINED
#define BDB_BASIC_DB_DEFINED

#include <utility>
#include <memory>
#ifndef DB_H_ALREADY_INCLUDED
#define DB_H_ALREADY_INCLUDED
#include <db.h>
#endif
#include "bdb_iterator.h"
#include "bdb_serialize.h"

namespace bdb {

	// should stand for no multiple value like STL <map> class using the HASH
	// structure type.
	template <
		class Key, 
		class T, 
		class Compare = std::less<Key>, 
		class Allocator = std::allocator<std::pair<const Key, T> > >
	class basic_db {
		friend class bdb_iterator<Key, T, Allocator>;
	
	protected :
	
		DB* m_db;
	
	public :

		// types :
		typedef Key key_type;
		typedef T mapped_type;
		typedef std::pair<const Key, T> value_type;
		typedef Compare key_compare;
		typedef Allocator allocator_type;
		typedef typename Allocator::reference reference;
		typedef typename Allocator::const_reference const_reference;
		typedef bdb_iterator<Key, T> iterator;
		typedef const bdb_iterator<Key, T> const_iterator;
		typedef size_t size_type;
		typedef int difference_type;
		typedef typename Allocator::pointer pointer;
		typedef typename Allocator::const_pointer const_pointer;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

		class value_compare : 
			public std::binary_function<value_type, value_type, bool> 
		{
			friend class basic_db;
		protected :
			Compare comp;
			value_compare(Compare c) : comp(c) {}
		public :
			bool operator() (const value_type& x, const value_type& y) {
				return comp(x.first, y.first);
			}
		};
		
		// ctor and dtor
		explicit basic_db(
			const Compare& comp = Compare(), 
			const Allocator& alloc = Allocator()) : m_db(NULL) {}
		
		// copy operator in fact pointing to the same one as is is not possible
		// to create a second database from this parameters is actually used by
		// the swap operation.
		basic_db<Key, T, Compare, Allocator>& operator=(
			const basic_db<Key, T, Compare, Allocator>& x) {
			m_db = x.m_db;
			return *this;
		}
		
		bool is_open() { 
			return (m_db) ? true : false; 
		}
		
		void close() { 
			if (m_db) 
				m_db->close(m_db, 0);
			m_db = NULL;
		}
		
		// force write to DB
		void flush() const { 
			if (m_db) m_db->sync(m_db, 0);
		}

		// iterators
		iterator begin() { 
			return iterator(m_db, DB_FIRST); 
		}
		
		const_iterator begin() const { 
			return const_iterator(m_db, DB_FIRST); 
		}
		
		iterator end() { 
			return iterator(m_db, DB_LAST);
		}
		
		const_iterator end() const { 
			return const_iterator(m_db, DB_LAST);
		}
		
		reverse_iterator rbegin() { 
			return reverse_iterator(end()); 
		}
		
		const_reverse_iterator rbegin() const { 
			return const_reverse_iterator(end()); 
		}
		
        reverse_iterator rend() { 
			return reverse_iterator(begin());
		}
		
		const_reverse_iterator rend() const { 
			return const_reverse_iterator(begin()); 
		}

		// capacity
		bool empty() const { return (begin() == end()); }

		// no real meaning
		size_type max_size() const {
			return size_type(-1);
		}

		// due to the structure of BDB cannot return a reference on something
		// so return a const! (not writable!).
		// see 23.3.1.2 element access.
		const mapped_type operator[] (const key_type& k) {
			iterator it = find(k);
			if (it == end()) 
				return mapped_type();
			return it->second;
		}

		// modifiers
		std::pair<iterator, bool> insert(const value_type& x) {
			if (!m_db) throw 0;
			std::pair<iterator, bool> ret;
			iterator it = find(x.first);
			if (it != end()) {
				ret.first = it;
				ret.second = false;
			} else {
				key_type k = x.first;
				serialize<key_type> skey;
				skey.set_type(k);
				DBT key = skey.get_DBT();
				serialize<mapped_type> sdata;
				mapped_type m = x.second;
				sdata.set_type(m);
				DBT data = sdata.get_DBT();
				if (int i = m_db->put(m_db, NULL, &key, &data, 0)) throw i;
				ret.first = iterator(m_db, key, data);
				ret.second = true;
			}
			return ret;
		}
		
		// no real use just provided for compatibility reasons
		iterator insert(iterator position, const value_type& x) {
			std::pair<iterator, bool> ret = insert(x);
			if (ret.second) 
				return ret.first;
			else
				return end();
		}
		
		template <class InputIterator>
		void insert(InputIterator first, InputIterator last) {
			InputIterator ite;
			for (ite = first; ite != last; ++ite) {
				insert(*ite);
			}
			insert(*ite);
		}
		
		iterator erase(iterator position) {
			iterator here = position;
			position++;
			here.m_cursor->del(here.m_cursor, 0);
			return position;
		}
		
		size_type erase(const key_type& k) {
			iterator it = find(k);
			if (it == end()) return 0;
			erase(it);
			return 1;
		}
		
		iterator erase(iterator first, iterator last) {
			iterator here = last;
			here++;
			iterator ite;
			for (ite = first; ite != last; ++ite) {
				erase(*ite);
			}
			erase(*ite);
			return here;
		}
		
		void swap(basic_db<Key, T, Compare, Allocator>& x) {
			DB* temp;
			temp = m_db;
			m_db = x.m_db;
			x.m_db = temp;
		}
		
		void clear() {
			unsigned int cnt;
			m_db->truncate(m_db, NULL, (u_int32_t*)&cnt, 0);
		}

		// observers
		key_compare key_comp() const { 
			return key_compare(); 
		}
		
		value_compare value_comp() const { 
			return value_compare(); 
		}

		// associative supplementary function
		// there is a potential memory leak in find in case of DB_THREAD
		iterator find(const key_type& k) { 
			if (!m_db) throw 0;
			DBT key, data;
			memset(&key, 0, sizeof(DBT));
			memset(&data, 0, sizeof(DBT));
			serialize<key_type> skey;
			key_type tk = k;
			skey.set_type(tk); 
			key = skey.get_DBT();
			return iterator(m_db, key, data);
		}
		
		// there is a potential memory leak in find in case of DB_THREAD
		const_iterator find(const key_type& k) const { 
			if (!m_db) throw 0;
			DBT key, data;
			memset(&key, 0, sizeof(DBT));
			memset(&data, 0, sizeof(DBT)); 
			serialize<key_type> skey;
			key_type tk = k;
			skey.set_type(tk); 
			key = skey.get_DBT();
			return const_iterator(m_db, key, data);
		}
		
		// this is suppose to count the number of element with a given key
		size_type count(const key_type& k) const {
			int ret;
			if (!m_db) throw 0;
			const_iterator ite = find(k);
			db_recno_t recno;
			ret = ite.m_cursor->count(ite.m_cursor, &recno, 0);
			if (ret) throw ret;
			return recno;
		} 

		// find the first element after a key or end()
		iterator lower_bound(const key_type& k) {
			iterator ite = find(k);
			while ((ite != end()) && (ite->first == k)) 
				++ite;
			return ite; 
		}
		
		const_iterator lower_bound(const key_type& k) const {
			const_iterator ite = find(k);
			while ((ite != end()) && (ite->first == k))
				++ite;
			return ite;
		}
		
		// find the first element before a key or begin()
		iterator upper_bound(const key_type& k) {
			iterator ite = find(k);
			while ((ite != begin()) && (ite->first == k))
				--ite;
			return ite;
		}
		
		const_iterator upper_bound(const key_type& k) const {
			const_iterator ite = find(k);
			while ((ite != begin()) && (ite->first == k))
				--ite;
			return ite;
		}
	};
	
	// comparaison operations on a whole DB I assume to return false in case
	// the database doesn't have the same size, it could be interresting to
	// raise some kind or exception.
	template <class Key, class T, class Compare, class Allocator>
	bool operator==(
		const basic_db<Key, T, Compare, Allocator>& x,
		const basic_db<Key, T, Compare, Allocator>& y) {
		if (x.size() != y.size()) 
			return false;
		bdb_iterator<Key, T> xite = x.begin();
		bdb_iterator<Key, T> yite = y.begin();
		while (xite != x.end() || yite != y.end()) {
			if (*xite != *yite) 
				return false;
			++xite;
			--yite;
		}
		return true;	
	}
	
	template <class Key, class T, class Compare, class Allocator>
	bool operator< (
		const basic_db<Key, T, Compare, Allocator>& x,
		const basic_db<Key, T, Compare, Allocator>& y) {
		if (x.size() != y.size()) return false;
		bdb_iterator<Key, T> xite = x.begin();
		bdb_iterator<Key, T> yite = y.begin();
		while (xite != x.end() || yite != y.end()) {
			if (*xite >= *yite) 
				return false;
			++xite;
			--yite;
		}
		return true;
	}
	
	template <class Key, class T, class Compare, class Allocator>
	bool operator!=(
		const basic_db<Key, T, Compare, Allocator>& x,
		const basic_db<Key, T, Compare, Allocator>& y) {
		if (x.size() != y.size()) return false;
		bdb_iterator<Key, T> xite = x.begin();
		bdb_iterator<Key, T> yite = y.begin();
		while (xite != x.end() || yite != y.end()) {
			if (*xite == *yite) 
				return false;
			++xite;
			--yite;
		}
		return true;
	}

	template <class Key, class T, class Compare, class Allocator>
	bool operator> (
		const basic_db<Key, T, Compare, Allocator>& x,
		const basic_db<Key, T, Compare, Allocator>& y) {
		if (x.size() != y.size()) return false;
		bdb_iterator<Key, T> xite = x.begin();
		bdb_iterator<Key, T> yite = y.begin();
		while (xite != x.end() || yite != y.end()) {
			if (*xite <= *yite) 
				return false;
			++xite;
			--yite;
		}
		return true;
	}
	
	template <class Key, class T, class Compare, class Allocator>
	bool operator>=(
		const basic_db<Key, T, Compare, Allocator>& x,
		const basic_db<Key, T, Compare, Allocator>& y) {
		if (x.size() != y.size()) return false;
		bdb_iterator<Key, T> xite = x.begin();
		bdb_iterator<Key, T> yite = y.begin();
		while (xite != x.end() || yite != y.end()) {
			if (*xite < *yite) 
				return false;
			++xite;
			--yite;
		}
		return true;
	}
	
	template <class Key, class T, class Compare, class Allocator>
	bool operator<=(
		const basic_db<Key, T, Compare, Allocator>& x,
		const basic_db<Key, T, Compare, Allocator>& y) {
		if (x.size() != y.size()) return false;
		bdb_iterator<Key, T> xite = x.begin();
		bdb_iterator<Key, T> yite = y.begin();
		while (xite != x.end() || yite != y.end()) {
			if (*xite > *yite) 
				return false;
			++xite;
			--yite;
		}
		return true;
	}

	// specialized algorithms:
	template<class Key, class T, class Compare, class Allocator>
	void swap(
		basic_db<Key, T, Compare, Allocator>& x,
		basic_db<Key, T, Compare, Allocator>& y) {
		x.swap(y);
	}
	
} // end of namespace bdb

#endif // BDB_BASIC_DB_DEFINED
