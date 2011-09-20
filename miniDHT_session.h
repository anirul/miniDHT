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
 * DISCLAIMED. IN NO EVENT SHALL Frederic DUBOUCHET BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MINIDHT_SESSION_HEADER_DEFINED
#define MINIDHT_SESSION_HEADER_DEFINED

// C++ headers
#include <exception>
#include <string>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <list>
#include <map>
#include <stack>
#include <deque>
#include <vector>
#include <string>
#include <fstream>
// boost headers
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#ifdef SERIALIZE_XML
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif
#ifdef SERIALIZE_BINARY
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#endif
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>

#include "miniDHT.h"

namespace miniDHT {

	template <size_t PACKET_SIZE = 1024 * 1024>		
	class basic_message {

	public :

		enum { header_length = 16 };
		enum { max_body_length = PACKET_SIZE - header_length };
	
		basic_message()
			: body_length_(0), listen_port_(0) {}

		const char* data() const {
			return data_;
		}

		char* data() {
			return data_;
		}

		size_t length() const {
			return header_length + body_length_;
		}

		const char* body() const {
			return data_ + header_length;
		}
		
		char* body() {
			return data_ + header_length;
		}

		size_t body_length() const {
			return body_length_;
		}

		void body_length(size_t new_length) {
			body_length_ = new_length;
			if (body_length_ > max_body_length)
				body_length_ = max_body_length;
		}

		unsigned short listen_port() const {
			return listen_port_;
		}

		void listen_port(unsigned short port) {
			listen_port_ = port;
		}

		bool decode_header() {
			char part1[(header_length / 2) + 1] = "";
			char part2[(header_length / 2) + 1] = "";
			std::strncat(part1, data_, header_length / 2);
			std::strncat(part2, &data_[header_length / 2], header_length / 2);
			body_length_ = std::atoi(part1);
			listen_port_ = std::atoi(part2);
			if (body_length_ > max_body_length) {
				body_length_ = 0;
				return false;
			}
			return true;
		}
		
		void encode_header() {
			char part1[(header_length / 2) + 1] = "";
			char part2[(header_length / 2) + 1] = "";
			std::sprintf(part1, "%8d", (int)body_length_);
			std::memcpy(data_, part1, header_length / 2);
			std::sprintf(part2, "%8d", (int)listen_port_);
			std::memcpy(&data_[header_length / 2], part2, header_length / 2);
		}

	private:
		char data_[header_length + max_body_length];
		size_t body_length_;
		unsigned short listen_port_;
	};

	template <size_t PACKET_SIZE = 1024 * 1024>		
	class session {

	public:

		typedef 
			boost::function<void (
				const boost::asio::ip::tcp::endpoint& ep, 
				const basic_message<PACKET_SIZE>& s)>
			got_message_callback_t;
		typedef 
			std::map<boost::asio::ip::tcp::endpoint, session<PACKET_SIZE>*> 
			map_endpoint_session_t;
		typedef std::deque<basic_message<PACKET_SIZE> > deque_basic_message_t;
		typedef typename
			std::map<
				boost::asio::ip::tcp::endpoint, 
				session<PACKET_SIZE>*>::iterator
			map_endpoint_session_iterator;

	protected:

		boost::asio::ip::tcp::socket socket_;
		boost::asio::ip::tcp::endpoint ep_;
		basic_message<PACKET_SIZE> read_msg_;
		deque_basic_message_t write_msgs_;
		bool connect_;
		const got_message_callback_t& got_message_callback_;
		map_endpoint_session_t& map_endpoint_session_;

	private:
		
		boost::mutex local_lock_;

	public:

		session(
			boost::asio::io_service& io_service,
			const got_message_callback_t& c,
			map_endpoint_session_t& map)
			:	socket_(io_service),
				connect_(false),
				got_message_callback_(c),
				map_endpoint_session_(map) {}

		boost::asio::ip::tcp::socket& socket() {
			return socket_;
		}

		void start() {
			boost::mutex::scoped_lock lock_it(local_lock_);
			if (!connect_) ep_ = socket_.remote_endpoint();
			std::cout << "session::start():[" << ep_ << "]" << std::endl;
			boost::asio::async_read(
				socket_,
				boost::asio::buffer(
					read_msg_.data(), 
					read_msg_.header_length),
				boost::bind(
					&session::handle_read_header,
					this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}

		void connect(boost::asio::ip::tcp::endpoint& ep) {
			boost::mutex::scoped_lock lock_it(local_lock_);
			connect_ = true;
			ep_ = ep;
			std::cout << "session::connect(" << ep << ")" << std::endl;
			socket_.async_connect(
				ep,
				boost::bind(
					&session::handle_connect, 
					this,
					boost::asio::placeholders::error));
		}

		void deliver(const basic_message<PACKET_SIZE>& msg) {
			boost::mutex::scoped_lock lock_it(local_lock_);
			std::cout << "session::deliver(" << msg.body_length() << ")" << std::endl;
			bool write_in_progress = !write_msgs_.empty();
			write_msgs_.push_back(msg);
			if (!write_in_progress) {
				boost::asio::async_write(
					socket_,
					boost::asio::buffer(
						write_msgs_.front().data(), 
						write_msgs_.front().length()),
					boost::bind(
						&session::handle_write, 
						this,
						boost::asio::placeholders::error));
				write_msgs_.pop_front();
			}
		}

	protected:

		void cleanup() {
			boost::mutex::scoped_lock lock_it(local_lock_);
			std::cout << "session::cleanup()[" << ep_ << "]" << std::endl;
			map_endpoint_session_iterator ite = 
				map_endpoint_session_.find(ep_);
			if (ite != map_endpoint_session_.end())
				map_endpoint_session_.erase(ite);
			delete this;
		}

		void handle_connect(const boost::system::error_code& error) {
			std::cout << "session::handle_connect(" << error << ")" << std::endl;
			if (!error)
				start();
			else
				cleanup();
		}

		void handle_read_header(
			const boost::system::error_code& error,
			size_t bytes_recvd)
		{
			std::cout
				<< "session::handle_read_header("
				<< error << ", " << bytes_recvd << ")"
				<< std::endl;
			if (!error && read_msg_.decode_header()) {
				boost::mutex::scoped_lock lock_it(local_lock_);
				if (!connect_) { 
					ep_ = boost::asio::ip::tcp::endpoint(
						socket_.remote_endpoint().address(), 
						read_msg_.listen_port());
				}
				map_endpoint_session_iterator ite = 
					map_endpoint_session_.find(ep_);
				if (ite == map_endpoint_session_.end()) {
					map_endpoint_session_.insert(std::make_pair(ep_, this));
				} else {
					if (ite->second != this) {
						map_endpoint_session_.erase(ite);
						map_endpoint_session_.insert(std::make_pair(ep_, this));
					}
				}
				boost::asio::async_read(
					socket_,
					boost::asio::buffer(
						read_msg_.body(), 
						read_msg_.body_length()),
					boost::bind(
						&session::handle_read_body,
						this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
			} else {
				cleanup();
			}
		}
		
		void handle_read_body(
			const boost::system::error_code& error,
			size_t bytes_recvd)
		{
			std::cout 
				<< "session::handle_read_body(" 
				<< error << ", " << bytes_recvd << ")" << std::endl;
			if (!error) {
				boost::mutex::scoped_lock lock_it(local_lock_);
				got_message_callback_(ep_, read_msg_);
				boost::asio::async_read(
					socket_,
					boost::asio::buffer(
						read_msg_.data(), 
						read_msg_.header_length),
					boost::bind(
						&session::handle_read_header, 
						this,
							boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
			} else {
				cleanup();
			}
		}

		void handle_write(const boost::system::error_code& error) {
			std::cout << "session::handle_write(" << error << ")";
			if (!error) {
				boost::mutex::scoped_lock lock_it(local_lock_);
				write_msgs_.pop_front();
				if (!write_msgs_.empty()) {
					boost::asio::async_write(
						socket_,
						boost::asio::buffer(
							write_msgs_.front().data(), 
							write_msgs_.front().length()),
						boost::bind(
							&session::handle_write, 
							this,
							boost::asio::placeholders::error));
				}
			} else {
				std::cout << std::endl;
				cleanup();
			}
		}	
	};

} // end namespace

#endif // MINIDHT_SESSION_HEADER_DEFINED

