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

namespace miniDHT {

	basic_message::~basic_message() {
			if (data_) delete [] data_;
			data_ = NULL;
	}

	basic_message::basic_message(const basic_message& msg) {
		body_length_ = msg.body_length_;
		listen_port_ = msg.listen_port_;
		if (body_length_ && msg.data_) {
			data_ = new char[header_length + body_length_];
			memcpy(data_, msg.data_, length());
		}
	}

	basic_message& basic_message::operator=(const basic_message& other) {
		if (this != &other) {
			body_length_ = other.body_length_;
			listen_port_ = other.listen_port_;
			if (body_length_ && other.data_) {
				if (data_) delete [] data_;
				data_ = new char[header_length + body_length_];
				memcpy(data_, other.data_, length());
			}
		}
		return *this;
	}

	const char* basic_message::data() const {
		if (!data_) 
			throw std::runtime_error(
				"basic_message::data() const, has no memory allocated.");
		return data_;
	}

	char* basic_message::data() {
		if (!data_)
			body_length(max_body_length);
		return data_;
	}

	size_t basic_message::length() const {
		return header_length + body_length_;
	}

	const char* basic_message::body() const {
		if (!data_) 
			throw std::runtime_error(
				"basic_message::body() const, has no memory allocated.");
		return data_ + header_length;
	}
		
	char* basic_message::body() {
		if (!data_) 
			throw std::runtime_error(
				"basic_message::body(), has no memory allocated.");
		return data_ + header_length;
	}

	size_t basic_message::body_length() const {
		return body_length_;
	}

	void basic_message::body_length(size_t new_length) {
		body_length_ = new_length;
		if (body_length_ > max_body_length)
			body_length_ = max_body_length;
		if (data_) delete [] data_;
		data_ = new char[header_length + body_length_];
	}

	unsigned short basic_message::listen_port() const {
		return listen_port_;
	}

	void basic_message::listen_port(unsigned short port) {
		listen_port_ = port;
	}

	bool basic_message::decode_header() {
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
		
	void basic_message::encode_header() {
		char part1[(header_length / 2) + 1] = "";
		char part2[(header_length / 2) + 1] = "";
		std::sprintf(part1, "%8d", (int)body_length_);
		std::memcpy(data_, part1, header_length / 2);
		std::sprintf(part2, "%8d", (int)listen_port_);
		std::memcpy(&data_[header_length / 2], part2, header_length / 2);
	}

	boost::asio::ip::tcp::socket& tcp_session::socket() {
//		boost::mutex::scoped_lock lock_it(local_lock_);
		return socket_;
	}

	void tcp_session::start() {
//		boost::mutex::scoped_lock lock_it(local_lock_);
		connected_ = true;
		if (!connect_) ep_ = socket_.remote_endpoint();
		std::cout << "start : " << ep_ << std::endl;
		boost::asio::async_read(
			socket_,
			boost::asio::buffer(
				read_msg_.data(), 
				read_msg_.header_length),
			boost::bind(
				&tcp_session::handle_read_header,
				this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	void tcp_session::connect(const boost::asio::ip::tcp::endpoint& ep) {
//		boost::mutex::scoped_lock lock_it(local_lock_);
		connect_ = true;
		ep_ = ep;
		socket_.async_connect(
			ep,
			boost::bind(
				&tcp_session::handle_connect, 
				this,
				boost::asio::placeholders::error));
	}

	void tcp_session::deliver(const basic_message& msg) {
//		boost::mutex::scoped_lock lock_it(local_lock_);
		bool write_in_progress = !write_msgs_.empty();
		write_msgs_.push_back(msg);
		if (!write_in_progress && connected_) {
			boost::asio::async_write(
				socket_,
				boost::asio::buffer(
					write_msgs_.front().data(), 
					write_msgs_.front().length()),
				boost::bind(
					&tcp_session::handle_write, 
					this,
					boost::asio::placeholders::error));
		}
	}

	void tcp_session::cleanup() {
		{
//			boost::mutex::scoped_lock lock_it(local_lock_);
			map_endpoint_session_iterator ite = 
				map_endpoint_session_.find(ep_);
			if (ite != map_endpoint_session_.end())
				map_endpoint_session_.erase(ite);
		}
	}

	void tcp_session::handle_connect(const boost::system::error_code& error) {
		if (!error) {
			start();
			// TODO send the defer delivers
		} else {
			std::cerr << error << std::endl;
			std::cerr << error.message() << std::endl;
			cleanup();
		}
	}

	void tcp_session::handle_read_header(
		const boost::system::error_code& error,
		size_t bytes_recvd)
	{
		if (!error && read_msg_.decode_header()) {
//			boost::mutex::scoped_lock lock_it(local_lock_);
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
				map_endpoint_session_.erase(ite);
				map_endpoint_session_.insert(std::make_pair(ep_, this));
			}
			boost::asio::async_read(
				socket_,
				boost::asio::buffer(
					read_msg_.body(), 
					read_msg_.body_length()),
				boost::bind(
					&tcp_session::handle_read_body,
					this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		} else {
			cleanup();
		}
	}
		
	void tcp_session::handle_read_body(
		const boost::system::error_code& error,
		size_t bytes_recvd)
	{
		if (!error) {
			got_message_callback_(ep_, read_msg_);
//			boost::mutex::scoped_lock lock_it(local_lock_);
			boost::asio::async_read(
				socket_,
				boost::asio::buffer(
					read_msg_.data(), 
					read_msg_.header_length),
				boost::bind(
					&tcp_session::handle_read_header, 
					this,
						boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		} else {
			cleanup();
		}
	}

	void tcp_session::handle_write(const boost::system::error_code& error) {
		if (!error) {
//			boost::mutex::scoped_lock lock_it(local_lock_);
			write_msgs_.pop_front();
			if (!write_msgs_.empty()) {
				boost::asio::async_write(
					socket_,
					boost::asio::buffer(
						write_msgs_.front().data(), 
						write_msgs_.front().length()),
					boost::bind(
						&tcp_session::handle_write, 
						this,
						boost::asio::placeholders::error));
			}
		} else {
			cleanup();
		}
	}
}

