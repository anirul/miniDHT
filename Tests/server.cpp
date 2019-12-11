/*
 * Copyright (c) 2010-2019, anirul
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

#include <vector>
#include <sstream>
#include <string>

#include "miniDHT.h"

miniDHT::miniDHT* pDht = nullptr;
bool g_enable_ansi = false;

bool is_port_valid(unsigned short port) {
	return ((port > 0) && (port < 0xc000));
}

void watch(boost::asio::deadline_timer* t) {
	static std::list<miniDHT::contact_proto> s_ls;
	std::list<miniDHT::contact_proto> ls;
	std::list<miniDHT::contact_proto>::const_iterator ite;
	std::list<miniDHT::contact_proto>::const_iterator s_ite;
	ls = pDht->nodes_description();
	if (ls != s_ls) {
		std::cout << std::endl;
		std::cout << miniDHT::update_time() << std::endl;
		for (ite = ls.begin(); ite != ls.end(); ++ite) {
			bool found_key = false;
			bool found_time = false;
			bool found_endpoint = false;
			for (s_ite = s_ls.begin(); s_ite != s_ls.end(); ++s_ite) {
				if (ite->key() == s_ite->key()) {
					found_key = true;
					if (ite->time() == s_ite->time())
						found_time = true;
					if (ite->ep() == s_ite->ep())
						found_endpoint = true;
				}
			}
			std::cout << "<<";
			if (g_enable_ansi) {
				if (!found_key) {
					std::cout << "\x1B[31m";
				}
			}
			for (int i = 0; i < 4; ++i)
				std::cout << ite->key()[i];
			std::cout << "...";
			size_t last = ite->key().size();
			for (size_t i = last - 4; i < last; ++i)
				std::cout << ite->key()[i];
			if (g_enable_ansi) {
				if (!found_key) {
					std::cout << "\x1B[0m";
				}
			}
			std::cout << ">>";
			std::cout << " - [";
			if (g_enable_ansi) {
				if (!found_time) {
					std::cout << "\x1B[31m";
				}
			}
			{
				time_t time = ite->time();
				boost::posix_time::ptime recv_time = 
					boost::posix_time::from_time_t(time);
				std::cout << recv_time;
			}
			if (g_enable_ansi) {
				if (!found_time) {
					std::cout << "\x1B[0m";
				}
			}
			std::cout << "]"
				<< " - {";
			if (g_enable_ansi) {
				if (!found_endpoint) {
					std::cout << "\x1B[31m";
				}
			}
			std::cout << ite->ep().address() << ":" 
				<< ite->ep().port();
			if (g_enable_ansi) {
				if (!found_endpoint) {
					std::cout << "\x1B[0m";
				}
			}
			std::cout  << "}" << std::endl;
		}
		s_ls = ls;
	}
	t->expires_at(t->expires_at() + boost::posix_time::seconds(1));
	t->async_wait(boost::bind(watch, t));
}

int main(int ac, char** av) {
	unsigned short listen = 4048;
	unsigned short port = 0;
	size_t max_record = -1;
	std::string address = "";
	bool is_port = false;
	bool is_address = false;
	bool is_max_record = false;
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
			("max-record,m", boost::program_options::value<size_t>(),
				"set the maximum number of local record")
			("enable-ansi,s", boost::program_options::value<bool>(),
				"enable use of ANSI as output")
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
				std::cerr << "error: Invalid port [1-49151]." << std::endl;
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
				std::cerr << "error: Invalid port [1-49151]." << std::endl;
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
		if (vm.count("max-record")) {
			std::cout
				<< "Max record was set to "
				<< vm["max-record"].as<size_t>()
				<< "." << std::endl;
			max_record = vm["max-record"].as<size_t>();
			is_max_record = true;
		} else {
			std::cout
				<< "Max record was not set (default is 8Gb)."
				<< std::endl;
		}
		if (vm.count("enable-ansi")) {
			g_enable_ansi = true;
		} else {
			g_enable_ansi = false;
		}
		{
			boost::asio::io_service ios_dht;
			boost::asio::io_service ios_watch;
			boost::asio::ip::tcp::endpoint ep(
				boost::asio::ip::tcp::v4(),
				listen);
			pDht = new miniDHT::miniDHT(ios_dht, ep);
			if (is_port && is_address)
				pDht->send_PING(miniDHT::create_endpoint_proto(address, port));
			if (is_max_record) pDht->set_max_record(max_record);
			boost::asio::deadline_timer t(
				ios_watch, 
				boost::posix_time::seconds(5));
			t.async_wait(boost::bind(watch, &t));
			boost::thread watch_thread(
				boost::bind(&boost::asio::io_service::run, &ios_watch));
			boost::thread dht_thread(
				boost::bind(&boost::asio::io_service::run, &ios_dht));
			watch_thread.join();
			dht_thread.join();
		}
	} catch (std::exception& e) {
		std::cerr << "error: " << e.what() << std::endl;
		return -1;
	}
    return 0;
}

