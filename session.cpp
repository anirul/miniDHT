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
 * DISCLAIMED. IN NO EVENT SHALL Frederic DUBOUCHET BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "miniDHT_session.h"

#define PACKET_SIZE (1024 * 1024)

boost::asio::io_service ios;
boost::asio::ip::tcp::acceptor* acceptor;
miniDHT::session::map_endpoint_session_t map_endpoint_session;
unsigned short listen_port;

void start_accept();
void handle_accept(
	miniDHT::session_ptr ps, 
	const boost::system::error_code& error);
void handle_receive(
	const boost::asio::ip::tcp::endpoint& ep,
	const miniDHT::basic_message& s);

void start_accept() {
	miniDHT::session_ptr accept_session( 
		new miniDHT::tcp_session(
			ios,
			handle_receive,
			map_endpoint_session));
	acceptor->async_accept(
		accept_session->socket(),
		boost::bind(
			&handle_accept, accept_session, 
			boost::asio::placeholders::error));
}

void handle_accept(
	miniDHT::session_ptr ps,
	const boost::system::error_code& error)
{
	if (!error)
		ps->start();
	start_accept();
}

void handle_receive(
	const boost::asio::ip::tcp::endpoint& ep,
	const miniDHT::basic_message& s)
{
	std::cout << "receive packet from : " << ep << std::endl;
	std::cout << std::string(s.body(), s.body_length()) << std::endl;
	miniDHT::session::map_endpoint_session_iterator ite = 
		map_endpoint_session.find(ep);
	if (ite == map_endpoint_session.end()) {
		std::cout 
			<< "could not find : " << ep 
			<< " in the session list..." << std::endl;
		return;
	}
	miniDHT::basic_message msg;
	msg.body_length(s.body_length() - 1);
	memcpy(msg.body(), &s.body()[1], msg.body_length());
	msg.listen_port(listen_port);
	msg.encode_header();
	if (msg.body_length() > 0)
		ite->second->deliver(msg);
}

bool is_port_valid(unsigned short port) {
	return ((port > 0) && (port < 0xc000));
}

int main(int ac, char** av) {
	unsigned short port = 0;
	unsigned short listen = 4048;
	std::string address = "";
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
				<< "Bootstrap port was not set."
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
		{ // listen to the socket	
			boost::asio::ip::tcp::endpoint ep(
				boost::asio::ip::tcp::v4(), 
				listen);
			listen_port = listen;
			acceptor = new boost::asio::ip::tcp::acceptor(ios, ep);
			start_accept();
			if (is_address && is_port) {
				if (address == std::string("0.0.0.0"))
					address = std::string("127.0.0.1");
				boost::asio::ip::tcp::endpoint uep;
				{
					std::stringstream ss("");
					ss << port;
					boost::asio::ip::tcp::resolver resolver(ios);
					boost::asio::ip::tcp::resolver::query query(
						boost::asio::ip::tcp::v4(),
						address,
						ss.str());
					boost::asio::ip::tcp::resolver::iterator iterator =
						resolver.resolve(query);
					uep = *iterator;
				}
				miniDHT::session::map_endpoint_session_iterator ite = 
					map_endpoint_session.find(uep);
				std::string hello = "hello world!";
				miniDHT::basic_message msg;
				msg.body_length(hello.size());
				memcpy(msg.body(), hello.c_str(), hello.size());
				msg.listen_port(listen);
				msg.encode_header();
				if (ite == map_endpoint_session.end()) {
					miniDHT::session_ptr new_session( 
						new miniDHT::tcp_session(
							ios,
							handle_receive,
							map_endpoint_session));
					new_session->connect(uep);
					new_session->deliver(msg);
				} else {
					ite->second->deliver(msg);
				}
				ios.run();
			} else {
				ios.run();
			}
		}
	} catch (std::exception& e) {
		std::cerr << "error: " << e.what() << std::endl;
		return -1;
	}
	return 0;
}

