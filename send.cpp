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
 * DISCLAIMED. IN NO EVENT SHALL anirul BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <boost/program_options.hpp>

#include <openssl/sha.h>
#include <openssl/evp.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <boost/filesystem.hpp>

#include <iostream>
#include <sstream>
#include <string>

#include "miniDHT.h"
#include "aes_crypt.h"
#include "send.h"

std::string dht_send_file::encode(
	const std::string& key, 
	const std::string& data) 
{
	aes_crypt<5, 42> ac(key);
	return ac.encrypt(data);
}

dht_send_file::dht_send_file(
	const std::string& file_name,
	miniDHT::miniDHT<key_size, token_size>* pDht) 
{	
	ifile_ = NULL;
	pDht_ = pDht;
	end_ = false;
	stop_ = false;
	packet_loaded_ = 0;
	boost::filesystem::path p(file_name);
	file_name_ = p.filename().string();
	std::cout 
		<< "Compute DIGEST of file [" 
		<< file_name 
		<< "] please wait." << std::endl;
	if (miniDHT::digest_file(digest_, file_name.c_str()))
		throw "Could not open file";
	std::cout << " -> DIGEST : [" << digest_ << "]" << std::endl;
	ifile_ = fopen(file_name.c_str(), "rb");
	if (!ifile_)
		throw "Invalid file to be send.";
	fseeko(ifile_, 0, SEEK_END);
	total_size_ = ftello(ifile_);
	if (total_size_ % buf_size) 
		packet_total_ = (total_size_ / buf_size) + 1;
	else
		packet_total_ = total_size_ / buf_size;
	fseeko(ifile_, 0, SEEK_SET);
	for (unsigned int i = 0; i < packet_total_; ++i)
		map_state_.insert(std::make_pair<size_t, upload_state_t>(
			i, 
			WAIT));
}

dht_send_file::~dht_send_file() {
	fclose(ifile_);
	ifile_ = NULL;
}

void dht_send_file::load(size_t index) {
	fseeko(ifile_, index * buf_size, SEEK_SET);
	std::string buffer;
	buffer.resize(buf_size);
	size_t bytes_read = fread(&buffer[0], sizeof(char), buf_size, ifile_);
	buffer.resize(bytes_read);
	map_load_.insert(std::make_pair<size_t, std::string>(
		index,
		buffer));
	map_state_[index] = LOADED;
}

void dht_send_file::crypt(size_t index) {
	// should not happen but how knows?
	if (map_load_.find(index) == map_load_.end()) {
		map_state_[index] = WAIT;
		return;
	}
	std::string key;
	{
		std::stringstream ss("");
		ss << digest_;
		key = ss.str();
	}
	map_crypt_.insert(
		std::make_pair<size_t, std::string>(
			index,
			encode(key, map_load_[index])));
	map_load_.erase(index);
	map_state_[index] = CRYPTED;
}

void dht_send_file::upload(size_t index) {
	std::string digest_string;
	std::string storage_string_id;
	{
		std::stringstream ss("");
		ss << digest_;
		digest_string = ss.str();
		ss << " " << std::dec << index;
		storage_string_id = ss.str();
	}
	std::string title_data;
	{
		aes_crypt<5, 42> ac(digest_string);
		std::stringstream ss("");
		ss << digest_;
		ss << " " << std::dec << index;
		ss << " " << std::dec << packet_total_;
		ss << " " << file_name_;
		title_data = ss.str();
	}
	miniDHT::data_item_t data;
	data.ttl = boost::posix_time::hours(24);
	data.time = miniDHT::update_time();
	data.title = encode(digest_string, title_data);
	std::string buffer = map_crypt_[index];
	data.data.resize(buffer.size());
	map_key_string_id_.insert(
		std::make_pair<std::string, size_t>(
			data.title,
			index));
	memcpy(&(data.data)[0], &buffer[0], buffer.size());
	pDht_->iterativeStore(
		miniDHT::digest_key_from_string<key_size>(storage_string_id), 
		data);
	map_state_[index] = UPLOADED;
}

void dht_send_file::check(size_t index) {
	std::stringstream ss("");
	ss << digest_ << " " << std::dec << index;
	std::string storage_string_id = ss.str();
	pDht_->iterativeFindValue(
		miniDHT::digest_key_from_string<key_size>(storage_string_id),
		boost::bind(&dht_send_file::found, this, _1));
}

void dht_send_file::found(const std::list<miniDHT::data_item_t>& b) {
	std::list<miniDHT::data_item_t>::const_iterator ite;
	for (ite = b.begin(); ite != b.end(); ++ite) {
		if (map_key_string_id_.find(ite->title) != map_key_string_id_.end()) {
			size_t index = map_key_string_id_[ite->title];
			// should not happen (make it faster not to check)
			std::string from = map_crypt_[index];
			std::string to = ite->data;
			if (from.size() != to.size())
				throw "Check error (size missmatch)";
			if (map_crypt_[index] != ite->data) {
				if (map_state_[index] != CHECKED)
					map_state_[index] = CRYPTED;
				throw "Check error (content missmatch)";
			}
			// cleanup
			std::cout << ".";
			std::cout.flush();
			map_key_string_id_.erase(map_key_string_id_.find(ite->title));
			map_crypt_.erase(map_crypt_.find(index));
			map_state_[index] = CHECKED;
			packet_loaded_++;
		}
	}
}

void dht_send_file::received(size_t index) { 
	if (map_index_counter_.find(index) !=
		map_index_counter_.end())
	{
		map_index_counter_[index]++;
	} else {
		map_index_counter_.insert(
		std::make_pair<size_t, size_t>(
			index,
			1));
		check(index);
	}
	if (map_index_counter_[index] > 10) {
		map_index_counter_.erase(
		map_index_counter_.find(index));
		map_state_[index] = CRYPTED;
	}
}

void dht_send_file::run_once(boost::asio::deadline_timer* t) {
	std::map<size_t, upload_state_t>::iterator ite;
	ite = map_state_.begin();
	for (int i = 0; i < 5; ++i) {
		while ((ite != map_state_.end()) && (ite->second == CHECKED))
			++ite;
		if (ite == map_state_.end()) {
			if (i == 0) {
				end_ = true;
				delete t;
				return;
			}
			break;
		}
		switch (ite->second) {
			case WAIT :
				load(ite->first);
				++ite;
				break;
			case LOADED :
				crypt(ite->first);
				++ite;
				break;
			case CRYPTED :
				upload(ite->first);
				++ite;
				break;
			case UPLOADED :
				received(ite->first);
				++ite;
				break;
			default :
				++ite;
				break;
		}
	}
	if (!stop_) {
		boost::posix_time::ptime now(
			boost::posix_time::microsec_clock::universal_time());
		t->expires_at(now + boost::posix_time::millisec(10));
		t->async_wait(boost::bind(&dht_send_file::run_once, this, t));
	}
}

inline bool is_port_valid(unsigned short port) {
	return (port > 0);
}

#ifdef SEND_MAIN_TEST
int main(int ac, char** av) {
	unsigned short listen = 4048;
	unsigned short port = 0;
	std::string address = "";
	std::string file_name = "";
	bool is_port = false;
	bool is_address = false;
	try {
		boost::program_options::options_description desc("Allowed options");
		desc.add_options()
		("help,h", "produce help message")
		("listen,l", boost::program_options::value<unsigned short>(), 
			"set the listen port")
		("port,p", boost::program_options::value<unsigned short>(), 
			"set the bootstrap port")
		("address,a", boost::program_options::value<std::string>(), 
			"set the bootstrap address")
		("file,f", boost::program_options::value<std::string>(), 
			"file to be send to the DHT")
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
		if (vm.count("listen")) {
			std::cout 
				<< "Listen port was set to "
				<< vm["listen"].as<unsigned short>() 
				<< "." << std::endl;
			listen = vm["listen"].as<unsigned short>();
			if (!is_port_valid(listen)) {
				std::cerr << "error: Invalid port [1-65535]." << std::endl;
				return 1;
			}
		} else {
			std::cout
				<< "Listen port was not set use default 4048."
				<< std::endl;
		}
		if (vm.count("port")) {
			std::cout
				<< "Bootstrap port was set to "
				<< vm["port"].as<unsigned short>() 
				<< "." << std::endl;
			port = vm["port"].as<unsigned short>();
			if (!is_port_valid(port)) {
				std::cerr << "error: Invalid port [1-65535]." << std::endl;
				return 1;
			}
			is_port = true;
		} else {
			std::cout
				<< "Boostrap port was not set."
				<< std::endl;
		}
		if (vm.count("address")) {
			std::cout
				<< "Bootstrap address was set to "
				<< vm["address"].as<std::string>() 
				<< "." << std::endl;
			address = vm["address"].as<std::string>();
			is_address = true;
		} else {
			std::cout
				<< "Bootstrap address was not set."
				<< std::endl;
		}
		if (vm.count("file")) {
			std::cout
				<< "File to send set to ["
				<< vm["file"].as<std::string>()
				<< "]." << std::endl;
			file_name = vm["file"].as<std::string>();		
		} else {
			std::cout
				<< "No file specified bailing out!"
				<< std::endl;
			return -1;
		}
		{
			boost::asio::io_service ios;
//			boost::asio::io_service ios_send;
			miniDHT::miniDHT<key_size, token_size>* pDht = NULL;
			pDht = new miniDHT::miniDHT<key_size, token_size>(
				ios, 
				listen);
			if (is_port && is_address) 
				pDht->send_PING(address, port);
			std::cout 
				<< "Waiting to the DHT to settle ("
				<< std::dec 
				<< wait_settle 
				<< " seconds)." << std::endl;
			boost::asio::deadline_timer t(
				ios,
				boost::posix_time::seconds(wait_settle));
			dht_send_file dsf(file_name, pDht);
			t.async_wait(boost::bind(&dht_send_file::run_once, &dsf, &t));
//			boost::thread send_thread(
//				boost::bind(&boost::asio::io_service::run, &ios_send));
			while (!dsf.is_end()) {
				ios.run_one();
			}
			std::cout << std::endl << "Finished." << std::endl;
			return 0;
		}
	} catch (std::exception& e) {
		std::cerr << "exception : " << e.what() << std::endl;
	} catch (const char* msg) {
		std::cerr << "exception : " << msg << std::endl;
	}
	return 0;
}
#endif // SEND_MAIN_TEST

