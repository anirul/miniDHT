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
#include <openssl/sha.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>

#include "miniDHT.h"

const unsigned int key_size = 256;
const unsigned int token_size = 32;
const unsigned int wait_settle = 10;
const size_t buf_size = 8192;

std::string g_file_name = "";
FILE* g_ifile = NULL;
size_t g_total_size = 0;
miniDHT::miniDHT<key_size, token_size>* g_pDht = NULL;
miniDHT::digest_t g_digest;

bool is_port_valid(unsigned short port) {
	return (port > 0);
}

void end_callback(boost::asio::deadline_timer* t) {
	fclose(g_ifile);
	std::cout 
		<< "Final storage digest : " 
		<< g_pDht->sorage_wait_queue() << std::endl;
	std::cout << "Finish!" << std::endl;
	delete g_pDht;
	exit(0);
}

void send_callback(boost::asio::deadline_timer* t) {
	static unsigned int count = 0;
	static size_t total_bytes = 0;
	static char buffer[buf_size];
	static size_t bytes_read = 0;
	static std::string storage_string_id = "";
	static std::string storage_string_title = "";
	size_t storage_digest_count = g_pDht->sorage_wait_queue();
	if (feof(g_ifile)) {
		std::cout 
			<< "Waiting for the end queue... (" 
			<< storage_digest_count
			<< ")"
			<< std::endl;
		t->expires_at(t->expires_at() + 
			boost::posix_time::seconds(storage_digest_count));
		if (storage_digest_count)
			t->async_wait(boost::bind(send_callback, t));
		else
			t->async_wait(boost::bind(end_callback, t));
		return;
	}
	if (!storage_digest_count) {
		std::stringstream ss("");
		ss << g_digest << ":" << std::dec << count++;
		storage_string_id = ss.str();
		ss << ":" << std::dec << g_total_size << ";";
		storage_string_title = ss.str();
		bytes_read = fread(buffer, 1, buf_size, g_ifile);
		total_bytes += bytes_read;
		std::cout 
			<< " total bytes transfered : " << total_bytes 
			<< " / " << g_total_size
			<< std::endl;
		std::cout 
			<< "\ttitle : " 
			<< storage_string_title 
			<< std::endl;
	} else {
		std::cout
			<< "resend last store..."
			<< std::endl;
	}
	if (bytes_read) {
		miniDHT::data_item_t data;
		data.ttl = boost::posix_time::hours(24);
		data.time = miniDHT::update_time();
		data.title = storage_string_title;
		data.data.resize(bytes_read);
		memcpy(&(data.data)[0], buffer, bytes_read);
		g_pDht->iterativeStore(
			miniDHT::digest_key_from_string<key_size>(storage_string_id), 
			data); 
	}
	t->expires_at(t->expires_at() + 
		boost::posix_time::millisec(50));
	t->async_wait(boost::bind(send_callback, t));
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
                << "File to send set to ["
                << vm["file"].as<std::string>()
                << "]." << std::endl;
            g_file_name = vm["file"].as<std::string>();
        } else {
            std::cout
                << "No file specified bailing out!"
                << std::endl;
            return -1;
        }
        {
            boost::asio::io_service ios;
            std::cout 
                << "Compute DIGEST of file [" 
                << g_file_name 
                << "] please wait." << std::endl;
            if (miniDHT::digest_file(g_digest, g_file_name.c_str())) {
                std::cerr << "could not open file." << std::endl;
                return -1;
            }
            std::cout << " -> DIGEST : [" << g_digest << "]" << std::endl;
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
                g_pDht = 
					new miniDHT::miniDHT<key_size, token_size>(ios, listen);
            }
            std::cout 
                << "Waiting to the DHT to settle ("
				<< std::dec 
                << wait_settle 
                << " seconds)." << std::endl;
			g_ifile = fopen(g_file_name.c_str(), "rb");
			if (!g_ifile) {
				std::cout << "Invalid file to be send." << std::endl;
				return -1;
			}
			fseeko(g_ifile, 0, SEEK_END);
			g_total_size = ftello(g_ifile);
			fseeko(g_ifile, 0, SEEK_SET);
            t.async_wait(boost::bind(send_callback, &t));
            ios.run();
        }
    } catch (std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
    }
    return 0;
}
