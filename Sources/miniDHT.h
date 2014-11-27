/*
 * Copyright (c) 2009-2012, Frederic DUBOUCHET
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

#ifndef MINIDHT_HEADER_DEFINED
#define MINIDHT_HEADER_DEFINED

#define DEFAULT_MAX_RECORDS (1024 * 1024)

// STL
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <list>
#include <bitset>
// BOOST
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>
// local
#include "miniDHT_proto.pb.h"
#include "miniDHT_session.h"
#include "miniDHT_db.h"
#include "miniDHT_const.h"
#include "miniDHT_bucket.h"
#include "miniDHT_search.h"

namespace miniDHT {

	class miniDHT {

	public :

		typedef uint32_t token_t;
		typedef std::string key_t;
		typedef search search_t;
		typedef bucket bucket_t;
		typedef
			std::map<endpoint_proto, session<PACKET_SIZE>*>
			map_ep_proto_session_t;
		typedef
			std::map<endpoint_proto, session<PACKET_SIZE>*>::iterator
			map_ep_proto_session_iterator;
		typedef
			bucket::iterator
			bucket_iterator;
		typedef std::map<key_t, time_t>::iterator map_key_time_iterator;

	private :

		const boost::posix_time::time_duration periodic_;
		key_t id_;
		size_t max_records_;
		boost::asio::io_service& io_service_;
		boost::asio::io_service periodic_io_;
		boost::asio::ip::tcp::acceptor acceptor_;
		boost::asio::deadline_timer dt_;
		boost::asio::ip::tcp::socket socket_;
		boost::asio::ip::tcp::endpoint sender_endpoint_;
		boost::thread* periodic_thread_;
		unsigned short listen_port_;
		// mutex lock
		boost::mutex giant_lock_;
		// bucket (contact list)
		bucket_t contact_list;
		// search list
		std::map<token_t, search_t> map_search;
		// key related storage
		db_multi_key_data db_storage;
		db_key_value db_backup;
		// token related storage
		std::map<token_t, boost::posix_time::ptime> map_ping_ttl;
		std::map<token_t, unsigned int> map_store_check_val;
		// session pointers
		map_ep_proto_session_t map_ep_proto_session;

	public :

		miniDHT(
			boost::asio::io_service& io_service,
			const boost::asio::ip::tcp::endpoint& ep,
			const std::string& path = std::string("./"),
			size_t max_records = DEFAULT_MAX_RECORDS);
		virtual ~miniDHT();

	public :

		// callback declaration
		typedef boost::function<void (const std::list<key_t>& k)>
			node_callback_t;
		typedef boost::function<void (const std::list<data_item_proto>& b)>
			value_callback_t;
		// iterative messages
		void iterativeStore(const std::string& k, const data_item_proto& b);
		void iterativeFindNode(const std::string& k);
		void iterativeFindNode(
			const key_t& k,
			const node_callback_t& c);
		void iterativeFindValue(
			const key_t& k,
			const value_callback_t& c,
			const std::string& hint = std::string(""));

	protected :

		void iterativeStore_nolock(
			const key_t& k,
			const data_item_proto& b);
		void iterativeFindNode_nolock(const key_t& k);

	public :

		std::list<contact_proto> nodes_description();
		size_t storage_size();
		size_t bucket_size();
		const key_t& get_local_key() const;
		const boost::asio::ip::tcp::endpoint get_local_endpoint();
		size_t storage_wait_queue() const;
		void set_max_record(size_t val);
		size_t get_max_record() const;

	protected :

		void restore_from_backup(
			const std::string& path,
			unsigned short port);
		void insert_db(const key_t& k, const data_item_proto& d);
		// called periodicly
		void periodic();
		void startNodeLookup(const token_t& t, const key_t& k);
		std::map<key_t, key_t> build_proximity(
			const key_t& k,
			const std::list<contact_proto>& lc);

	protected :

		void replyIterative(
			const std::list<contact_proto>& lc,
			const token_t& t,
			const key_t& k);
		void replyIterativeNodeSearch(
			const std::list<contact_proto>& lc,
			const token_t& t,
			const key_t& k);
		void replyIterativeValueSearch(
			const std::list<contact_proto>& lc,
			const token_t& t,
			const key_t& k);
		void replyIterativeStoreSearch(
			const std::list<contact_proto>& lc,
			const token_t& t,
			const key_t& k);

	protected :

		void start_accept();
		void handle_accept(
			session<PACKET_SIZE>* ps,
			const boost::system::error_code& error);
		void handle_receive(
			const boost::asio::ip::tcp::endpoint& ep,
			const basic_message<PACKET_SIZE>& msg);
		void handle_disconnect(const boost::asio::ip::tcp::endpoint& ep);
      void handle_message(const message_proto& m);
      void handle_message(
            const message_proto& m,
            const endpoint_proto& epp);
		void handle_SEND_PING(const message_proto& m);
		void handle_REPLY_PING(const message_proto& m);
		void handle_SEND_STORE(const message_proto& m);
		void handle_REPLY_STORE(const message_proto& m);
		void handle_SEND_FIND_NODE(const message_proto& m);
		void handle_REPLY_FIND_NODE(const message_proto& m);
		void handle_SEND_FIND_VALUE(const message_proto& m);
		void handle_REPLY_FIND_VALUE(const message_proto& m);

	protected :

		// generic send message
		void send_MESSAGE(const message_proto& m, const endpoint_proto& epp);
		void send_MESSAGE(const message_proto& m);

	public :

		// ping is now public so that you can bootstrap to a network after
		// class initialization.
		void send_PING(
			const endpoint_proto& epp,
			const token_t& t = random_bitset<TOKEN_SIZE>().to_ulong());
		void send_PING(
			const key_t& to_id,
			const token_t& t = random_bitset<TOKEN_SIZE>().to_ulong());

	protected :

		// ping with no lock to call from an already locked function
		void send_PING_nolock(
			const endpoint_proto& epp,
			const token_t& t = random_bitset<TOKEN_SIZE>().to_ulong());
		void send_PING_nolock(
			const key_t& to_id,
			const token_t& t = random_bitset<TOKEN_SIZE>().to_ulong());

	protected :

		void send_STORE(
			const key_t& to_id,
			const key_t& query_id,
			const data_item_proto& cbf,
			const token_t& t = random_bitset<TOKEN_SIZE>().to_ulong());
		void send_FIND_NODE(
			const key_t& to_id,
			const key_t& query_id,
			const token_t& t = random_bitset<TOKEN_SIZE>().to_ulong());
		void send_FIND_VALUE(
			const key_t& to_id,
			const key_t& query_id,
			const token_t& t = random_bitset<TOKEN_SIZE>().to_ulong(),
			const std::string& hint = std::string(""));

	protected :

		void reply_PING(
			const key_t& to_id,
			const token_t& t);
		void reply_STORE(
			const key_t& to_id,
			const token_t& t,
			const data_item_proto& cbf);
		void reply_FIND_NODE(
			const key_t& to_id,
			const token_t& t);
		void reply_FIND_VALUE(
			const key_t& to_id,
			const token_t& t,
			const std::string& hint = std::string(""));

	};

} // end of namespace miniDHT

#endif // MINIDHT_HEADER_DEFINED
