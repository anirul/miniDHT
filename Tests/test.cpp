/*
 * Copyright (c) 2009-2011, Frederic DUBOUCHET
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

#include "miniDHT.h"

#include <vector>
#include <sstream>
#include <string>

using std::list;
using std::stringstream;
using std::string;
using std::string;
using boost::posix_time::minutes;

const unsigned int key_size = 256;
const unsigned int token_size = 32;

std::vector<miniDHT::miniDHT*> list_ptd;
std::bitset<key_size> g_key = miniDHT::random_bitset<key_size>();
unsigned int g_send_count = 0;
unsigned int g_receive_count = 0;
unsigned int milli_wait = 100;

bool is_port_valid(unsigned short port) {
	return (port > 2048);
}

void store_value(boost::asio::deadline_timer* t);

void myValue (const list<miniDHT::data_item_proto>& b) {
	g_receive_count++;
	static unsigned long lost_nb = 0;
//	if (lost_nb != g_send_count - g_receive_count) {
		lost_nb = g_send_count - g_receive_count;
		double percentage = (double)(lost_nb) / (double)g_send_count;
		percentage *= 100.0;
		std::cout << ">> RECEIVE loose ratio " << lost_nb
			<< "/" << g_send_count
			<< "(" << percentage << "%)"
			<< std::endl;
//	}
	list<miniDHT::data_item_proto>::const_iterator ite = b.begin();
	for (; ite != b.end(); ++ite) {
//		std::cout 
//			<< "<" << ite->time 
//			<< ", " << ite->ttl << "> " 
//			<< "<[" << ite->title 
//			<< "], [" << ite->data << "]>"
//			<< std::endl;
	}
}

void retrieve_value(boost::asio::deadline_timer* t) {
	g_send_count++;
	std::vector<miniDHT::miniDHT*>::iterator ite;
//	int i = 0;
//	for (ite = list_ptd.begin(); ite != list_ptd.end(); ++ite) {
//		std::cout 
//			<< "[" << i++ << "] : nb of data : " 
//			<< (*ite)->storage_size() << " <<" 
//			<< miniDHT::key_to_string((*ite)->get_local_key()) 
//			<< ">> IP " << (*ite)->get_local_endpoint() 
//			<< std::endl;
//	}
	miniDHT::miniDHT::value_callback_t vc = &myValue;
	unsigned long position = random() % list_ptd.size();
	miniDHT::miniDHT* pDHT = list_ptd.at(position);
//	std::cout << "|| node : " << position << " knows : " 
//		<< pDHT->bucket_size() << " nodes." << std::endl;
//	std::cout << std::endl;
//	std::cout << std::endl;
//	std::cout << std::endl;
//	std::cout << "|| find value <<" << miniDHT::key_to_string(g_key) << ">>" << std::endl;
//	std::cout << "|| from node  <<" << miniDHT::key_to_string(pDHT->get_local_key()) << ">>" << std::endl;
	pDHT->iterativeFindValue(miniDHT::key_to_string(g_key), vc, "test");
	t->expires_at(t->expires_at() + boost::posix_time::millisec(milli_wait));
	static int local_count = 0;
	local_count++;
	if (local_count % 10)
		t->async_wait(boost::bind(retrieve_value, t));
	else 
		t->async_wait(boost::bind(store_value, t));
}

void store_value(boost::asio::deadline_timer* t) {
	g_key = miniDHT::random_bitset<key_size>();
	std::cout 
		<< std::endl
		<< "<< STORE! <<" << miniDHT::key_to_string(g_key) 
		<< ">>" << std::endl;
	string test = "<<";
	string hkey = miniDHT::key_to_string(g_key);
	test += string(hkey.begin(), hkey.end());
	test += ">> Hello World!";
	miniDHT::data_item_proto data;
	data.set_ttl(minutes(5).total_seconds());
	data.set_time(miniDHT::to_time_t(miniDHT::update_time()));
	data.set_title(string("test.message"));
	data.set_data(&test[0], test.size());
	miniDHT::miniDHT* pDHT = list_ptd.at(random() % list_ptd.size());
//	std::list<std::string> ls = pDHT->nodes_description();
//	std::cout 
//		<< "get local ID <<"
//		<< miniDHT::key_to_string<key_size>(pDHT->get_local_key())
//		<< ">> & IP " << pDHT->get_local_endpoint() << std::endl;
//	for (std::list<std::string>::iterator ite = ls.begin(); ite != ls.end(); ++ite)
//		std::cout << (*ite) << std::endl;
	pDHT->iterativeStore(miniDHT::key_to_string(g_key), data);
	std::cout << std::endl;
	t->expires_at(t->expires_at() + boost::posix_time::millisec(milli_wait));
	t->async_wait(boost::bind(retrieve_value, t));
}

void start_operation(boost::asio::deadline_timer* t) 
{
	t->expires_at(t->expires_at() + boost::posix_time::millisec(100));
	t->async_wait(boost::bind(store_value, t));
} 

int main(int ac, char** av) {
	try {
		// nb of server
		short port = 4048;
		int nb = 2;
		boost::program_options::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "produce help message")
			("listen,l", boost::program_options::value<unsigned short>(), 
				"set the listen port")
			("nb-servers,n", boost::program_options::value<unsigned int>(), 
				"set the number of virtual server")
		;
		boost::program_options::variables_map vm;
		boost::program_options::store(
			boost::program_options::parse_command_line(ac, av, desc), 
			vm);
		if (vm.count("help")) {
			std::cout << desc << std::endl;
			return 1;
		}
		if (vm.count("listen")) {
			std::cout 
				<< "Listen port was set to "
				<< vm["listen"].as<unsigned short>() 
				<< "." << std::endl;
			port = vm["listen"].as<unsigned short>();
			if (!is_port_valid(port)) {
				std::cerr << "error: Invalid port ]2048-65535]." << std::endl;
				return 1;
			}
		} else {
			std::cout << desc << std::endl;
			std::cerr 
				<< "error: you have to specify the listen port." 
				<< std::endl;
			return -1;
		}
		if (vm.count("nb-servers")) {
			std::cout
				<< "starting " 
				<< vm["nb-servers"].as<unsigned int>()
				<< " virtual servers."
				<< std::endl;
			nb = vm["nb-servers"].as<unsigned int>();
		} else {
			std::cout << desc << std::endl;
			std::cerr 
				<< "error : you have to specify the number of virtual servers."
				<< std::endl;
			return -2;
		}
		if ((nb < 1) || (nb > 1000)) {
			std::cout << desc << std::endl;
			std::cerr 
				<< "error : nb should be [1-1000]."
				<< std::endl;
			return -3;
		}
		boost::asio::io_service io_service;
		boost::asio::ip::tcp::endpoint ep(
			boost::asio::ip::tcp::v4(),
			port);
		miniDHT::miniDHT* root = new miniDHT::miniDHT(io_service, ep);
		list_ptd.push_back(root);
		string previous_port = "";
		{
			std::stringstream ss("");
			ss << port;
			previous_port = ss.str();
		}
		std::list<std::string> list_name;
		std::list<std::string> list_port;
		for (int i = 1; i < nb; ++i) {
			stringstream ss("");
			port++;
			ss << port;
			string next_port = ss.str();
			miniDHT::miniDHT* ptd = NULL;
			list_name.push_back(std::string("localhost"));
			list_port.push_back(previous_port);
			boost::asio::ip::tcp::endpoint ep(
				boost::asio::ip::address::from_string("localhost"),
				atoi(next_port.c_str()));
			ptd = new miniDHT::miniDHT(io_service, ep);
			ptd->send_PING(
				miniDHT::create_endpoint_proto(
					string("localhost"), // list_name, 
					previous_port)); // list_port);
			previous_port = next_port;
			list_ptd.push_back(ptd);
		}
		boost::asio::deadline_timer t(
			io_service, 
			boost::posix_time::seconds(10));
		t.async_wait(boost::bind(start_operation, &t));
		srandom((unsigned int)time(0));
		io_service.run();
	} catch (std::exception& e) {
		std::cerr << "Exception : " << e.what() << std::endl;
	}
	return 0;
}

