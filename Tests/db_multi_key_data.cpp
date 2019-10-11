/*
 * Copyright (c) 2011, Frederic DUBOUCHET
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

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "miniDHT_proto.pb.h"
#include "miniDHT_db.h"

void add(
	const std::string& db_file,
	const std::string& key, 
	const std::string& title, 
	const std::string& file) 
{
	miniDHT::db_multi_key_data db(db_file);
	miniDHT::data_item_proto item;
	item.set_time(
		miniDHT::to_time_t(boost::posix_time::second_clock::universal_time()));
	item.set_ttl(boost::posix_time::minutes(15).total_seconds());
	item.set_title(title);
	{
		FILE* pfile = fopen(file.c_str(), "rb");
		if (!pfile) {
			std::stringstream ss("");
			ss << "Could not open file [" << file << "]!";
			throw std::runtime_error(ss.str());
		}
#if defined(_WIN64)
		_fseeki64(pfile, 0, SEEK_END);
		size_t total_size = _ftelli64(pfile);
		_fseeki64(pfile, 0, SEEK_SET);
#else
		fseeko(pfile, 0, SEEK_END);
		size_t total_size = ftello(pfile);
		fseeko(pfile, 0, SEEK_SET);
#endif
		item.mutable_data()->resize(total_size);
		size_t bytes_read = fread(
			&((*item.mutable_data())[0]), 
			sizeof(char), 
			total_size, 
			pfile);
		fclose(pfile);
	}	
	db.insert(
		key, 
		item.title(), 
		item.time(), 
		item.ttl(), 
		item.data());
}

void find(
	const std::string& db_file,
	const std::string& key) 
{
	miniDHT::db_multi_key_data db(db_file);
	std::cout << "\tkey   : " << key << std::endl;
	std::list<miniDHT::data_item_proto> item;
	db.find(key, item);
	std::list<miniDHT::data_item_proto>::iterator ite;
	int i = 0;
	for (ite = item.begin(); ite != item.end(); ++ite) {
		std::cout << "\titem(" << ++i << ")" << std::endl;
		std::cout << "\t\ttitle       : " << ite->title() << std::endl;
		std::cout << "\t\ttime        : " << ite->time() << std::endl;
		std::cout << "\t\tttl         : " << ite->ttl() << std::endl;
		std::cout << "\t\tdata.size() : " << ite->data().size() << std::endl;
	}
}

void list(const std::string& db_file) {
	miniDHT::db_multi_key_data db(db_file);
	std::multimap<std::string, miniDHT::data_item_proto> mmkd;
	db.list(mmkd);
	std::multimap<std::string, miniDHT::data_item_proto>::iterator ite;
	std::cout << "\tkey, data_item (" << db.size() << ")" << std::endl;
	for (ite = mmkd.begin(); ite != mmkd.end(); ++ite) {
		std::cout << "\t\tkey    : " << ite->first << std::endl;
		std::cout << "\t\t\ttitle       : " << ite->second.title() << std::endl;
		std::cout << "\t\t\ttime        : " << ite->second.time() << std::endl;
		std::cout << "\t\t\tttl         : " << ite->second.ttl() << std::endl;
		std::cout << "\t\t\tdata.size() : " << ite->second.data().size() << std::endl;
	}
}

void remove(
	const std::string& db_file,
	const std::string& key,
	const std::string& title) 
{
	miniDHT::db_multi_key_data db(db_file);
	db.remove(key, title);
}

int main(int ac, char** av) {
	std::string key = "";
	std::string title = "";
	std::string file = "";
	std::string db_file = "";
	try {
		boost::program_options::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "produce help message.")
			("key,k", boost::program_options::value<std::string>(), 
				"key that you want to find or add a value.")
			("title,t", boost::program_options::value<std::string>(),
				"value you want to add the the DB at key value.")
			("file,f", boost::program_options::value<std::string>(),
				"file you want associated to key and title.")
			("db,d", boost::program_options::value<std::string>(),
				"DB file you want to connect to.")
			("add,a", "add a value to the DB (key and value must be valid).")
			("remove,r", "remove a value associated to a key in the DB.")
			("find,f", "find a value in the BD (key must be valid).")
			("list,l", "list the whole DB.")
		;
		boost::program_options::variables_map vm;
		boost::program_options::store(
			boost::program_options::command_line_parser(
				ac,
				av).options(desc).run(),
			vm);
		boost::program_options::notify(vm);
		if (vm.count("help")) {
			std::cout << desc << std::endl;
			return 1;
		}
		if (vm.count("key")) {
			key = vm["key"].as<std::string>();
		}
		if (vm.count("title")) {
			title = vm["title"].as<std::string>();
		}
		if (vm.count("file")) {
			file = vm["file"].as<std::string>();
		}
		if (vm.count("db")) {
			db_file = vm["db"].as<std::string>();
		} else {
			db_file = "mulit_test";
		}
		if (vm.count("add")) {
			if (key == std::string("")) 
				throw std::runtime_error("Need a key to add data at key!");
			if (title == std::string(""))
				throw std::runtime_error("Need a title to add data at a key!");
			if (file == std::string(""))
				throw std::runtime_error("Need a file to add data at key!");
			std::cout << "add a value to the DB" << std::endl;
			add(db_file, key, title, file);
 		}
		if (vm.count("remove")) {
			if (key == std::string(""))
				throw std::runtime_error("Need a key to remove data!");
			if (title == std::string(""))
				throw std::runtime_error("Need a title to remove data!");
			std::cout << "remove a data associated to a key in the DB" << std::endl;
			remove(db_file, key, title);
		}
		if (vm.count("find")) {
			if (key == std::string(""))
				throw std::runtime_error("Need a key to data associated values!");
			std::cout << "find a data for a key in DB" << std::endl;
			find(db_file, key);
		}
		if (vm.count("list")) {
			std::cout << "List the whole DB" << std::endl;
			list(db_file);
		}
	} catch (std::exception& e) {
		std::cerr << "error: " << e.what() << std::endl;
		return -1;
	}
	return 0;
}


