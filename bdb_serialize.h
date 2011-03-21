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
 * DISCLAIMED. IN NO EVENT SHALL Frederic DUBOUCHEDT BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef BDB_SERIALIZE_DEFINED
#define BDB_SERIALIZE_DEFINED

// STL types I could need to cast
#include <string>
#include <sstream>
// Sequence Containers
#include <vector>
#include <list>
#include <deque>
// Associative Containers
#include <map>
#include <set>
// Container adapters
#include <stack>
// needed for memset memcpy etc...
#include <string.h>
// BDB include
#ifndef DB_H_ALREADY_INCLUDED
#define DB_H_ALREADY_INCLUDED
#include <db.h>
#endif

#include <iostream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>

namespace bdb {

	typedef unsigned long size_32;

	// generic form
	template<typename T>
	class serialize {
	
		DBT m_db_store;
		T m_type_store;
		
	public :
	
		serialize() {
			memset(&m_db_store, 0, sizeof(DBT));
		}
		
		~serialize() {
			if (m_db_store.data)
				free(m_db_store.data);
		}
		
		static bool fixed_size() { return true; }
		
		void set_DBT(const DBT& db_data) {
			{
				m_db_store.size = db_data.size;
				m_db_store.data = realloc(m_db_store.data, db_data.size);
				memcpy(m_db_store.data, db_data.data, db_data.size);
			}
			try {
				std::string str((char*)db_data.data, db_data.size);
				std::stringstream ss(str);
				boost::archive::xml_iarchive xia(ss);
				xia >> BOOST_SERIALIZATION_NVP(m_type_store);
			} catch (std::exception& e) {
				std::cerr 
					<< "Exception in serialize<T>::set_DBT() : " << e.what() 
					<< std::endl;
			}
		}
		
		void set_type(T& prog_data) {
			{
				m_type_store = prog_data;
			}
			try {
				std::stringstream ss("");
				boost::archive::xml_oarchive xoa(ss);
				xoa << BOOST_SERIALIZATION_NVP(m_type_store);
				std::string str = ss.str();
				m_db_store.size = (unsigned int)str.size();
				m_db_store.data = realloc(m_db_store.data, str.size());
				memcpy(m_db_store.data, str.c_str(), str.size());
			} catch (std::exception& e) {
				std::cerr 
					<< "Exception in serialize<T>::set_type : " << e.what() 
					<< std::endl;
			}
		}
		
		const DBT& get_DBT() const { return m_db_store; }
		
		const T& get_type() const { return m_type_store; }
	};

	// TODO add suport for other types...
	// (should not be needed is handled by boost)

} // end namespace bdb

#endif // BDB_SERIALIZE_DEFINED

