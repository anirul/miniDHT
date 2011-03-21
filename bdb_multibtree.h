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
 
#ifndef BDB_MULTIBTREE_DEFINED
#define BDB_MULTIBTREE_DEFINED

#include <utility>
#include <memory>
#ifndef DB_H_ALREADY_INCLUDED
#define DB_H_ALREADY_INCLUDED
#include <db.h>
#endif
#include "bdb_iterator.h"
#include "bdb_serialize.h"
#include "bdb_basic_db.h"

namespace bdb {

	// should stand for no multiple value like STL <map> class using the HASH
	// structure type.
	template <
		class Key, 
		class T, 
		class Compare = std::less<Key>, 
		class Allocator = std::allocator<std::pair<const Key, T> > >
	class multibtree : public basic_db<Key, T, Compare, Allocator> {
	
		friend class bdb_iterator<Key, T, Allocator>;
	
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

		class value_compare : 
			public std::binary_function<value_type, value_type, bool> 
		{
			friend class multibtree;
		protected :
			Compare comp;
			value_compare(Compare c) : comp(c) {}
		public :
			bool operator() (const value_type& x, const value_type& y) {
				return comp(x.first, y.first);
			}
		};

		multibtree() { this->m_db = NULL; }

		virtual ~multibtree() { 
			this->flush();
			this->close(); 
		}
		
		// file operation (place the DB is stored)
		bool open(
			const char* db_file,
			const char* db_name = "default") {
			if (this->m_db) this->close();
			if (db_create(&this->m_db, NULL, 0)) return false;
			if (this->m_db->set_flags(this->m_db, DB_DUPSORT)) return false;
			if (this->m_db->open(
				this->m_db, 
				NULL, 
				db_file, 
				db_name, 
				DB_BTREE, 
				DB_CREATE, 
				0644)) 
				return false;
			return true;
		}
		
		// this could be long
		size_type size() const {
			DB_BTREE_STAT* pdbs = NULL;
			int ret;
			this->flush();
			if (!this->m_db) throw 0;
			ret = this->m_db->stat(this->m_db, NULL, &pdbs, 0); 
			if (ret) throw ret;
			size_t sz = (size_t)pdbs->bt_ndata;
			return sz;
		}

		// modifiers
		iterator insert(const value_type& x) {
			if (!this->m_db) throw 0;
			key_type k = x.first;
			serialize<key_type> skey;
			skey.set_type(k);
			serialize<mapped_type> sdata;
			mapped_type m = x.second;
			sdata.set_type(m);
			DBT key = skey.get_DBT();
			DBT data = sdata.get_DBT();
			iterator ret(this->m_db, key, data);
			if (ret == this->end())
				if (int i = this->m_db->put(this->m_db, NULL, &key, &data, 0)) 
					throw i;
			ret = iterator(this->m_db, key, data);
			return ret;
		}
	};
	
} // end of namespace bdb

#endif // BDB_MULTIBTREE_DEFINED
