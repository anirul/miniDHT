/*
 * Copyright (c) 2010, anirul
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

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <list>
#include <limits>

#include <string.h>
#include <stdio.h>

#include "miniDHT.h"

const unsigned int key_size = 256;
const unsigned int token_size = 32;
const unsigned int wait_settle = 10;
const size_t buf_size = 8192;

std::string g_file_name = "";
std::string g_digest_string = "";
FILE* g_ofile = NULL;
size_t g_total_size = std::numeric_limits<std::size_t>::max();
miniDHT::miniDHT<key_size, token_size>* g_pDht = NULL;
miniDHT::digest_t g_digest;

enum query_state {
	NOT_QUERY = 0,
	QUERY = 1,
	RECEIVED = 2,
	SAVED = 3,
	DONT_EXIST = 4
};

std::map<size_t, query_state> g_query_state_map;

bool is_port_valid(unsigned short port) {
	return (port > 0);
}

void got_value(const std::list<miniDHT::data_item_t>& b) {
	std::ofstream ofs;
	std::list<miniDHT::data_item_t>::const_iterator ite = b.begin();
	for (;ite != b.end(); ++ite) {
		std::string title = ite->title;
		std::cout 
			<< "Recieved packet : "
			<< title
			<< std::endl;
		std::stringstream ss("");
		ss << title;
		miniDHT::digest_t digest;
		ss >> digest;
		if (digest == g_digest) {
			size_t part = 0;
			char colon = ' ';
			ss >> colon;
			ss >> part;
			ss >> colon;
			ss >> g_total_size;
			std::cout 
				<< "RECEIVED part : " << part
				<< "\t / \t" << g_total_size
				<< "\tsize : " << ite->data.size() << std::endl;
			g_query_state_map[part] = RECEIVED;
			fseeko(g_ofile, part * buf_size, SEEK_SET);
			fwrite(&(ite->data)[0], sizeof(char), ite->data.size(), g_ofile);
			g_query_state_map[part] = SAVED;
			if (ite->data.size() < buf_size) {
				g_query_state_map[part + 1] = DONT_EXIST;
			}
		} else {
			std::cout << "Invalid part digest : " << std::endl;
			std::cout << "\t" << digest << std::endl;
			std::cout << "\t" << g_digest << std::endl;
			std::cout << "\t are different." << std::endl;
		}
	}
}

void recv_callback(boost::asio::deadline_timer* t) {
	std::map<size_t, query_state>::iterator ite = g_query_state_map.begin();
	int count_query = 0;
	if (g_total_size < std::numeric_limits<std::size_t>::max()) {
		size_t last_block = (g_total_size / buf_size) + 
			((g_total_size % buf_size) ? 1 : 0);
		g_query_state_map.insert(
			std::make_pair<size_t, query_state>(last_block + 1, DONT_EXIST));
		for (size_t i = 0; i < last_block + 1; ++i)
			if (g_query_state_map.find(i) == g_query_state_map.end())
				g_query_state_map.insert(
					std::make_pair<size_t, query_state>(i, NOT_QUERY));
	}
	while (ite != g_query_state_map.end() && ite->second == SAVED) ite++;
	if (ite != g_query_state_map.end() && ite->second == DONT_EXIST) {
		std::cout << "Finished." << std::endl;
		fclose(g_ofile);
		exit(0);
	}
	while (ite != g_query_state_map.end() && ite->second != NOT_QUERY) {
		if (ite->second == QUERY)
			count_query += 1;
		if (ite->second == DONT_EXIST || count_query > 10) {
			std::map<size_t, query_state>::iterator ite2 =
				g_query_state_map.begin();
			for (; ite2 != g_query_state_map.end(); ite2++) {
				if (ite2->second == QUERY) {
					ite2->second = NOT_QUERY;
					break;
				}
			}
			ite = ite2;
			break;
		}
		ite++;
	}
	if (ite == g_query_state_map.end()) {
		g_query_state_map.insert(
			std::make_pair<size_t, query_state>(ite->first + 1, NOT_QUERY));
		ite++;
	}
	if (ite->second == NOT_QUERY) {
		std::stringstream ss("");
		ss << g_digest << ":" << std::dec << ite->first;

		std::cout 
			<< "Query part : (" 
			<< std::dec << ite->first << ")." << std::endl;
		ite->second = QUERY;

		std::bitset<key_size> key = 
			miniDHT::digest_key_from_string<key_size>(ss.str());

		miniDHT::miniDHT<key_size, token_size>::value_callback_t vc = 
			&got_value;
		g_pDht->iterativeFindValue(key, vc, ss.str());
	}
	t->expires_at(t->expires_at() + boost::posix_time::millisec(100));
	t->async_wait(boost::bind(recv_callback, t));
}


int main(int ac, char** av) {
	unsigned short listen = 4048;
    unsigned short port = 0;
    std::string address = "";
    bool is_port = false;
    bool is_address = false;
    try {
        boost::program_options::options_description desc("Allowed options");
        desc.add_options()
        ("help,h", "produce help message")
        ("listen,l", boost::program_options::value<unsigned short>(), "set the listen port")
        ("port,p", boost::program_options::value<unsigned short>(), "set the bootstrap port")
        ("address,a", boost::program_options::value<std::string>(), "set the bootstrap address")
        ("file,f", boost::program_options::value<std::string>(), "file to be send to the DHT")
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
        if (vm.count("file")) {
            std::cout
                << "File to receive ["
                << vm["file"].as<std::string>()
                << "]." << std::endl;
            g_file_name = vm["file"].as<std::string>();
        } else {
            std::cerr
                << "No file specified bailing out!"
                << std::endl;
            return -1;
        }
		if (vm.count("digest")) {
			std::cout
				<< "Digest of the file ["
				<< vm["digest"].as<std::string>()
				<< "]." << std::endl;
			g_digest_string = vm["digest"].as<std::string>();
			std::stringstream ss(g_digest_string);
			ss >> g_digest;
			std::cout << g_digest << std::endl;
		} else {
			std::cerr
				<< "No digest secified bailing out!"
				<< std::endl;
			return -1;
		}
        {
            boost::asio::io_service ios;
            boost::asio::deadline_timer t(
                ios, 
                boost::posix_time::seconds(wait_settle));
            if (is_port && is_address) {
                std::stringstream ss("");
                ss << port;
                g_pDht = new miniDHT::miniDHT<key_size, token_size>(
                    ios,
                    listen,
                    address,
                    ss.str());
            } else {
                g_pDht = new miniDHT::miniDHT<key_size, token_size>(ios, listen);
            }
            std::cout 
                << "Waiting to the DHT to settle ("
				<< std::dec 
                << wait_settle 
                << " seconds)." << std::endl;
			g_ofile = fopen(g_file_name.c_str(), "wb");
            t.async_wait(boost::bind(recv_callback, &t));
            ios.run();
        }
    } catch (std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
    }
    return 0;
}