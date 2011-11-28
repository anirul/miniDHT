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

	template <
		// size in bit of the key
		unsigned int KEY_SIZE,
		// size in bit of the token
		unsigned int TOKEN_SIZE,
		// parallelism level, this is also the numbere of nodes per packet
		// sent back in a FIND_NODE message.
		unsigned int ALPHA = 3,
		// node (contact) per bucket
		unsigned int BUCKET_SIZE = 5,
		// call back for clean up (minutes) this is also used as a timeout
		// for the contact list (node list).
		size_t PERIODIC = 15,
		// maximum size of a packet
		size_t PACKET_SIZE = 1024 * 1024>
		
	class miniDHT {
	
	public :

		typedef std::bitset<TOKEN_SIZE> token_t;
		typedef std::bitset<KEY_SIZE> key_t;
		typedef less_bitset<KEY_SIZE> less_key;
		typedef less_bitset<TOKEN_SIZE> less_token;
		typedef std::map<key_t, key_t, less_key> map_key_key_t;
		typedef search<KEY_SIZE, BUCKET_SIZE> search_t;
		typedef bucket<BUCKET_SIZE, KEY_SIZE> bucket_t;
		typedef 
			std::map<boost::asio::ip::tcp::endpoint, session<PACKET_SIZE>*> 
			map_endpoint_session_t;
		typedef typename
			std::map<
				boost::asio::ip::tcp::endpoint, 
				session<PACKET_SIZE>*>::iterator
			map_endpoint_session_iterator;
		typedef typename 
			bucket<BUCKET_SIZE, KEY_SIZE>::iterator 
			bucket_iterator;
		typedef typename 
			std::map<key_t, time_t, less_key>::iterator
			map_key_time_iterator;
		typedef typename 
			std::map<key_t, key_t, less_key>::const_iterator
			const_map_key_key_iterator;
			
	private :
	
		const boost::posix_time::time_duration periodic_;
		const key_t id_;
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
		std::map<std::string, search_t> map_search;
		// key related storage
		db_multi_key_data db_storage;
		db_key_value db_backup;
		// token related storage
		std::map<std::string, boost::posix_time::ptime> map_ping_ttl;
		std::map<std::string, unsigned int> map_store_check_val;
		// session pointers
		map_endpoint_session_t map_endpoint_session;
			
	public :
	
		miniDHT(
			boost::asio::io_service& io_service, 
			const boost::asio::ip::tcp::endpoint& ep,
			const std::string& path = std::string("./"),
			size_t max_records = DEFAULT_MAX_RECORDS)
			:	periodic_(boost::posix_time::minutes(PERIODIC)),
				id_(local_key<KEY_SIZE>(ep.port())),
				periodic_io_(),
				max_records_(max_records),
				io_service_(io_service),
				acceptor_(io_service, ep),
				dt_(
					periodic_io_, 
					boost::posix_time::seconds(random() % 120)),
				socket_(io_service),
				listen_port_(ep.port()),
				contact_list(id_)
		{
			boost::mutex::scoped_lock lock_it(giant_lock_);
			std::stringstream ss("");
			ss << path << "localhost.store." << listen_port_ << ".db";
			db_storage.open(ss.str().c_str());
			restore_from_backup(path, listen_port_);
			dt_.async_wait(boost::bind(&miniDHT::periodic, this));
			periodic_thread_ = new boost::thread(
				boost::bind(&boost::asio::io_service::run,
				&periodic_io_));
			start_accept();
		}

		virtual ~miniDHT() {
			periodic_io_.stop();
//			periodic_thread_->join();
			delete periodic_thread_;
		}

	public :
				
		// callback declaration
		typedef boost::function<void (std::list<key_t> k)> 
			node_callback_t;
		typedef boost::function<void (const std::list<data_item_proto>& b)>  
			value_callback_t;

		// iterative messages
		void iterativeStore(
			const key_t& k, 
			const data_item_proto& b) 
		{
			boost::mutex::scoped_lock lock_it(giant_lock_);
			iterativeStore_nolock(k, b);
		}
		
	protected :

		void iterativeStore_nolock(
			const key_t& k, 
			const data_item_proto& b) 
		{
			token_t token = random_bitset<TOKEN_SIZE>();
			{
				map_search[key_to_string(token)] = search_t(id_, k, STORE_SEARCH);
				// save it temporary in the local DB
				map_search[key_to_string(token)].buffer = b;
			}
			startNodeLookup(token, k);
		}

	public :

		void iterativeFindNode(
			const key_t& k)
		{
			boost::mutex::scoped_lock lock_it(giant_lock_);
			iterativeFindNode_nolock(k);
		}

	protected :
	
		void iterativeFindNode_nolock(
			const key_t& k)
		{
			token_t token = random_bitset<TOKEN_SIZE>();
			map_search[key_to_string(token)] = search_t(id_, k, NODE_SEARCH);
			map_search[key_to_string(token)].node_callback_valid = false;
			startNodeLookup(token, k);
		}
		
	public :

		void iterativeFindNode(
			const key_t& k, 
			const node_callback_t& c) 
		{
			boost::mutex::scoped_lock lock_it(giant_lock_);
			token_t token = random_bitset<TOKEN_SIZE>();
			{
				map_search[key_to_string(token)] = search_t(id_, k, NODE_SEARCH);
				map_search[key_to_string(token)].node_callback_valid = true;
				map_search[key_to_string(token)].node_callback = c;
			}
			startNodeLookup(token, k);
		}
		
		void iterativeFindValue(
			const key_t& k, 
			const value_callback_t& c,
			const std::string& hint = std::string("")) 
		{
			boost::mutex::scoped_lock lock_it(giant_lock_);
			token_t token = random_bitset<TOKEN_SIZE>();
			{
				map_search[key_to_string(token)] = search_t(id_, k, VALUE_SEARCH);
				map_search[key_to_string(token)].value_callback = c;
				map_search[key_to_string(token)].hint = hint;
			}
			startNodeLookup(token, k);
		}
	
	public :
	
		std::list<contact_proto> nodes_description() {
			boost::mutex::scoped_lock lock_it(giant_lock_);
			std::list<contact_proto> ls;
			bucket_iterator ite = contact_list.begin();
			for (; ite != contact_list.end(); ++ite)
				ls.push_back(ite->second);
			return ls;
		}

		size_t storage_size() { 
			boost::mutex::scoped_lock lock_it(giant_lock_);
			return db_storage.size(); 
		}

		size_t bucket_size() { 
			boost::mutex::scoped_lock lock_it(giant_lock_);
			return db_backup.size(); 
		}

		const key_t& get_local_key() const { 
			boost::mutex::scoped_lock lock_it(giant_lock_);
			return id_; 
		}

		const boost::asio::ip::tcp::endpoint get_local_endpoint() { 
			boost::mutex::scoped_lock lock_it(giant_lock_);
			return socket_.local_endpoint(); 
		}

		size_t storage_wait_queue() const { 
			boost::mutex::scoped_lock lock_it(giant_lock_);
			return map_store_check_val.size(); 
		}

		void set_max_record(size_t val) { 
			boost::mutex::scoped_lock lock_it(giant_lock_);
			max_records_ = val; 
		}

		size_t get_max_record() const { 
			boost::mutex::scoped_lock lock_it(giant_lock_);
			return max_records_; 
		}
			
	protected :
	
		void restore_from_backup(
			const std::string& path,
			unsigned short port) 
		{
			std::stringstream sb("");
			sb << path << "localhost.buckets." << port << ".db";
			db_backup.open(sb.str().c_str());
			std::list<std::string> l;
			db_backup.list_value(l);
			std::list<std::string>::iterator ite;
			std::pair<std::string, std::string> endpoint_pair;
			for (ite = l.begin(); ite != l.end(); ++ite) {
				endpoint_pair = string_to_endpoint_pair((*ite));
				try {
					send_PING_nolock(endpoint_pair.first, endpoint_pair.second);
				} catch (std::exception& e) {
					std::cerr 
						<< "Contacting Host -> " << endpoint_pair.first 
						<< ":" << endpoint_pair.second 
						<< " -> exception : " << e.what() << std::endl;
				}
			}
		}
		
		void insert_db(const std::string& k, const data_item_proto& d) {
			// check if the element already exist
			if (db_storage.count(k) == 0) {
				// check if size limit is reached
				while (db_storage.size() >= max_records_) {
					// drop the oldest record
					db_storage.remove_oldest();
				}
				db_storage.insert(
					k, 
					d.title(),
					d.time(),
					d.ttl(),
					d.data());
				return;
			}
			// key already in
			std::list<data_item_proto> ld;
			db_storage.find_no_blob(k, ld);
			std::list<data_item_proto>::iterator ite;
			for (ite = ld.begin(); ite != ld.end(); ++ite) {
				if (ite->title() == d.title()) {
					boost::posix_time::ptime now = update_time();
					boost::posix_time::time_duration temp_time =
						now - boost::posix_time::from_time_t(ite->time());
					boost::posix_time::time_duration d_time = now - 
						boost::posix_time::from_time_t(d.time());
					if (d_time < temp_time)
						db_storage.update(k, d.title(), d.time(), d.ttl());
					return;
				}
			}
			// a duplicate with different title (should not happen)
			db_storage.insert(
				k, 
				d.title(),
				d.time(),
				d.ttl(),
				d.data());
		}
	
		// called periodicly
		void periodic() {
			{ // check timeout on key
				boost::mutex::scoped_lock lock_it(giant_lock_);
				std::cout << "!!! PERIODIC !!! part 1" << std::endl;
				bucket_iterator itc;
				const boost::posix_time::time_duration tRefresh = 
					boost::posix_time::minutes(PERIODIC);
				for (	itc = contact_list.begin(); 
						itc != contact_list.end(); 
						++itc) 
				{
					if (itc->first == KEY_SIZE) continue;
					// contact key
					boost::posix_time::time_duration td = 
						update_time() - boost::posix_time::from_time_t(
							itc->second.time());
					if (td > tRefresh) {
						send_PING_nolock(
							itc->second.ep().address(),
							itc->second.ep().port());
						contact_list.erase(itc);
						itc = contact_list.begin();
						db_backup.remove(itc->second.key());
					} else {
						std::string key = itc->second.key();
						std::string endpoint = endpoint_to_string(itc->second.ep());
						std::string value = db_backup.find(key);
						// already in
						if (value == endpoint) continue;
						// remove in case present
						if (value != std::string(""))
							db_backup.remove(key);
						db_backup.insert(key, endpoint);
					}
				}
			}
			periodic_thread_->yield();
			{ // try to diversify the bucket list
				boost::mutex::scoped_lock lock_it(giant_lock_);
				std::cout << "!!! PERIODIC !!! part 2" << std::endl;
				for (unsigned int i = 0; i < (KEY_SIZE - 1); ++i) {
					if (contact_list.count(i)) continue;
					key_t skey = contact_list.random_key_in_bucket(i);
					iterativeFindNode_nolock(skey); 
				}
			}
			periodic_thread_->yield();
			{ 
				// search if storage DB need any cleaning
				std::list<data_item_header_t> ldh;
				std::list<data_item_header_t>::iterator ite;
				{
					boost::mutex::scoped_lock lock_it(giant_lock_);
					std::cout 
						<< "!!! PERIODIC !!! part 3.1 - fetch header" 
						<< std::endl;
					db_storage.list_headers(ldh);
				}
				boost::posix_time::ptime check_time = update_time();
				for (ite = ldh.begin(); ite != ldh.end(); ++ite) {
					boost::posix_time::time_duration time_elapsed = 
						check_time - boost::posix_time::from_time_t(ite->time);
					if (time_elapsed > boost::posix_time::seconds(ite->ttl)) {
						// data is no more valid
						boost::mutex::scoped_lock lock_it(giant_lock_);
						std::cout
							<< "!!! PERIODIC !!! part 3.2 - remove : "
							<< ite->key << std::endl;
						db_storage.remove(ite->key, ite->title);
					} else {
						// republish
						boost::mutex::scoped_lock lock_it(giant_lock_);
						std::cout
							<< "!!! PERIODIC !!! part 3.3 - republish : "
							<< ite->key << std::endl;
						data_item_proto item;
						db_storage.find(ite->key, ite->title, item);
						item.set_ttl(item.ttl() - time_elapsed.total_seconds());
						iterativeStore_nolock(
							string_to_key<KEY_SIZE>(ite->key), item);
					}
					periodic_thread_->yield();
				}
			}
			{
				boost::mutex::scoped_lock lock_it(giant_lock_);
				std::cout << "!!! PERIODIC !!! part 4" << std::endl;
				// call back later
				boost::posix_time::time_duration wait_time = 
					periodic_ + boost::posix_time::seconds(random() % 60);
				boost::posix_time::ptime now = update_time();
				dt_.expires_at(now + wait_time);
				dt_.async_wait(boost::bind(&miniDHT::periodic, this));
			}
		}

		void startNodeLookup(const token_t& t, const key_t& k) {
			std::list<contact_proto> temp_list;
			// get the proximity list from the contact_list
			const map_key_key_t& map_proximity = 
				contact_list.build_proximity(k);
			const_map_key_key_iterator itp = map_proximity.begin();
			for (; itp != map_proximity.end(); ++itp)	{
				if (contact_list.find_key(itp->second) == contact_list.end())
					continue;
				contact_proto c;
				c.set_key(key_to_string(itp->second));
				boost::asio::ip::tcp::endpoint ep = contact_list[itp->second];
				endpoint_proto epp;
				epp.set_address(ep.address().to_string());
				{
					std::stringstream ss("");
					ss << ep.port();
					epp.set_port(ss.str());
				}
				temp_list.push_back(c);
			}
			replyIterative(temp_list, key_to_string(t), key_to_string(k));
		}
		
		map_key_key_t build_proximity(
			const key_t& k, 
			const std::list<contact_proto>& lc)
		{
			map_key_key_t map_proximity;
			std::list<contact_proto>::const_iterator itc;
			for (itc = lc.begin(); itc != lc.end(); ++itc)
				map_proximity[k ^ string_to_key<KEY_SIZE>(itc->key())] = 
					string_to_key<KEY_SIZE>(itc->key());
			return map_proximity;
		}
		
	protected :

		void replyIterative(
			const std::list<contact_proto>& lc, 
			const std::string& t, 
			const std::string& k)
		{
			search_t local_search;
			{
				local_search = map_search[t];
			}
			switch (local_search.search_type) {
				case NODE_SEARCH :
					replyIterativeNodeSearch(lc, t, k);
					break;
				case VALUE_SEARCH :
					replyIterativeValueSearch(lc, t, k);
					break;
				case STORE_SEARCH :
					replyIterativeStoreSearch(lc, t, k);
					break;
				default:
					std::cerr << "\t\t(" << local_search.search_type 
						<< ") ???" << std::endl;
					break;
			}
		}
		
		void replyIterativeNodeSearch(
			const std::list<contact_proto>& lc, 
			const std::string& t, 
			const std::string& k)
		{
			if (map_search[t].update_list(lc)) {
				// iterativeFindNode
				if (map_search[t].node_callback_valid) {
					try {
						map_search[t].call_node_callback();	
					}
					catch (std::exception& ex) {
						std::cerr 
							<< "??? counld not call callback ex : " << ex.what()
							<< std::endl;
					}
				}
				map_search.erase(t);
			} else {
				for (unsigned int i = 0; 
					i < ALPHA && !map_search[t].is_node_full(); 
					++i)
				{
					send_FIND_NODE(
						map_search[t].get_node_endpoint(), 
						k, 
						string_to_key<TOKEN_SIZE>(t));
				}
			}
		}
		
		void replyIterativeValueSearch(
			const std::list<contact_proto>& lc, 
			const std::string& t, 
			const std::string& k)
		{
			map_search[t].update_list(lc);
			// iterativeFindValue
			for (	unsigned int i = 0; 
					i < ALPHA && !map_search[t].is_node_full(); 
					++i)
				send_FIND_VALUE(
					map_search[t].get_node_endpoint(), 
					k, 
					string_to_key<TOKEN_SIZE>(t));
		}
		
		void replyIterativeStoreSearch(
			const std::list<contact_proto>& lc, 
			const std::string& t, 
			const std::string& k)
		{
			assert(k != std::string("0000000000000000000000000000000"\
					"000000000000000000000000000000000"));
			if (map_search[t].update_list(lc)) {
				// iterativeStore
				while (!map_search[t].is_value_full())
					send_STORE(
						map_search[t].get_value_endpoint(), 
						k, 
						map_search[t].buffer, 
						string_to_key<TOKEN_SIZE>(t));
				// free some local space
				map_search.erase(t);
			} else {
				for (	unsigned int i = 0; 
						i < ALPHA && !map_search[t].is_node_full(); 
						++i)
					send_FIND_NODE(
						map_search[t].get_node_endpoint(), 
						k, 
						string_to_key<TOKEN_SIZE>(t));
			}
		}

	protected :

		void start_accept() {
			session<PACKET_SIZE>* new_session = new session<PACKET_SIZE>(
				io_service_,
				boost::bind(
					&miniDHT::handle_receive,
					this,
					_1, 
					_2),
				map_endpoint_session);
			acceptor_.async_accept(
				new_session->socket(),
				boost::bind(
					&miniDHT::handle_accept,
					this,
					new_session,
					boost::asio::placeholders::error));
		}

		void handle_accept(
			session<PACKET_SIZE>* ps,
			const boost::system::error_code& error) 
		{
			boost::mutex::scoped_lock lock_it(giant_lock_);
			if (!error) {
				ps->start();
			}
			start_accept();
		}
		
		void handle_receive(
			const boost::asio::ip::tcp::endpoint& ep,
			const basic_message<PACKET_SIZE>& msg)	
		{
			boost::mutex::scoped_lock lock_it(giant_lock_);
			sender_endpoint_ = ep;
			message_proto m;
			std::stringstream ss(std::string(msg.body(), msg.body_length()));
			try {
				m.ParseFromIstream(&ss);
				switch (m.type()) {
					case message_proto::SEND_PING :
						handle_SEND_PING(m);
						contact_list.add_contact(
							string_to_key<KEY_SIZE>(m.from_id()), 
							ep);
						break;
					case message_proto::REPLY_PING :
						handle_REPLY_PING(m);
						break;
					case message_proto::SEND_STORE :
						handle_SEND_STORE(m);
						contact_list.add_contact(
							string_to_key<KEY_SIZE>(m.from_id()), 
							ep);
						break;
					case message_proto::REPLY_STORE :
						handle_REPLY_STORE(m);
						break;
					case message_proto::SEND_FIND_NODE :
						handle_SEND_FIND_NODE(m);
						contact_list.add_contact(
							string_to_key<KEY_SIZE>(m.from_id()), 
							ep);
						break;
					case message_proto::REPLY_FIND_NODE :
						handle_REPLY_FIND_NODE(m);
						break;
					case message_proto::SEND_FIND_VALUE :
						handle_SEND_FIND_VALUE(m);
						contact_list.add_contact(
							string_to_key<KEY_SIZE>(m.from_id()), 
							ep);
						break;
					case message_proto::REPLY_FIND_VALUE :
						handle_REPLY_FIND_VALUE(m);
						break;
					case message_proto::NONE :
					default :
						break;
				}
			} catch (std::exception& e) {
				std::cerr 
					<< "[" << socket_.local_endpoint()
					<< "] error in deserializing message (dump)." 
					<< std::endl
					<< "\texception : " << e.what() << std::endl;
				std::cerr
					<< ss.str() << std::endl;
			}
		}
		
		void handle_SEND_PING(const message_proto& m) {
			reply_PING(sender_endpoint_, m.token());
		}
					
		void handle_REPLY_PING(const message_proto& m) {
			const boost::posix_time::time_duration tTimeout = 
			boost::posix_time::minutes(1);
			{
				if (map_ping_ttl.find(m.token()) != map_ping_ttl.end()) {
					boost::posix_time::time_duration td = 
						map_ping_ttl[m.token()] - update_time();
					if (td < tTimeout) {
						contact_list.add_contact(
							string_to_key<KEY_SIZE>(m.from_id()), 
							sender_endpoint_);
					} else {
						std::cout 
							<< "\tTimeout no recording." 
							<< std::endl;
					}
					map_ping_ttl.erase(map_ping_ttl.find(m.token()));
				} else {
					std::cerr << "\ttoken [" << m.token()
						<< "] unknown." << std::endl;
				}
			}
		}
		
		void handle_SEND_STORE(const message_proto& m) {
			data_item_proto di(m.data_item());
			di.set_time(to_time_t(update_time()));
			insert_db(m.to_id(), di);
			reply_STORE(sender_endpoint_, m.token(), di);
		}
		
		void handle_REPLY_STORE(const message_proto& m) {
			if (map_store_check_val.find(m.token()) != map_store_check_val.end()) {
				if (m.check_val() != map_store_check_val[m.token()]) {
					std::cerr 
						<< "[" << socket_.local_endpoint()
						<< "] check val missmatch storage problem?" 
						<< std::endl;
					std::cerr 
						<< m.check_val() << " != " 
						<< map_store_check_val[m.token()] << std::endl;
				} 
				map_store_check_val.erase(map_store_check_val.find(m.token()));
			} 
		}
		
		void handle_SEND_FIND_NODE(const message_proto& m) {
			reply_FIND_NODE(sender_endpoint_, m.token(),	m.to_id());
		}
		
		void handle_REPLY_FIND_NODE(const message_proto& m) {
			{ // check if the search exist
				if (map_search.find(m.token()) == map_search.end())
					return;
			}
			std::list<contact_proto> lc;
			for (int i = 0; i < m.contact_list_size(); ++i) {
				contact_proto c = m.contact_list(i);
				std::string address = c.ep().address();
				if ((address == std::string("0.0.0.0")) ||
					(address == std::string("127.0.0.1")) ||
					(address == std::string("localhost"))) {
					address = sender_endpoint_.address().to_string();
					boost::asio::ip::tcp::endpoint uep(
						boost::asio::ip::address::from_string(address),
						atoi(c.ep().port().c_str()));
					contact_list.add_contact(
						string_to_key<KEY_SIZE>(c.key()), 
						uep, 
						false);
				} else {
					contact_list.add_contact(
						string_to_key<KEY_SIZE>(c.key()), 
						boost::asio::ip::tcp::endpoint(
							boost::asio::ip::address::from_string(c.ep().address()),
							::atoi(c.ep().port().c_str())), 
						false);
				}
				lc.push_back(c);				
			}
			replyIterative(lc, m.token(), m.to_id());
		}
		
		void handle_SEND_FIND_VALUE(const message_proto& m) {
			bool is_present = false;
			is_present = (db_storage.count(m.to_id()) != 0);
			if (is_present) {
				reply_FIND_VALUE(sender_endpoint_, m.token(), m.to_id(), m.hint());
			} else {
				reply_FIND_NODE(sender_endpoint_, m.token(), m.to_id());
			}
		}
		
		void handle_REPLY_FIND_VALUE(const message_proto& m) {
			if (map_search.find(m.token()) != map_search.end()) {
				std::list<data_item_proto> ld;
				for (int i = 0; i < m.data_item_list_size(); ++i)
					ld.push_back(m.data_item_list(i));
				map_search[m.token()].value_callback(ld);
				map_search.erase(m.token());
			}
		}

		void send_MESSAGE(
			const message_proto& m,
			const boost::asio::ip::tcp::endpoint& ep) 
		{
			std::string address = ep.address().to_string();
			unsigned short port = ep.port();
			std::stringstream ss("");
			try {
				m.SerializeToOstream(&ss);
				if (address == std::string("0.0.0.0"))
					address = std::string("127.0.0.1");
				boost::asio::ip::tcp::endpoint uep(
					boost::asio::ip::address::from_string(address), 
					port);
				map_endpoint_session_iterator ite = map_endpoint_session.find(uep);
				basic_message<PACKET_SIZE> msg(ss.str().size());
				std::memcpy(msg.body(), &(ss.str())[0], ss.str().size());
				msg.listen_port(listen_port_);
				msg.encode_header();
				if (ite == map_endpoint_session.end())	{
					session<PACKET_SIZE>* new_session = new session<PACKET_SIZE>(
						io_service_,
						boost::bind(
							&miniDHT::handle_receive,
							this,
							_1, 
							_2),
						map_endpoint_session);
					new_session->connect(uep);
					new_session->deliver(msg);
				} else {
					ite->second->deliver(msg);
				}
			} catch (std::exception& e) {
				std::cerr 
					<< "Exception in send_MESSAGE(" << ep << " - " 
					<< ss.str().size()
					<< ") : " << e.what() 
					<< std::endl;				
			}
		}
		
	public :
	
		// primitive messages
		
		// ping is now public so that you can bootstrap to a network after
		// class initialization.
		void send_PING(
			const boost::asio::ip::tcp::endpoint& ep,
			token_t token = random_bitset<TOKEN_SIZE>())
		{
			boost::mutex::scoped_lock lock_it(giant_lock_);
			send_PING_nolock(ep, token);
		}
		
		// helper function now that there is no more ping & constructor
		void send_PING(
			const std::string& address,
			const unsigned short port)
		{
			std::stringstream ss("");
			ss << port;
			send_PING(address, ss.str());
		}

		void send_PING(
			const std::string& address,
			const std::string& port)
		{
			boost::asio::ip::tcp::resolver resolver(io_service_);
			boost::asio::ip::tcp::resolver::query query(
				boost::asio::ip::tcp::v4(), 
				address, 
				port);
			boost::asio::ip::tcp::resolver::iterator iterator = 
				resolver.resolve(query);		
			send_PING(*iterator);			
		}

	protected :

		// ping with no lock to call from an already locked function
		void send_PING_nolock(
			const std::string& address,
			const std::string& port)
		{
			boost::asio::ip::tcp::resolver resolver(io_service_);
			boost::asio::ip::tcp::resolver::query query(
				boost::asio::ip::tcp::v4(), 
				address, 
				port);
			boost::asio::ip::tcp::resolver::iterator iterator = 
				resolver.resolve(query);		
			send_PING_nolock(*iterator);
		}

		void send_PING_nolock(
			const boost::asio::ip::tcp::endpoint& ep,
			token_t token = random_bitset<TOKEN_SIZE>())
		{
			try {
				message_proto m;
				m.set_type(message_proto::SEND_PING);
				m.set_from_id(key_to_string(id_));
				m.set_token(key_to_string(token));	
				map_ping_ttl[m.token()] = update_time();
				send_MESSAGE(m, ep);
			} catch (std::exception& e) {
				std::cerr 
					<< "Exception in send_PING(" << ep
					<< ") : " << e.what() 
					<< std::endl;				
			}
		}
		
	protected :

		void send_STORE(
			const boost::asio::ip::tcp::endpoint& ep,
			const std::string& to_id,
			const data_item_proto& cbf,
			token_t token = random_bitset<TOKEN_SIZE>())
		{
			try {
				message_proto m;
				m.set_type(message_proto::SEND_STORE);
				m.set_from_id(key_to_string(id_));
				m.set_token(key_to_string(token));
				m.set_to_id(to_id);
				(*m.mutable_data_item()) = cbf;
				map_store_check_val[key_to_string(token)] = cbf.data().size();
				send_MESSAGE(m, ep);
			} catch (std::exception& e) {
				std::cerr 
					<< "Exception in send_STORE(" << ep 
					<< ") : " << e.what() 
					<< std::endl;				
			}
		}

		void send_FIND_NODE(
			const boost::asio::ip::tcp::endpoint& ep,
			const std::string& to_id,
			token_t token = random_bitset<TOKEN_SIZE>())
		{
			try {
				message_proto m;
				m.set_type(message_proto::SEND_FIND_NODE);
				m.set_from_id(key_to_string(id_));
				m.set_token(key_to_string(token));
				m.set_to_id(to_id);
				send_MESSAGE(m, ep);
			} catch (std::exception& e) {
				std::cerr 
					<< "Exception in send_FIND_NODE(" << ep
					<< ") : " << e.what() 
					<< std::endl;				
			}
		}

		void send_FIND_VALUE(
			const boost::asio::ip::tcp::endpoint& ep,
			const std::string& to_id,
			token_t token = random_bitset<TOKEN_SIZE>(),
			const std::string& hint = std::string(""))
		{
			try {
				message_proto m;
				m.set_type(message_proto::SEND_FIND_VALUE);
				m.set_from_id(key_to_string(id_));
				m.set_token(key_to_string(token));
				m.set_to_id(to_id);
				m.set_hint(hint);
				send_MESSAGE(m, ep);
			} catch (std::exception& e) {
				std::cerr 
					<< "Exception in send_FIND_VALUE(" << ep
					<< ") : " << e.what() 
					<< std::endl;				
			}
		}
			
	protected :
	
		void reply_PING(
			const boost::asio::ip::tcp::endpoint& ep, 
			const std::string& tok)
		{
			try {
				message_proto m;
				m.set_type(message_proto::REPLY_PING);
				m.set_from_id(key_to_string(id_));
				m.set_token(tok);
				send_MESSAGE(m, ep);
			} catch (std::exception& e) {
				std::cerr 
					<< "Exception in reply_PING(" << ep
					<< ") : " << e.what() 
					<< std::endl;				
			}
		}

		void reply_STORE(
			const boost::asio::ip::tcp::endpoint& ep,
			const std::string& tok,
			const data_item_proto& cbf)
		{
			try {
				digest_t digest;
				message_proto m;
				m.set_type(message_proto::REPLY_STORE);
				m.set_from_id(key_to_string(id_));
				m.set_token(tok);
				m.set_check_val(cbf.data().size());
				send_MESSAGE(m, ep);
			} catch (std::exception& e) {
				std::cerr 
					<< "Exception in reply_STORE(" << ep
					<< ") : " << e.what() 
					<< std::endl;				
			}
		}

		void reply_FIND_NODE(
			const boost::asio::ip::tcp::endpoint& ep,
			const std::string& tok,
			const std::string& to_id)
		{
			try {
				message_proto m;
				m.set_type(message_proto::REPLY_FIND_NODE);
				m.set_from_id(key_to_string(id_));
				m.set_token(tok);
				m.set_to_id(to_id);
				for (bucket_iterator bit = contact_list.begin(); 
					bit != contact_list.end();) 
				{
					(*m.add_contact_list()) = bit->second;
					++bit;
					if ((m.contact_list_size() >= ALPHA) || 
						(bit == contact_list.end()))
					{
						send_MESSAGE(m, ep);
						m.clear_contact_list();
					}
				}
			} catch (std::exception& e) {
				std::cerr 
					<< "\t\t\tException in reply_FIND_NODE(" << ep
					<< ") : " << e.what() 
					<< std::endl;
			}
		}

		void reply_FIND_VALUE(
			const boost::asio::ip::tcp::endpoint& ep,
			const std::string& tok,
			const std::string& to_id,
			const std::string& hint = std::string(""))
		{
			try {
				message_proto m;
				m.set_type(message_proto::REPLY_FIND_VALUE);
				m.set_from_id(key_to_string(id_));
				m.set_token(tok);
				m.set_to_id(to_id);
				{
					std::list<data_item_proto> ld;
					std::list<data_item_proto>::iterator ite;
					db_storage.find(to_id, ld);
					for (ite = ld.begin(); ite != ld.end(); ++ite) {
						const data_item_proto& item = (*ite);
						if (item.title().find(hint) != std::string::npos)
							(*m.add_data_item_list()) = item;
					}
				}
				if (m.data_item_list_size() > 0) {
					send_MESSAGE(m, ep);
				}
			} catch (std::exception& e) {
				std::cerr 
					<< "Exception in reply_FIND_VALUE(" << ep 
					<< ") : " << e.what() 
					<< std::endl;				
			}
		}
	
	};

} // end of namespace miniDHT

#endif // MINIDHT_HEADER_DEFINED

