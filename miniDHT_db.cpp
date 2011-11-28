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
		std::string sql_query = 
			"PRAGMA foreign_keys = ON;"\
			"CREATE TABLE IF NOT EXISTS data_header("\
				"id INTEGER PRIMARY KEY AUTOINCREMENT, "\
				"key TEXT, "\
				"title TEXT);"\
			"CREATE TABLE IF NOT EXISTS data_time("\
				"time_id INTEGER, "\
				"time BIGINT, "\
				"ttl BIGINT, "\
				"FOREIGN KEY (time_id) REFERENCES data_header(id) "\
				"ON DELETE CASCADE);"\
			"CREATE TABLE IF NOT EXISTS data_item("\
				"item_id INTEGER, "\
				"data BLOB, "\
				"FOREIGN KEY (item_id) REFERENCES data_header(id) "\
				"ON DELETE CASCADE);"\
			"CREATE INDEX IF NOT EXISTS data_header_key_title "\
			"ON data_header(key, title);";
		rc = sqlite3_exec(
			db_,
			sql_query.c_str(),
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
			"DROP TABLE IF EXISTS data_header",
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
		rc = sqlite3_exec(
			db_,
			"DROP TABLE IF EXISTS data_time",
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

	void db_multi_key_data::remove(
		const std::string& key, 
		const std::string& title) 
	{
		int rc = 0;
		char* szMsg;
		{ // data header search and clean
			std::stringstream ss("");
			ss << "DELETE FROM data_header WHERE key = '";
			ss << key << "' AND title = '";
			ss << title << "'";
			rc = sqlite3_exec(
				db_,
				ss.str().c_str(),
				NULL,
				0,
				&szMsg);
		}
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
			"DELETE FROM data_header WHERE id IN "\
				"(SELECT data_header.id FROM data_header, data_time "\
				"ORDER BY data_time.time ASC LIMIT 1)",
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

	void db_key_value::insert(
		const std::string& key, 
		const std::string& value) 
	{
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
		const std::string& title,
		const long long& time,
		const long long& ttl)
	{
		int rc = 0;
		char* szMsg;
		{
			std::stringstream ss("");
			ss << "UPDATE data_time SET time = '";
			ss << time << "', ttl = '";
			ss << ttl << "' WHERE data_time.time_id IN ( ";
			ss << "SELECT data_time.time_id ";
			ss << "FROM data_time, data_header ";
			ss << "WHERE data_header.key = '" << key << "' ";
			ss << "AND data_header.title = '" << title << "' ";
			ss << "AND data_time.time_id = data_header.id)";
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
		const std::string& title,
		const long long& time,
		const long long& ttl,
		const std::string& data) 
	{
		int rc = 0;
		char* szMsg;
		{ // insert the header
			std::stringstream ss("");
			ss << "INSERT INTO data_header VALUES(NULL, '";
			ss << key << "', '";
			ss << title << "')";
			rc = sqlite3_exec(
				db_,
				ss.str().c_str(),
				NULL,
				0,
				&szMsg);
		}
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in INSERT data_header : ";
			ss << szMsg;
			sqlite3_free(szMsg);
			throw std::runtime_error(ss.str());
		}
		{ // insert the time
			std::stringstream ss("");
			ss << "INSERT INTO data_time ";
			ss << "SELECT data_header.id, '" << time << "', '";
			ss << ttl << "' ";
			ss << "FROM data_header ";
			ss << "WHERE data_header.key = '" << key << "' ";
			ss << "AND data_header.title = '" << title << "' ";
			rc = sqlite3_exec(
				db_,
				ss.str().c_str(),
				NULL,
				0,
				&szMsg);
		}
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in INSERT data_time : ";
			ss << szMsg;
			sqlite3_free(szMsg);
			throw std::runtime_error(ss.str());
		}
		sqlite3_stmt* stmt = NULL;
		{ // insert the BLOB
			std::stringstream ss("");
			ss << "INSERT INTO data_item ";
			ss << "SELECT data_header.id, ? ";
			ss << "FROM data_header ";
			ss << "WHERE data_header.key = '" << key << "' ";
			ss << "AND data_header.title = '" << title << "' ";
			rc = sqlite3_prepare_v2(
				db_, 
				ss.str().c_str(),
				ss.str().size(),
				&stmt,
				NULL);
		}
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in prepare INSERT data_item : ";
			ss << sqlite3_errmsg(db_);
			throw std::runtime_error(ss.str());
		}
		// assign the BLOB
		rc = sqlite3_bind_blob(
			stmt, 
			1, 
			&(data[0]), 
			data.size(), 
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
		ss << "SELECT Count(*) FROM data_header WHERE key = '";
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
			"SELECT Count(*) FROM data_header",
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
		data_item_proto& out)
	{
		int rc = 0;
		sqlite3_stmt* stmt = NULL;
		{ // for ss I m lazy
			std::stringstream ss("");
			ss << "SELECT ";
			ss << "data_time.time, data_time.ttl, ";
			ss << "data_header.title, data_item.data ";
			ss << "FROM data_header, data_time, data_item ";
			ss << "WHERE data_header.id IN (";
			ss << "SELECT data_header.id ";
			ss << "FROM data_header WHERE key = '" << key << "'";
			ss << " AND title = '" << title << "') ";
			ss << "AND data_time.time_id IN ( ";
			ss << "SELECT data_time.time_id ";
			ss << "FROM data_header, data_time ";
			ss << "WHERE data_header.key = '" << key << "' "; 
			ss << "AND data_header.title = '" << title <<  "' ";
			ss << " AND data_header.id = data_time.time_id) ";
			ss << "AND data_item.item_id IN ( ";
			ss << "SELECT data_item.item_id ";
			ss << "FROM data_header, data_item ";
			ss << "WHERE data_header.key = '" << key << "' ";
			ss << "AND data_header.title = '" << title << "' ";
			ss << "AND data_header.id = data_item.item_id);";
			rc = sqlite3_prepare_v2(
				db_,
				ss.str().c_str(),
				-1,
				&stmt,
				NULL);
		}
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in SELECT [key title] : ";
			ss << sqlite3_errmsg(db_);
			throw std::runtime_error(ss.str());
		}
		if (sqlite3_step(stmt) != SQLITE_ROW) {
			std::stringstream ss("");
			ss << "SQL error in step : "; 
			ss << sqlite3_errmsg(db_);
			throw std::runtime_error(ss.str());
		}
		// for each column
		for (int i = 0; i < sqlite3_column_count(stmt); ++i) {
			std::string column_name = sqlite3_column_name(stmt, i);
			if (column_name == std::string("time")) 
				out.set_time(sqlite3_column_int64(stmt, i));
			if (column_name == std::string("ttl"))
				out.set_ttl(sqlite3_column_int64(stmt, i));
			if (column_name == std::string("title"))
				out.set_title((const char*)sqlite3_column_text(stmt, i));
			if (column_name == std::string("data")) {
				size_t data_size = sqlite3_column_bytes(stmt, i);
				out.set_data(sqlite3_column_blob(stmt, i), data_size);
			}
		}
		rc = sqlite3_finalize(stmt);
		if (rc != SQLITE_OK)
			throw std::runtime_error(
				"SQL error finalize in SELECT [key value] !");	
	}

	void db_multi_key_data::find(
		const std::string& key,
		std::list<data_item_proto>& out)
	{
		int rc = 0;
		sqlite3_stmt* stmt = NULL;
		{ // for ss I m lazy
			std::stringstream ss("");
			ss << "SELECT ";
			ss << "data_time.time, data_time.ttl, ";
			ss << "data_header.title, data_item.data ";
			ss << "FROM data_header, data_time, data_item ";
			ss << "WHERE data_header.id IN ( ";
			ss << "SELECT data_header.id ";
			ss << "FROM data_header ";
			ss << "WHERE key = '" << key << "') ";
			ss << "AND data_time.time_id IN ( ";
			ss << "SELECT data_time.time_id ";
			ss << "FROM data_header, data_time ";
			ss << "WHERE data_header.key = '" << key << "' "; 
			ss << "AND data_header.id = data_time.time_id) ";
			ss << "AND data_item.item_id IN ( ";
			ss << "SELECT data_item.item_id ";
			ss << "FROM data_header, data_item ";
			ss << "WHERE data_header.key = '" << key << "' ";
			ss << "AND data_header.id = data_item.item_id);";
			rc = sqlite3_prepare_v2(
				db_,
				ss.str().c_str(),
				-1,
				&stmt,
				NULL);
		}
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in SELECT [...] : ";
			ss << sqlite3_errmsg(db_);
			throw std::runtime_error(ss.str());
		}
		// for each row
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			// for each column
			data_item_proto di;
			for (int i = 0; i < sqlite3_column_count(stmt); ++i) {
				std::string column_name = sqlite3_column_name(stmt, i);
				if (column_name == std::string("time")) 
					di.set_time(sqlite3_column_int64(stmt, i));
				if (column_name == std::string("ttl"))
					di.set_ttl(sqlite3_column_int64(stmt, i));
				if (column_name == std::string("title"))
					di.set_title((const char*)sqlite3_column_text(stmt, i));
				if (column_name == std::string("data")) {
					size_t data_size = sqlite3_column_bytes(stmt, i);
					di.set_data(sqlite3_column_blob(stmt, i), data_size);
				}
			}
			out.push_back(di);
		}
		rc = sqlite3_finalize(stmt);
		if (rc != SQLITE_OK)
			throw std::runtime_error("SQL error finalize in SELECT * !");
	}

	void db_multi_key_data::find_no_blob(
		const std::string& key,
		std::list<data_item_proto>& out)
	{
		int rc = 0;
		sqlite3_stmt* stmt = NULL;
		{ // for ss I m lazy
			std::stringstream ss("");
			ss << "SELECT ";
			ss << "data_time.time, data_time.ttl, data_header.title ";
			ss << "FROM data_header, data_time, data_item ";
			ss << "WHERE data_header.id IN ( ";
			ss << "SELECT data_header.id ";
			ss << "FROM data_header ";
			ss << "WHERE key = '" << key << "') ";
			ss << "AND data_time.time_id IN ( ";
			ss << "SELECT data_time.time_id ";
			ss << "FROM data_header, data_time ";
			ss << "WHERE data_header.key = '" << key << "' "; 
			ss << "AND data_header.id = data_time.time_id)";
			rc = sqlite3_prepare_v2(
				db_,
				ss.str().c_str(),
				-1,
				&stmt,
				NULL);
		}
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in SELECT [...] : ";
			ss << sqlite3_errmsg(db_);
			throw std::runtime_error(ss.str());
		}
		// for each row
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			// for each column
			data_item_proto di;
			for (int i = 0; i < sqlite3_column_count(stmt); ++i) {
				std::string column_name = sqlite3_column_name(stmt, i);
				if (column_name == std::string("time")) 
					di.set_time(sqlite3_column_int64(stmt, i));
				if (column_name == std::string("ttl"))
					di.set_ttl(sqlite3_column_int64(stmt, i));
				if (column_name == std::string("title"))
					di.set_title((const char*)sqlite3_column_text(stmt, i));
			}
			out.push_back(di);
		}
		rc = sqlite3_finalize(stmt);
		if (rc != SQLITE_OK)
			throw std::runtime_error("SQL error finalize in SELECT * !");
	}

	void db_multi_key_data::list(
		std::multimap<std::string, data_item_proto>& out) 
	{
		int rc = 0;
		sqlite3_stmt* stmt = NULL;
		rc = sqlite3_prepare_v2(
			db_,
			"SELECT data_time.time, data_time.ttl, "\
			"data_header.title, data_item.data "\
			"FROM data_header, data_time, data_item "\
			"WHERE data_header.id = data_time.time_id "\
			"AND data_header.id = data_item.item_id",
			-1,
			&stmt,
			NULL);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in SELECT * : ";
			ss << sqlite3_errmsg(db_);
			throw std::runtime_error(ss.str());
		}
		// for each row
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			// for each column
			data_item_proto di;
			std::string key = "";
			for (int i = 0; i < sqlite3_column_count(stmt); ++i) {
				std::string column_name = sqlite3_column_name(stmt, i);
				if (column_name == std::string("key"))
					key = (const char*)sqlite3_column_text(stmt, i);
				if (column_name == std::string("time"))
					di.set_time(sqlite3_column_int64(stmt, i));
				if (column_name == std::string("ttl"))
					di.set_ttl(sqlite3_column_int64(stmt, i));
				if (column_name == std::string("title"))
					di.set_title((const char*)sqlite3_column_text(stmt, i));
				if (column_name == std::string("data")) {
					size_t data_size = sqlite3_column_bytes(stmt, i);
					di.set_data(sqlite3_column_blob(stmt, i), data_size);
				}
			}
			out.insert(std::make_pair<std::string, data_item_proto>(key, di));				
		}
		rc = sqlite3_finalize(stmt);
		if (rc != SQLITE_OK)
			throw std::runtime_error("SQL error finalize in SELECT * !");
	}
		
	void db_multi_key_data::list_headers(std::list<data_item_header_t>& out) {	
		int rc = 0;
		sqlite3_stmt* stmt = NULL;
		rc = sqlite3_prepare_v2(
			db_,
			"SELECT data_header.key, data_time.time, "\
			"data_time.ttl, data_header.title "\
			"FROM data_header, data_time, data_item "\
			"WHERE data_header.id = data_time.time_id "\
			"AND data_header.id = data_item.item_id",
			-1,
			&stmt,
			NULL);
		if (rc != SQLITE_OK) {
			std::stringstream ss("");
			ss << "SQL error in SELECT key, time, ttl, title : ";
			ss << sqlite3_errmsg(db_);
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

