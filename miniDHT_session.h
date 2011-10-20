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
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace miniDHT {

	class basic_message {
	public :
		enum { header_length = 16 };
		enum { max_body_length = (1024 * 1024) - header_length };
	public :
		basic_message() :	data_(NULL), body_length_(0), listen_port_(0) {}
		~basic_message();
		basic_message(const basic_message& msg);
		basic_message& operator=(const basic_message& other);
		const char* data() const;
		char* data();
		size_t length() const;
		const char* body() const;
		char* body();
		size_t body_length() const;
		void body_length(size_t new_length);
		unsigned short listen_port() const;
		void listen_port(unsigned short port);
		bool decode_header();
		void encode_header();
	private :
		char* data_;
		size_t body_length_;
		unsigned short listen_port_;
	};
	
	class session {
	public :
		typedef 
			boost::function<void (
				const boost::asio::ip::tcp::endpoint& ep, 
				const basic_message& s)>
			got_message_callback_t;
		typedef 
			std::map<
				boost::asio::ip::tcp::endpoint, 
				boost::shared_ptr<session> > 
			map_endpoint_session_t;
		typedef std::deque<basic_message> deque_basic_message_t;
		typedef
			std::map<
				boost::asio::ip::tcp::endpoint, 
				boost::shared_ptr<session> >::iterator
			map_endpoint_session_iterator;
	public :
		virtual void start() = 0;
		virtual void connect(const boost::asio::ip::tcp::endpoint& ep) = 0;
		virtual void deliver(const basic_message& msg) = 0;
		virtual boost::asio::ip::tcp::socket& socket() = 0;
	};

	typedef boost::shared_ptr<session> session_ptr;

	class tcp_session : 
		public session,
		public boost::enable_shared_from_this<tcp_session> {
	protected:
		boost::asio::ip::tcp::socket socket_;
		boost::asio::ip::tcp::endpoint ep_;
		basic_message read_msg_;
		deque_basic_message_t write_msgs_;
		bool connect_;
		bool connected_;
		got_message_callback_t got_message_callback_;
		map_endpoint_session_t& map_endpoint_session_;
//		boost::mutex local_lock_;
	public:
		tcp_session(
			boost::asio::io_service& io_service,
			const got_message_callback_t& c,
			map_endpoint_session_t& map)
			:	socket_(io_service),
				connect_(false),
				connected_(false),
				got_message_callback_(c),
				map_endpoint_session_(map) {}
		boost::asio::ip::tcp::socket& socket();
		void start();
		void connect(const boost::asio::ip::tcp::endpoint& ep);
		void deliver(const basic_message& msg);
	protected :
		void cleanup();
		void handle_connect(const boost::system::error_code& error);
		void handle_read_header(
			const boost::system::error_code& error,
			size_t bytes_recvd);
		void handle_read_body(
			const boost::system::error_code& error,
			size_t bytes_recvd);
		void handle_write(const boost::system::error_code& error);
	};

} // end namespace

#endif // MINIDHT_SESSION_HEADER_DEFINED

