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
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include "miniDHT_proto.pb.h"
#include "miniDHT_const.h"

namespace miniDHT {

	template <size_t PACKET_SIZE = 1024 * 1024>		
	class basic_message {

	public :

		enum { header_length = 16 };
		enum { max_body_length = PACKET_SIZE - header_length };
	
		basic_message()
			: data_(NULL), body_length_(0), listen_port_(0) 
		{
			data_ = new char[PACKET_SIZE];
		}

		basic_message(size_t size) 
			: data_(NULL), body_length_(0), listen_port_(0)
		{
			body_length(size);
		}

		basic_message(const basic_message<PACKET_SIZE>& m) 
			: data_(NULL), body_length_(0), listen_port_(0)
		{
			body_length(m.body_length());
			listen_port_ = m.listen_port_;
			memcpy(data_, m.data_, length());
		}

		basic_message<PACKET_SIZE>& operator=(const basic_message<PACKET_SIZE>& m) {
			body_length(m.body_length());
			listen_port_ = m.listen_port_;
			memcpy(data_, m.data_, length());
			return *this;	
		}

		virtual ~basic_message() {
			if (data_)
				delete [] data_;
			data_ = NULL;
		}

		const char* data() const {
			if (!data_) throw std::runtime_error("empty data");
			return data_;
		}

		char* data() {
			if (!data_) throw std::runtime_error("empty data");
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
			if (data_) {
				delete [] data_;
			}
			data_ = new char[length()];
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

		char* data_;
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
			boost::function<void (const boost::asio::ip::tcp::endpoint& ep)>
			got_disconnected_callback_t;
		typedef 
			std::map<endpoint_proto, session<PACKET_SIZE>*> 
			map_ep_proto_session_t;
		typedef std::deque<basic_message<PACKET_SIZE> > deque_basic_message_t;
		typedef typename
			std::map<endpoint_proto, session<PACKET_SIZE>*>::iterator
			map_ep_proto_session_iterator;

	protected:

		boost::asio::ip::tcp::socket socket_;
		boost::asio::ip::tcp::endpoint ep_;
		basic_message<PACKET_SIZE> read_msg_;
		deque_basic_message_t write_msgs_;
		bool connect_;
		int ref_count_;
		got_message_callback_t got_message_callback_;
		got_disconnected_callback_t got_disconnected_callback_;
		map_ep_proto_session_t& map_ep_proto_session_;
		boost::mutex local_lock_;

	public:

		session(
			boost::asio::io_service& io_service,
			const got_message_callback_t& c,
			const got_disconnected_callback_t& d,
			map_ep_proto_session_t& map)
			:	socket_(io_service),
				connect_(false),
				ref_count_(1),
				got_message_callback_(c),
				got_disconnected_callback_(d),
				map_ep_proto_session_(map) {}

		boost::asio::ip::tcp::socket& socket() {
			boost::mutex::scoped_lock lock_it(local_lock_);
			return socket_;
		}

		void start() {
			boost::mutex::scoped_lock lock_it(local_lock_);
			if (!connect_) ep_ = socket_.remote_endpoint();
			ref_count_ += 1;
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
			ref_count_ += 1;
			socket_.async_connect(
				ep,
				boost::bind(
					&session::handle_connect, 
					this,
					boost::asio::placeholders::error));
		}

		void deliver(const basic_message<PACKET_SIZE>& msg) {
			boost::mutex::scoped_lock lock_it(local_lock_);
			bool write_in_progress = !write_msgs_.empty();
			write_msgs_.push_back(msg);
			if (!write_in_progress) {
				ref_count_ += 1;
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
		}

	protected:

		void release() {
			local_lock_.lock();
			ref_count_ -= 1;
			if (ref_count_ <= 0) {
				map_ep_proto_session_iterator ite = 
					map_ep_proto_session_.find(
						endpoint_to_proto(ep_));
				if (ite != map_ep_proto_session_.end())
					map_ep_proto_session_.erase(ite);
				local_lock_.unlock();
				delete this;
				return;
			}
			local_lock_.unlock();
		}

		void handle_connect(const boost::system::error_code& error) {
			local_lock_.lock();
			ref_count_ -= 1;
			if (!error) {
				local_lock_.unlock();
				start();
			} else {
				local_lock_.unlock();
				release();
			}
		}

		void handle_read_header(
			const boost::system::error_code& error,
			size_t bytes_recvd)
		{
			local_lock_.lock();
			ref_count_ -= 1;
			try {
				if (!error && read_msg_.decode_header()) {
					if (!connect_) { 
						ep_ = boost::asio::ip::tcp::endpoint(
							socket_.remote_endpoint().address(), 
							read_msg_.listen_port());
					}
					map_ep_proto_session_iterator ite = 
						map_ep_proto_session_.find(
							endpoint_to_proto(ep_));
					if (ite == map_ep_proto_session_.end()) {
						map_ep_proto_session_.insert(
							std::make_pair(
								endpoint_to_proto(ep_), 
								this));
					} else {
						if (ite->second != this) {
							map_ep_proto_session_.erase(ite);
							map_ep_proto_session_.insert(
								std::make_pair(
									endpoint_to_proto(ep_), 
									this));
						}
					}
					ref_count_ += 1;
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
					local_lock_.unlock();
				} else {
					std::stringstream ss("");
					ss << "Error in handle_read_header : " << error;
					throw std::runtime_error(ss.str());
				}
			} catch (std::exception& e) {
				local_lock_.unlock();
				std::cerr 
//					<< "Exception (catched) : " << e.what()
					<< " -> " << ep_.address().to_string() << ":" << ep_.port() 
					<< " Disconnected!"
					<< std::endl;
				got_disconnected_callback_(ep_);
				release();
			}
		}
		
		void handle_read_body(
			const boost::system::error_code& error,
			size_t bytes_recvd)
		{
			local_lock_.lock();
			ref_count_ -= 1;
			if (!error) {
				local_lock_.unlock();
				got_message_callback_(ep_, read_msg_);
				local_lock_.lock();
				ref_count_ += 1;
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
				local_lock_.unlock();
			} else {
				local_lock_.unlock();
				release();
			}
		}

		void handle_write(const boost::system::error_code& error) {
			local_lock_.lock();
			ref_count_ -= 1;
			if (!error) {
				write_msgs_.pop_front();
				if (!write_msgs_.empty()) {
					ref_count_ += 1;
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
				local_lock_.unlock();
			} else {
				local_lock_.unlock();
				release();
			}
		}	
	};

} // end namespace

#endif // MINIDHT_SESSION_HEADER_DEFINED

