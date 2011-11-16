/*
 * Copyright (c) 2009-2011, Frederic DUBOUCHET
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

#ifndef MINIDHT_DB_HEADER_DEFINED
#define MINIDHT_DB_HEADER_DEFINED

// STL
#include <fstream>
#include <map>
#include <bitset>
#include <sstream>
// BOOST
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
// SQLite3
#include <sqlite3.h>
// local
#include "miniDHT_const.h"

namespace miniDHT {
	
	class db_key_value {
	protected :
		sqlite3* db_;
		std::string file_name_;

	public :
		db_key_value()
			:	db_(NULL), file_name_("") {}
		db_key_value(const std::string& file_name);
		virtual ~db_key_value();

	public :
		void open(const std::string& file_name);
		void create_table();
		void clear();
		std::string find(const std::string& key);
		void remove(const std::string& key);
		void insert(const std::string& key, const std::string& value);
		size_t size();
		// debug
		void list(std::map<std::string, std::string>& mm);
		void list_value(std::list<std::string>& l);
	};

	struct data_item_header_t {
		std::string key;
		std::string title;
		long long time;
		long long ttl;
	};
	
	class db_multi_key_data	{
	protected :
		sqlite3* db_;
		std::string file_name_;

	public :
		db_multi_key_data() 
			:	db_(NULL), file_name_("") {}
		db_multi_key_data(const std::string& file_name);
		virtual ~db_multi_key_data();

	public :
		void open(const std::string& file_name);
		void create_table();
		void clear();
		void find(const std::string& key, std::list<data_item_t>& out);
		void find(
			const std::string& key, 
			const std::string& title, 
			data_item_t& out);
		void remove(const std::string& key, const std::string& title);
		void remove(const std::string& key);
		void remove_oldest();
		void insert(const std::string& key, const data_item_t& item);
		void update(const std::string& key, const data_item_t& item);
		size_t count(const std::string& key);
		size_t size();
		// debug
		void list(std::multimap<std::string, data_item_t>& out);
		void list_headers(std::list<data_item_header_t>& ldh);
	};
}

#endif // MINIDHT_DB_HEADER_DEFINED
