/*
 * Copyright (c) 2011, anirul
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
 * DISCLAIMED. IN NO EVENT SHALL Frederic DUBOUCHEDT BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "miniDHT_db.h"

namespace miniDHT {

	db_key_value::db_key_value(const std::string& file_name)
		: db_(NULL), file_name_("") 
	{
		open(file_name);
	}

	db_multi_key_data::db_multi_key_data(const std::string& file_name)
		:	db_(NULL), file_name_("")
	{
		open(file_name);
	}

	db_key_value::~db_key_value() { 
		sqlite3_close(db_);	
		db_ = NULL;
	}

	db_multi_key_data::~db_multi_key_data() {
		sqlite3_close(db_);
		db_ = NULL;
	}

	void db_key_value::open(const std::string& file_name) {
		file_name_ = std::string(file_name);
		int rc = 0;
		rc = sqlite3_open(file_name_.c_str(), &db_);
		if (rc) {
			std::stringstream error("");
			error << "Can't open database: ";
			error << sqlite3_errmsg(db_);
			sqlite3_close(db_);
			db_ = NULL;
			throw std::runtime_error(error.str());
		}
		create_table();
	}

	void db_multi_key_data::open(const std::string& file_name) {
		file_name_ = std::string(file_name);
		int rc = 0;
		rc = sqlite3_open(file_name_.c_str(), &db_);
		if (rc) {
			std::stringstream error("");
			error << "Can't open database: ";
			error << sqlite3_errmsg(db_);
			sqlite3_close(db_);
			db_ = NULL;
			throw std::runtime_error(error.str());
		}
		create_table();	
	}

	void db_key_value::create_table() {
		int rc = 0;
		char* szErrMsg = 0;
		rc = sqlite3_exec(
			db_,
			"CREATE TABLE IF NOT EXISTS "\
			"contacts(key TEXT PRIMARY KEY, value TEXT)",
			NULL,
			0,
			&szErrMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in CREATE table : ";
			ss << szErrMsg;
			sqlite3_free(szErrMsg);
			throw std::runtime_error(ss.str());
		}
	}

	void db_multi_key_data::create_table() {		
		int rc = 0;
		char* szErrMsg = 0;
		rc = sqlite3_exec(
			db_,
			"CREATE TABLE IF NOT EXISTS "\
			"data_item(key TEXT, "\
			"time BIGINT, ttl BIGINT, title TEXT, data BLOB)",
			NULL,
			0,
			&szErrMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in CREATE TABLE : ";
			ss << szErrMsg;
			sqlite3_free(szErrMsg);
			throw std::runtime_error(ss.str());
		}
		rc = sqlite3_exec(
			db_,
			"CREATE INDEX IF NOT EXISTS "\
			"data_item_key_title ON data_item(key, title)",
			NULL,
			0,
			&szErrMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in CREATE INDEX : ";
			ss << szErrMsg;
			sqlite3_free(szErrMsg);
			throw std::runtime_error(ss.str());
		}
	}

	void db_key_value::clear() {
		int rc = 0;
		char* szErrMsg = 0;
		rc = sqlite3_exec(
			db_,
			"DROP TABLE IF EXISTS contacts",
			NULL,
			0,
			&szErrMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in DROP table : ";
			ss << szErrMsg;
			sqlite3_free(szErrMsg);
			throw std::runtime_error(ss.str());
		}
		create_table();
	}

	void db_multi_key_data::clear() {
		int rc = 0;
		char* szErrMsg = 0;
		rc = sqlite3_exec(
			db_,
			"DROP TABLE IF EXISTS data_item",
			NULL,
			0,
			&szErrMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in DROP table : ";
			ss << szErrMsg;
			sqlite3_free(szErrMsg);
			throw std::runtime_error(ss.str());
		}
		create_table();
	}

	std::string db_key_value::find(const std::string& key) {
		std::stringstream ss("");
		ss << "SELECT value FROM contacts WHERE key = '";
		ss << key << "'";
		int rc = 0;
		char** result;
		int nrow, ncol;
		char* szMsg;
		rc = sqlite3_get_table(
			db_,
			ss.str().c_str(),
			&result,
			&nrow,
			&ncol,
			&szMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in SELECT value : ";
			ss << szMsg;
			sqlite3_free(szMsg);
			sqlite3_free_table(result);
			throw std::runtime_error(ss.str());
		} 
		std::string ret = "";
		if (ncol != 1) return ret; 
		if (nrow == 1) ret = result[1];
		sqlite3_free_table(result);
		return ret;
	}

	void db_key_value::remove(const std::string& key) {
		std::stringstream ss("");
		ss << "DELETE FROM contacts WHERE key = '";
		ss << key << "'";
		int rc = 0;
		char* szMsg;
		rc = sqlite3_exec(
			db_,
			ss.str().c_str(),
			NULL,
			0,
			&szMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in DELETE : ";
			ss << szMsg;
			sqlite3_free(szMsg);
			throw std::runtime_error(ss.str());
		}
	}

	void db_multi_key_data::remove(const std::string& key) {
		std::stringstream ss("");
		ss << "DELETE FROM data_item WHERE key = '";
		ss << key << "'";
		int rc = 0;
		char* szMsg;
		rc = sqlite3_exec(
			db_,
			ss.str().c_str(),
			NULL,
			0,
			&szMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in DELETE : ";
			ss << szMsg;
			sqlite3_free(szMsg);
			throw std::runtime_error(ss.str());
		}
	}

	void db_multi_key_data::remove(
		const std::string& key, 
		const std::string& title) 
	{
		std::stringstream ss("");
		ss << "DELETE FROM data_item WHERE key = '";
		ss << key << "' AND title = '";
		ss << title << "'";
		int rc = 0;
		char* szMsg;
		rc = sqlite3_exec(
			db_,
			ss.str().c_str(),
			NULL,
			0,
			&szMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in DELETE : ";
			ss << szMsg;
			sqlite3_free(szMsg);
			throw std::runtime_error(ss.str());
		}
	}

	void db_multi_key_data::remove_oldest() {
		int rc = 0;
		char* szMsg;
		rc = sqlite3_exec(
			db_,
			"DELETE FROM data_item WHERE EXISTS time IN"\
			"(SELECT time FROM data_item ORDER BY time ASC LIMIT 1)",
			NULL,
			0,
			&szMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in DELETE oldest : ";
			ss << szMsg;
			throw std::runtime_error(ss.str());
		}	
	}

	void db_key_value::insert(const std::string& key, const std::string& value) {
		std::stringstream ss("");
		ss << "INSERT INTO contacts VALUES('";
		ss << key << "', '";
		ss << value << "')";
		int rc = 0;
		char* szMsg;
		rc = sqlite3_exec(
			db_,
			ss.str().c_str(),
			NULL,
			0,
			&szMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in INSERT table : ";
			ss << szMsg;
			sqlite3_free(szMsg);
			throw std::runtime_error(ss.str());
		}			
	}

	void db_multi_key_data::update(
		const std::string& key,
		const data_item_t& item)
	{
		int rc = 0;
		char* szMsg;
		{
			std::stringstream ss("");
			ss << "UPDATE data_item SET time = '";
			ss << item.time << "', ttl = '";
			ss << item.ttl << "' WHERE key = '";
			ss << key << "' AND title = '";
			ss << item.title << "'";
			rc = sqlite3_exec(
				db_,
				ss.str().c_str(),
				NULL,
				0,
				&szMsg);
		}
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in UPDATE table : ";
			ss << szMsg;
			sqlite3_free(szMsg);
			throw std::runtime_error(ss.str());
		}
	}

	void db_multi_key_data::insert(
		const std::string& key, 
		const data_item_t& item) 
	{
		int rc = 0;
		const char* szMsg;
		std::string sql_query_str = 
			"INSERT INTO data_item VALUES(?, ?, ?, ?, ?)";
		sqlite3_stmt* stmt = NULL;
		rc = sqlite3_prepare_v2(
			db_, 
			sql_query_str.c_str(),
			sql_query_str.size(),
			&stmt,
			&szMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in prepare INSERT table : ";
			ss << szMsg;
			sqlite3_free(&szMsg);
			throw std::runtime_error(ss.str());
		}
		rc = sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
		if (rc != SQLITE_OK) 
			throw std::runtime_error("SQL error binding key in INSERT!");
		rc = sqlite3_bind_int64(stmt, 2, to_time_t(item.time));
		if (rc != SQLITE_OK)
			throw std::runtime_error("SQL error binding time in INSERT!");
		rc = sqlite3_bind_int64(stmt, 3, item.ttl.total_seconds());
		if (rc != SQLITE_OK)
			throw std::runtime_error("SQL error binding ttl in INSERT!");
		rc = sqlite3_bind_text(
			stmt, 
			4, 
			item.title.c_str(), 
			-1, 
			SQLITE_TRANSIENT);
		if (rc != SQLITE_OK)
			throw std::runtime_error("SQL error binding title in INSERT!");
		rc = sqlite3_bind_blob(
			stmt, 
			5, 
			&(item.data[0]), 
			item.data.size(), 
			SQLITE_STATIC);
		if (rc != SQLITE_OK)
			throw std::runtime_error("SQL error binding data in INSERT!");
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE) {
			std::stringstream ss("");
			ss << "SQL error step in INSERT : ";
			ss << sqlite3_errmsg(db_);
			throw std::runtime_error(ss.str());
		}
		rc = sqlite3_finalize(stmt);
		if (rc != SQLITE_OK)
			throw std::runtime_error("SQL error finalize in INSERT!");
	}

	size_t db_key_value::size() {
		int rc = 0;
		char** result;
		int nrow, ncol;
		char* szMsg;
		rc = sqlite3_get_table(
			db_,
			"SELECT Count(*) FROM contacts",
			&result,
			&nrow,
			&ncol,
			&szMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in SELECT Count(*) : ";
			ss << szMsg;
			sqlite3_free(szMsg);
			sqlite3_free_table(result);
			throw std::runtime_error(ss.str());
		}
		if (ncol != 1) return 0;
		if (nrow != 1) return 0;
		return (size_t)atoi(result[1]);
	}

	size_t db_multi_key_data::count(const std::string& key) {
		int rc = 0;
		char** result;
		int nrow, ncol;
		char* szMsg;
		std::stringstream ss("");
		ss << "SELECT Count(*) FROM data_item WHERE key = '";
		ss << key << "'";
		rc = sqlite3_get_table(
			db_,
			ss.str().c_str(),
			&result,
			&nrow,
			&ncol,
			&szMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in SELECT Count(*) WHERE key = '";
			ss << key << "' : ";
			ss << szMsg;
			sqlite3_free(szMsg);
			sqlite3_free_table(result);
			throw std::runtime_error(ss.str());
		}
		if (ncol != 1) return 0;
		if (nrow != 1) return 0;
		return (size_t)atoi(result[1]);
	}

	size_t db_multi_key_data::size() {
		int rc = 0;
		char** result;
		int nrow, ncol;
		char* szMsg;
		rc = sqlite3_get_table(
			db_,
			"SELECT Count(*) FROM data_item",
			&result,
			&nrow,
			&ncol,
			&szMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in SELECT Count(*) : ";
			ss << szMsg;
			sqlite3_free(szMsg);
			sqlite3_free_table(result);
			throw std::runtime_error(ss.str());
		}
		if (ncol != 1) return 0;
		if (nrow != 1) return 0;
		return (size_t)atoi(result[1]);
	}

	void db_key_value::list(std::map<std::string, std::string>& mm) {
		int rc = 0;
		char** result;
		int nrow, ncol;
		char* szMsg;
		rc = sqlite3_get_table(
			db_,
			"SELECT * FROM contacts",
			&result,
			&nrow,
			&ncol,
			&szMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in SELECT * : ";
			ss << szMsg;
			sqlite3_free(szMsg);
			sqlite3_free_table(result);
			throw std::runtime_error(ss.str());
		}
		if (ncol != 2) return;
		for (int i = 2; i < (ncol * (nrow + 1)); i+= 2) {
			mm.insert(
				std::make_pair<std::string, std::string>(
					result[i],
					result[i + 1]));
		}
		sqlite3_free_table(result);
	}

	void db_multi_key_data::find(
		const std::string& key,
		const std::string& title,
		data_item_t& out)
	{
		int rc = 0;
		const char* szMsg = NULL;
		sqlite3_stmt* stmt = NULL;
		{ // for ss I m lazy
			std::stringstream ss("");
			ss << "SELECT time, ttl, title, data ";
			ss << "FROM data_item WHERE key = '";
			ss << key << "' AND title = '";
			ss << title << "'";
			rc = sqlite3_prepare_v2(
				db_,
				ss.str().c_str(),
				-1,
				&stmt,
				&szMsg);
		}
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in SELECT [key title] : ";
			ss << szMsg;
			sqlite3_free(&szMsg);
			throw std::runtime_error(ss.str());
		}
		if (sqlite3_step(stmt) != SQLITE_ROW) {
			std::stringstream ss("");
			ss << "SQL error in step : " << sqlite3_errmsg(db_);
			throw std::runtime_error(ss.str());
		}
		// for each column
		for (int i = 0; i < sqlite3_column_count(stmt); ++i) {
			std::string column_name = sqlite3_column_name(stmt, i);
			if (column_name == std::string("time")) 
				out.time = boost::posix_time::from_time_t(
					sqlite3_column_int64(stmt, i));
			if (column_name == std::string("ttl"))
				out.ttl = boost::posix_time::seconds(
					sqlite3_column_int64(stmt, i));
			if (column_name == std::string("title"))
				out.title = (const char*)sqlite3_column_text(stmt, i);
			if (column_name == std::string("data")) {
				size_t data_size = sqlite3_column_bytes(stmt, i);
				out.data.resize(data_size);
				memcpy(&(out.data[0]), sqlite3_column_blob(stmt, i), data_size);
			}
		}
		rc = sqlite3_finalize(stmt);
		if (rc != SQLITE_OK)
			throw std::runtime_error(
				"SQL error finalize in SELECT [key value] !");	
	}

	void db_multi_key_data::find(
		const std::string& key,
		std::list<data_item_t>& out)
	{
		int rc = 0;
		const char* szMsg = NULL;
		sqlite3_stmt* stmt = NULL;
		{ // for ss I m lazy
			std::stringstream ss("");
			ss << "SELECT time, ttl, title, data FROM data_item WHERE key = '";
			ss << key << "'";
			rc = sqlite3_prepare_v2(
				db_,
				ss.str().c_str(),
				-1,
				&stmt,
				&szMsg);
		}
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in SELECT [...] : ";
			ss << szMsg;
			sqlite3_free(&szMsg);
			throw std::runtime_error(ss.str());
		}
		// for each row
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			// for each column
			data_item_t di;
			for (int i = 0; i < sqlite3_column_count(stmt); ++i) {
				std::string column_name = sqlite3_column_name(stmt, i);
				if (column_name == std::string("time")) 
					di.time = boost::posix_time::from_time_t(
						sqlite3_column_int64(stmt, i));
				if (column_name == std::string("ttl"))
					di.ttl = boost::posix_time::seconds(
						sqlite3_column_int64(stmt, i));
				if (column_name == std::string("title"))
					di.title = (const char*)sqlite3_column_text(stmt, i);
				if (column_name == std::string("data")) {
					size_t data_size = sqlite3_column_bytes(stmt, i);
					di.data.resize(data_size);
					memcpy(&(di.data[0]), sqlite3_column_blob(stmt, i), data_size);
				}
			}
			out.push_back(di);
		}
		rc = sqlite3_finalize(stmt);
		if (rc != SQLITE_OK)
			throw std::runtime_error("SQL error finalize in SELECT * !");
	}

	void db_multi_key_data::list(
		std::multimap<std::string, data_item_t>& out) 
	{
		int rc = 0;
		const char* szMsg = NULL;
		sqlite3_stmt* stmt = NULL;
		rc = sqlite3_prepare_v2(
			db_,
			"SELECT * FROM data_item",
			-1,
			&stmt,
			&szMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in SELECT * : ";
			ss << szMsg;
			sqlite3_free(&szMsg);
			throw std::runtime_error(ss.str());
		}
		// for each row
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			// for each column
			data_item_t di;
			std::string key = "";
			for (int i = 0; i < sqlite3_column_count(stmt); ++i) {
				std::string column_name = sqlite3_column_name(stmt, i);
				if (column_name == std::string("key"))
					key = (const char*)sqlite3_column_text(stmt, i);
				if (column_name == std::string("time"))
					di.time = boost::posix_time::from_time_t(
						sqlite3_column_int64(stmt, i));
				if (column_name == std::string("ttl"))
					di.ttl = boost::posix_time::seconds(
						sqlite3_column_int64(stmt, i));
				if (column_name == std::string("title"))
					di.title = (const char*)sqlite3_column_text(stmt, i);
				if (column_name == std::string("data")) {
					size_t data_size = sqlite3_column_bytes(stmt, i);
					di.data.resize(data_size);
					memcpy(&(di.data[0]), sqlite3_column_blob(stmt, i), data_size);
				}
			}
			out.insert(std::make_pair<std::string, data_item_t>(key, di));				
		}
		rc = sqlite3_finalize(stmt);
		if (rc != SQLITE_OK)
			throw std::runtime_error("SQL error finalize in SELECT * !");
	}
		
	void db_multi_key_data::list_headers(std::list<data_item_header_t>& out) {	
		int rc = 0;
		const char* szMsg = NULL;
		sqlite3_stmt* stmt = NULL;
		rc = sqlite3_prepare_v2(
			db_,
			"SELECT key, time, ttl, title FROM data_item",
			-1,
			&stmt,
			&szMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in SELECT key, time, ttl, title : ";
			ss << szMsg;
			sqlite3_free(&szMsg);
			throw std::runtime_error(ss.str());
		}
		// for each row
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			// for each column
			data_item_header_t dh;
			std::string key = "";
			for (int i = 0; i < sqlite3_column_count(stmt); ++i) {
				std::string column_name = sqlite3_column_name(stmt, i);
				if (column_name == std::string("key"))
					dh.key = (const char*)sqlite3_column_text(stmt, i);
				if (column_name == std::string("time"))
					dh.time = sqlite3_column_int64(stmt, i);
				if (column_name == std::string("ttl"))
					dh.ttl = sqlite3_column_int64(stmt, i);
				if (column_name == std::string("title"))
					dh.title = (const char*)sqlite3_column_text(stmt, i);
			}
			out.push_back(dh);				
		}
		rc = sqlite3_finalize(stmt);
		if (rc != SQLITE_OK)
			throw std::runtime_error(
				"SQL error finalize in SELECT key, time, ttl, title !");
	}

	void db_key_value::list_value(std::list<std::string>& l) {
		int rc = 0;
		char** result;
		int nrow, ncol;
		char* szMsg;
		rc = sqlite3_get_table(
			db_,
			"SELECT value FROM contacts",
			&result,
			&nrow,
			&ncol,
			&szMsg);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in SELECT value : ";
			ss << szMsg;
			sqlite3_free(szMsg);
			sqlite3_free_table(result);
			throw std::runtime_error(ss.str());				
		}
		if (ncol != 1) return;
		for (int i = 1; i < (nrow + 1); ++i)
			l.push_back(result[i]);
		sqlite3_free_table(result);
	}

} // end namespace miniDHT

