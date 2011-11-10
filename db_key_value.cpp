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
#include "miniDHT_db.h"

void add(const std::string& key, const std::string& value) {
	miniDHT::db_key_value db("test");
	db.insert(key, value);
}

void find(const std::string& key) {
	miniDHT::db_key_value db("test");
	std::cout << "\tkey   : " << key << std::endl;
	std::cout << "\tvalue : " << db.find(key) << std::endl;
}

void list() {
	miniDHT::db_key_value db("test");
	std::map<std::string, std::string> mm;
	db.list(mm);
	std::map<std::string, std::string>::iterator ite;
	std::cout << "\tkey, value (" << db.size() << ")" << std::endl;
	for (ite = mm.begin(); ite != mm.end(); ++ite) {
		std::cout << "\t" << ite->first << ", " << ite->second << std::endl;
	}
}

void remove(const std::string& key) {
	miniDHT::db_key_value db("test");
	db.remove(key);
}

int main(int ac, char** av) {
	std::string key = "";
	std::string value = "";
	try {
		boost::program_options::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "produce help message.")
			("key,k", boost::program_options::value<std::string>(), 
				"key that you want to find or add a value.")
			("value,v", boost::program_options::value<std::string>(),
				"value you want to add the the DB at key value.")
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
		if (vm.count("value")) {
			value = vm["value"].as<std::string>();
		}
		if (vm.count("add")) {
			if (key == std::string("")) 
				throw std::runtime_error("Need a key to add a value!");
			if (value == std::string(""))
				throw std::runtime_error("Need a valut to add a value at a key!");
			std::cout << "add a value to the DB" << std::endl;
			add(key, value);
 		}
		if (vm.count("remove")) {
			if (key == std::string(""))
				throw std::runtime_error("Need a key to remove a value!");
			std::cout << "remove a value associated to a key in the DB" << std::endl;
			remove(key);
		}
		if (vm.count("find")) {
			if (key == std::string(""))
				throw std::runtime_error("Need a key to find assiciated values!");
			std::cout << "find a value for a key in DB" << std::endl;
			find(key);
		}
		if (vm.count("list")) {
			std::cout << "List the whole DB" << std::endl;
			list();
		}
	} catch (std::exception& e) {
		std::cerr << "error: " << e.what() << std::endl;
		return -1;
	}
	return 0;
}

