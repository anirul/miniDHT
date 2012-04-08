/*
 * Copyright (c) 2010-2011, anirul
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

#ifndef RECV_MAIN_TEST
#include <wx/wx.h>
#endif

#include <openssl/sha.h>
#include <openssl/evp.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <list>
#include <limits>

#include <string.h>
#include <stdio.h>

#include "aes_crypt.h"
#include "miniDHT.h"
#include "recv.h"

std::string hex_to_string(const std::string& in) {
	std::string out;
	out.resize(in.size() / 2);
	for (int i = 0; i < in.size(); i += 2) {
		char temp[3];
		memset(temp, 0, 3);
		temp[0] = in[i];
		temp[1] = in[i + 1];
		int val = strtol(temp, NULL, 16);
		out[i / 2] = (char)val;
	}
	return out;
}

std::string dht_recv_file::decode(
	const std::string& key, 
	const std::string& data)
{
	aes_crypt<5, 42> ac(key);
	return ac.decrypt(data);
}

dht_recv_file::dht_recv_file(
	const miniDHT::digest_t& digest,
	miniDHT::miniDHT* pDht,
	const std::string& path)
{
	ofile_ = NULL;
	pDht_ = pDht;
	end_ = false;
	stop_ = false;
	path_ = path;
	packet_total_ = 0;
	packet_loaded_ = 0;
	total_size_ = 0;	
	digest_ = digest;
	// try to get the 5 first part (don't know yet how many there is)
	for (unsigned int i = 0; i < 5; ++i)
		map_state_.insert(std::make_pair<size_t, download_state_t>(
			i,
			WAIT));
}

dht_recv_file::~dht_recv_file() {
	if (ofile_)
		fclose(ofile_);
	ofile_ = NULL;
}

void dht_recv_file::download(size_t index) {
	std::string storage_string_id;
	std::string key_string_id;
	{ // create the id
		std::stringstream ss("");
		ss << digest_ << " " << std::dec << index;
		storage_string_id = ss.str();
	}
	{ // create string from digest
		std::stringstream ss("");
		ss << miniDHT::digest_key_from_string<key_size>(storage_string_id);
		key_string_id = ss.str();
	}
	// store string -> index
	map_key_string_id_.insert(std::make_pair<std::string>(
			key_string_id,
			index));
	// ask for it!
	pDht_->iterativeFindValue(
		miniDHT::key_to_string(
			miniDHT::digest_key_from_string<key_size>(storage_string_id)),
		boost::bind(&dht_recv_file::found, this, _1));
	map_state_[index] = ASKED;
}

void dht_recv_file::received(size_t index) { 
	if (map_index_counter_.find(index) !=
		map_index_counter_.end())
	{
		map_index_counter_[index]++;
	} else {
		map_index_counter_.insert(
		std::make_pair<size_t, size_t>(index, 1));
	}
	// timeout didn't receive yet so ask for it again
	if (map_index_counter_[index] > 20) {
#ifdef RECV_MAIN_TEST
		std::cout 
			<< std::endl 
			<< "Warning packet " 
			<< index 
			<< " missing requery." << std::endl;
#endif
		map_index_counter_.erase(
		map_index_counter_.find(index));
		map_state_[index] = WAIT;
	}
}

void dht_recv_file::decrypt(size_t index) {
	std::string key;
	{
		std::stringstream ss("");
		ss << digest_;
		key = ss.str();
	}
	map_crypt_.insert(
		std::pair<size_t, std::string>(
			index, 
			decode(key, map_load_[index])));
	map_state_[index] = DECRYPTED;
}

void dht_recv_file::write(size_t index) {
	std::string full_path = path_ + file_name_;
	if (!ofile_) 
		ofile_ = fopen(full_path.c_str(), "wb");
	if (!ofile_) 
		throw std::runtime_error(
			full_path + std::string(" is an invalid file, cannot write."));
	fseeko(ofile_, (index * buf_size), SEEK_SET);
	std::string buffer = map_crypt_[index];
	fwrite(&buffer[0], 1, buffer.size(), ofile_);
	map_crypt_.erase(index);
	map_load_.erase(index);
	map_state_[index] = WRITTEN;
	packet_loaded_++; 
}

void dht_recv_file::check() {
#ifdef RECV_MAIN_TEST
	std::cout 
		<< std::endl
		<< "Check DIGEST of file ["
		<< file_name_
		<< "] please wait." << std::endl;
#endif
	if (ofile_) fclose(ofile_);
	ofile_ = NULL;
	miniDHT::digest_t new_digest;
	if (miniDHT::digest_file(new_digest, (path_ + file_name_).c_str()))
#ifdef RECV_MAIN_TEST
		throw std::runtime_error("Could not open file");
#else
		wxMessageBox(
			_("download error, digest missmatch, file may be corrupted!"));
#endif
#ifdef RECV_MAIN_TEST
	std::cout << "asked digest [" << digest_ << "]" << std::endl;
	std::cout << "got  digest  [" << new_digest << "]" << std::endl;
#endif
	if (new_digest == digest_) {
#ifdef RECV_MAIN_TEST
		std::cout << "check ok!" << std::endl;
#endif
	} else {
		std::cerr << "DIGEST MISSMATCH!" << std::endl;
	}
}

void dht_recv_file::found(const std::list<miniDHT::data_item_proto>& b) {
	boost::mutex::scoped_lock lock_it(local_lock_);
	std::list<miniDHT::data_item_proto>::const_iterator ite;
	for (ite = b.begin(); ite != b.end(); ++ite) {
		std::string key;
		{
			std::stringstream ss("");
			ss << digest_;
			key = ss.str();
		}
		std::string title = ite->title();		
		std::string title_string_id;
		size_t packet_number;
		size_t packet_total;
		std::string decrypted_title = decode(key, hex_to_string(title));
		if (!decrypted_title.size()) 
			throw std::runtime_error("unable to decode title");
		{
			std::stringstream ss(decrypted_title);
			ss >> title_string_id;
			if (title_string_id != key) 
				throw std::runtime_error("incorrect packet");
			ss >> packet_number;
			ss >> packet_total;
			size_t pos = 0;
			pos = ss.str().find(" ", pos + 1);
			pos = ss.str().find(" ", pos + 1);
			pos = ss.str().find(" ", pos + 1);
			pos++;
			file_name_ = ss.str().substr(pos);
		}
#ifdef RECV_MAIN_TEST
//		std::cout 
//			<< "\t[" << packet_number 
//			<< "/" << packet_total 
//			<< "] found : [" << file_name_
//			<< "]";
//		std::cout << " OK!" << std::endl;
		std::cout << ".";
#endif
		// update packet total
		if (!packet_total_) {
			packet_total_ = packet_total;
			total_size_ = packet_total_ * buf_size;
			for (int i = 0; i < packet_total; ++i) {
				if (map_state_.find(i) == map_state_.end()) {
					map_state_.insert(
						std::make_pair<size_t, download_state_t>(
							i,
							WAIT));
				}
			}
		}
		std::map<size_t, download_state_t>::iterator dite;
		for (dite = map_state_.begin(); dite != map_state_.end(); ++dite) {
			if (dite->first >= packet_total) {
				map_state_.erase(dite->first);
				dite = map_state_.begin();
			}
		}		 
		map_load_.insert(std::pair<size_t, std::string>(
				packet_number,
				ite->data()));
		map_state_[packet_number] = DOWNLOADED;
		return;
	}
}

void dht_recv_file::run_once(boost::asio::deadline_timer* t) {
	boost::mutex::scoped_lock lock_it(local_lock_);
	assert(t != NULL);
	std::map<size_t, download_state_t>::iterator ite;
	ite = map_state_.begin();
	for (int i = 0; i < 5; ++i) {
		while ((ite != map_state_.end()) && (ite->second == WRITTEN))
			++ite;
		if (ite == map_state_.end()) {
			if (i == 0) {
				end_ = true;
				check();
				return;
			}
			break;
		}
		switch (ite->second) {
			case WAIT :
				download(ite->first);
				++ite;
				break;
			case ASKED :
				received(ite->first);
				++ite;
				break;
			case DOWNLOADED :
				decrypt(ite->first);
				++ite;
				break;
			case DECRYPTED :
				write(ite->first);
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
		t->async_wait(boost::bind(&dht_recv_file::run_once, this, t));
	}
}

inline bool is_port_valid(unsigned short port) {
	return (port > 0);
}

#ifdef RECV_MAIN_TEST
int main(int ac, char** av) {
	unsigned short listen = 4048;
    unsigned short port = 0;
    std::string address = "";
    std::string digest_string = "";
    miniDHT::digest_t digest;
    bool is_port = false;
    bool is_address = false;
    try {
        boost::program_options::options_description desc("Allowed options");
        desc.add_options()
        ("help,h", "produce help message")
        ("listen,l", boost::program_options::value<unsigned short>(), "set the listen port")
        ("port,p", boost::program_options::value<unsigned short>(), "set the bootstrap port")
        ("address,a", boost::program_options::value<std::string>(), "set the bootstrap address")
		  ("digest,d", boost::program_options::value<std::string>(), "digest of the file to be received")
        ;
        boost::program_options::variables_map vm;
        boost::program_options::store(
			boost::program_options::command_line_parser(ac, av).options(desc).run(), 
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
		if (vm.count("digest")) {
			std::cout
				<< "Digest of the file ["
				<< vm["digest"].as<std::string>()
				<< "]." << std::endl;
			digest_string = vm["digest"].as<std::string>();
			std::stringstream ss(digest_string);
			ss >> digest;
		} else {
			std::cerr
				<< "No digest secified bailing out!"
				<< std::endl;
			return -1;
		}
      {
         boost::asio::io_service ios;
			miniDHT::miniDHT* pDht = NULL;
         boost::asio::deadline_timer t(
				ios, 
				boost::posix_time::seconds(wait_settle));
			boost::asio::ip::tcp::endpoint ep(
				boost::asio::ip::tcp::v4(),
				listen);
			pDht = new miniDHT::miniDHT(ios, ep);
			if (is_port && is_address)
         	pDht->send_PING(miniDHT::create_endpoint_proto(address, port));
			std::cout 
				<< "Waiting to the DHT to settle ("
				<< std::dec 
				<< wait_settle 
				<< " seconds)." << std::endl;
			dht_recv_file drf(digest, pDht);
			t.async_wait(boost::bind(&dht_recv_file::run_once, &drf, &t));
			while (!drf.is_end()) {
				ios.run_one();
			}
			std::cout << std::endl << "Finished." << std::endl;
			return 0;
		}
	} catch (std::exception& e) {
		std::cerr << "error: " << e.what() << std::endl;
	}
	return 0;
}
#endif // RECV_MAIN_TEST

