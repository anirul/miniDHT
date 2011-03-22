/*
 * Copyright (c) 2009, Frederic DUBOUCHET
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Frederic DUBOUCHET ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Frederic DUBOUCHET BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef BDB_ITERATOR_DEFINED
#define BDB_ITERATOR_DEFINED

#include <memory>
#ifndef DB_H_ALREADY_INCLUDED
#define DB_H_ALREADY_INCLUDED
#include <db.h>
#endif
#include "bdb_serialize.h"

namespace bdb {
	
	template <
		class Key, 
		class T, 
		class Allocator = std::allocator<std::pair<const Key, T> > >
	class bdb_iterator : public std::iterator<
			std::bidirectional_iterator_tag, 
			typename Allocator::value_type,
			typename Allocator::difference_type > 
	{
	public :
	
		typedef Key key_type;
		typedef T mapped_type;
		typedef std::pair<const Key, T> value_type;
		typedef value_type& reference;
		typedef value_type* pointer;

	private :

		bool m_end;
		DB* m_db;
		mutable std::pair<Key, T> m_temp_value;
		
	protected :
	
		bool is_end() const { 
			return m_end; 
		}
		
		DBC* get_cursor() const {
			DBC* pdbc = NULL;
			int ret = 0;
			if (!m_db) throw 0;
			ret = m_db->cursor(m_db, NULL, &pdbc, 0);
			if (ret) {
				m_db->err(
					m_db, 
					ret, 
					"get_cursor() cursor(DB* <%08x>, NULL, DBC* <%08x>, 0)", 
					m_db, 
					pdbc);
				throw ret;
			}
			return pdbc;
		}
		
	public :
		
		// for erase
		DBC* m_cursor;

		bdb_iterator() 
			:	m_end(true), m_db(NULL), m_cursor(NULL) {}
		
		bdb_iterator(DB* pdb, unsigned int flags) 
			:	m_end(true), m_db(pdb), m_cursor(NULL) 
		{
			if (flags == DB_LAST){ 
				m_end = true;
				return;
			}
			m_end = false;
			int ret = 0;
			DBT key, data;
			memset(&key, 0, sizeof(DBT));
			memset(&data, 0, sizeof(DBT));
			m_cursor = get_cursor();
			ret = m_cursor->get(m_cursor, &key, &data, flags);
			if (ret == DB_NOTFOUND) {
				m_end = true;
				return;
			}
			if (ret) {
				m_db->err(
					m_db, 
					ret, 
					"bdb_iterator cursor(DB* <%08x>, NULL, DBC* <%08x>, 0)", 
					m_db, 
					m_cursor);
				throw ret;
			}
		}
		
		bdb_iterator(DB* pdb, DBT key, DBT data) 
			:	m_end(true), m_db(pdb), m_cursor(NULL)
		{
			int ret = 0;
			if (!pdb) {
				m_end = true;
				return;
			}
			m_db = pdb;
			m_cursor = get_cursor();
			if (data.size)
				ret = m_cursor->get(m_cursor, &key, &data, DB_GET_BOTH);
			else
				ret = m_cursor->get(m_cursor, &key, &data, DB_SET);
			if ((ret == DB_NOTFOUND) || (ret == DB_KEYEMPTY)) {
				m_end = true;
				return;
			} else if (ret) {
				m_db->err(
					m_db, 
					ret, 
					"bdb_iterator c_get(DBC* <%08x>, DBT <%08x, %d>, DBT "\
					"<%08x, %d>, DB_SET)", 
					m_cursor, 
					key.data, key.size, 
					data.data, data.size);
				throw ret;
			} else {
				m_end = false;
			}
		}

		bdb_iterator(const bdb_iterator& i)
			:	m_end(true), m_db(NULL), m_cursor(NULL)
		{
			m_end = i.is_end();
			m_db = i.m_db;
			if (i.m_cursor)
				i.m_cursor->dup(i.m_cursor, &m_cursor, DB_POSITION);
		}
		
		~bdb_iterator() {
			if (m_cursor) m_cursor->close(m_cursor);
		}
		
		bdb_iterator& operator=(const bdb_iterator& i) {
			if (m_cursor) m_cursor->close(m_cursor);
			m_cursor = NULL;
			m_end = i.is_end();
			m_db = i.m_db;
			if (i.m_cursor)
				i.m_cursor->dup(i.m_cursor, &m_cursor, DB_POSITION);
			else 
				m_cursor = NULL;
			return *this;
		}
		
		bdb_iterator& operator++() {
			int ret = 0;
			db_recno_t cnt = 0;
			DBT data, key;
			memset(&key, 0, sizeof(DBT));
			memset(&data, 0, sizeof(DBT));
			ret = m_cursor->count(m_cursor, &cnt, 0);
			if (cnt && !ret)
				ret = m_cursor->get(m_cursor, &key, &data, DB_NEXT_DUP);
			if (ret == DB_NOTFOUND)
				ret = m_cursor->get(m_cursor, &key, &data, DB_NEXT);
			if (ret == DB_NOTFOUND) {
				m_end = true;
				return *this;
			} else if (ret) {
				m_db->err(
					m_db, 
					ret, 
					"bdb_iterator& operator++() c_get(DB* <%08x>, DBT <%08x,"\
					" %d>, DBT <%08x, %d>, DB_NEXT)", 
					m_db, 
					key.data, key.size, 
					data.data, data.size);
				throw ret;
			} else {
				m_end = false;
				return *this;
			}
		}
		
		bdb_iterator& operator--() {
			DBT data, key;
			memset(&key, 0, sizeof(DBT));
			memset(&data, 0, sizeof(DBT));
			if (is_end()) {
				m_end = false;
				if (!m_cursor) m_cursor = get_cursor();
				m_cursor->get(m_cursor, &key, &data, DB_LAST);
				return *this;
			}
			if (!m_cursor) throw 0;
			int ret = 0;
			db_recno_t cnt = 0;
			ret = m_cursor->count(m_cursor, &cnt, 0);
			if (cnt && !ret)
				ret = m_cursor->get(m_cursor, &key, &data, DB_PREV_DUP);
			if (ret == DB_NOTFOUND)
				ret = m_cursor->get(m_cursor, &key, &data, DB_PREV);
			if (ret == DB_NOTFOUND)
				throw 0;
			if (ret) {
				m_db->err(
					m_db, 
					ret, 
					"bdb_iterator& operator--() c_get(DB* <%08x>, DBT <%08x,"\
					" %d>, DBT <%08x, %d>, DB_PREV)", 
					m_db, 
					key.data, key.size, 
					data.data, data.size);
				throw ret;
			}
			return *this;
		}
		
		// not suppose to be writable
		reference operator*() const {
			if (!m_cursor) throw 0;
			int ret = 0;
			// CHECKME should throw?
			if (m_end) throw 0;
			serialize<key_type> skey;
			serialize<mapped_type> sdata;
			DBT key;
			DBT data;
			memset(&key, 0, sizeof(DBT));
			memset(&data, 0, sizeof(DBT));
			ret = m_cursor->get(m_cursor, &key, &data, DB_CURRENT);
			if (ret == DB_NOTFOUND) {
				m_temp_value = value_type(key_type(), mapped_type());
				return (reference)m_temp_value;
			} else if (ret) {
				m_db->err(
					m_db, 
					ret, 
					"value_type operator*() put(DB* <%08x>, NULL, DBT <%08x,"\
					" %d>, DBT <%08x, %d>, 0)", 
					m_db, 
					key.data, key.size, 
					data.data, data.size);
				throw ret;
			} else {
				skey.set_DBT(key);
				sdata.set_DBT(data);
				m_temp_value = value_type(skey.get_type(), sdata.get_type());
				return (reference)m_temp_value;
			}
		}
		
		bdb_iterator operator++(int) {
			bdb_iterator ret = *this;
			++*this;
			return ret;
		}

		bdb_iterator operator--(int) {
			bdb_iterator ret = *this;
			--*this;
			return ret;
		}
		
		bool operator==(const bdb_iterator& right) const {
			if (this->is_end() && right.is_end()) return true;
			if (this->is_end() || right.is_end()) return false;
			return (this->operator*() == (*right));
		}
		
		bool operator!=(const bdb_iterator& right) const {
			if (this->is_end() && right.is_end()) return false;
			if (this->is_end() || right.is_end()) return true;
			return (this->operator*() != (*right));
		}
		
		// not suppose to be writable
		const pointer operator->() {
			m_temp_value = this->operator*();
			return (std::pair<const Key, T>*)(&m_temp_value);
		}
		
	};

} // end of namespace bdb

#endif // BDB_ITERATOR_DEFINED

