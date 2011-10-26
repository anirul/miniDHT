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

#define SERIALIZE_XML

#ifdef SERIALIZE_BINARY
	#ifdef SERIALIZE_XML
		#undef SERIALIZE_XML
	#endif // SERIALIZE_XML
#else // !SERIALIZE_BINARY
	#ifndef SERIALIZE_XML
		#define SERIALIZE_BINARY
	#endif // !SERIALIZE_XML
#endif // SERIALIZE_BINARY

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
#ifdef SERIALIZE_XML
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif // SERIALIZE_XML
#ifdef SERIALIZE_BINARY
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#endif // SERIALIZE_BINARY
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
// local
#include "miniDHT_session.h"
#include "miniDHT_db.h"
#include "miniDHT_message.h"
#include "miniDHT_const.h"
#include "miniDHT_contact.h"
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
		unsigned int ALPHA = 5,
		// node (contact) per bucket
		unsigned int BUCKET_SIZE = 5,
		// call back for clean up (minutes) this is also used as a timeout
		// for the contact list (node list).
		size_t PERIODIC = 5,
		// maximum size of a packet
		size_t PACKET_SIZE = 1024 * 1024>
		
	class miniDHT {
	
	public :

		typedef std::bitset<TOKEN_SIZE> token_t;
		typedef std::bitset<KEY_SIZE> key_t;
		typedef contact<KEY_SIZE> contact_t;
		typedef	message<KEY_SIZE, TOKEN_SIZE> message_t;
		typedef less_bitset<KEY_SIZE> less_key;
		typedef less_bitset<TOKEN_SIZE> less_token;
		typedef std::map<key_t, key_t, less_key> map_key_key_t;
		typedef search<KEY_SIZE, BUCKET_SIZE> search_t;
		typedef bucket<BUCKET_SIZE, KEY_SIZE> bucket_t;
		typedef 
			std::map<boost::asio::ip::tcp::endpoint, session<PACKET_SIZE>*> 
			map_endpoint_session_t;
		typedef 
			db_multi_wrapper<key_t, data_item_t, less_key> 
			db_key_data_t;
		typedef 
			db_wrapper<key_t, boost::asio::ip::tcp::endpoint, less_key> 
			db_key_endpoint_t;
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
		typedef typename 
			std::list<contact_t>::const_iterator 
			const_list_contact_t_iterator;
		typedef typename std::map<size_t, contact_t>::iterator
			map_size_contact_iterator;
		typedef typename 
			std::map<token_t, std::list<contact_t>, less_token>::iterator
			map_token_list_contact_iterator;
		typedef typename
			db_multi_wrapper<key_t, data_item_t, less_key>::iterator
			db_key_data_item_iterator;
		typedef typename
			db_wrapper<
				key_t, 
				boost::asio::ip::tcp::endpoint, 
				less_key>::iterator
			db_key_endpoint_iterator;
			
	private :
	
		const boost::posix_time::time_duration periodic_;
		const key_t id_;
		size_t max_records_;
		boost::asio::io_service& io_service_;
		boost::asio::ip::tcp::acceptor acceptor_;
		boost::asio::deadline_timer dt_;
		boost::asio::ip::tcp::socket socket_;
		boost::asio::ip::tcp::endpoint sender_endpoint_;
		unsigned short listen_port_;
		// mutex lock
		boost::mutex giant_lock_;
		// bucket (contact list)
		bucket_t contact_list;
		// search list
		std::map<token_t, search_t, less_token> map_search;
		// key related storage
		db_key_data_t db_storage;
		db_key_endpoint_t db_backup;
		// token related storage
		std::map<token_t, boost::posix_time::ptime, less_token> map_ping_ttl;
		std::map<token_t, digest_t, less_token> map_store_digest;
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
				max_records_(max_records),
				io_service_(io_service),
				acceptor_(io_service, ep),
				dt_(
					io_service, 
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
			start_accept();
		}

		virtual ~miniDHT() {
			boost::mutex::scoped_lock lock_it(giant_lock_);
			db_storage.flush();
			db_storage.close();
			db_backup.flush();
			db_backup.close();
		}

	public :
				
		// callback declaration
		typedef boost::function<void (std::list<key_t> k)> 
			node_callback_t;
		typedef boost::function<void (const std::list<data_item_t>& b)>  
			value_callback_t;

		// iterative messages
		void iterativeStore(
			const key_t& k, 
			const data_item_t& b) 
		{
			boost::mutex::scoped_lock lock_it(giant_lock_);
			iterativeStore_nolock(k, b);
		}
		
	protected :

		void iterativeStore_nolock(
			const key_t& k, 
			const data_item_t& b) 
		{
			token_t token = random_bitset<TOKEN_SIZE>();
			{
				map_search[token] = search_t(id_, k, STORE_SEARCH);
				// save it temporary in the local DB
				map_search[token].buffer = b;
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
			map_search[token] = search_t(id_, k, NODE_SEARCH);
			map_search[token].node_callback_valid = false;
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
				map_search[token] = search_t(id_, k, NODE_SEARCH);
				map_search[token].node_callback_valid = true;
				map_search[token].node_callback = c;
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
				map_search[token] = search_t(id_, k, VALUE_SEARCH);
				map_search[token].value_callback = c;
				map_search[token].hint = hint;
			}
			startNodeLookup(token, k);
		}
	
	public :
	
		std::list<contact_t> nodes_description() {
			boost::mutex::scoped_lock lock_it(giant_lock_);
			std::list<contact_t> ls;
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
			return map_store_digest.size(); 
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
			db_key_endpoint_iterator dbit;
			for (dbit = db_backup.begin(); dbit != db_backup.end(); ++dbit) {
				boost::asio::ip::tcp::endpoint ep = dbit->second;
				send_PING_nolock(ep, random_bitset<TOKEN_SIZE>());
			}
		}
		
		void insert_db(const key_t& k, const data_item_t& d) {
			// flush (force database update)
			db_storage.flush();
			// check is the element already exist
			db_key_data_item_iterator ite = db_storage.find(k);
			if (ite == db_storage.end()) {
				// check if size limit is reached
				while (db_storage.size() >= max_records_) {
					// drop the oldest record
					db_key_data_item_iterator ite = db_storage.begin();
					db_key_data_item_iterator oldest_ite = db_storage.begin();
					for (; ite != db_storage.end(); ++ite) {
						if ((ite->second.time + ite->second.ttl) < 
							(oldest_ite->second.time + oldest_ite->second.ttl)) 
							oldest_ite = ite;
					}
					// clean the oldest record & flush
					db_storage.erase(oldest_ite);
					db_storage.flush();
				}
				db_storage.insert(make_pair(k, d));
				return;
			}
			// already in
			while ((ite != db_storage.end()) && (ite->first == k)) {
				// check title
				if (ite->second.title == d.title) {
					boost::posix_time::ptime now = update_time();
					boost::posix_time::time_duration temp_time = 
						now - ite->second.time;
					boost::posix_time::time_duration d_time = now - d.time;
					// more recent -> update
					if (d_time < temp_time) {
						db_storage.erase(ite);
						db_storage.insert(make_pair(k, d));
					} 
					return;
				} else {
					++ite;
				}
			}
			db_storage.insert(make_pair(k, d));
		}
	
		// called periodicly
		void periodic() {
			{ // check timeout on key
				boost::mutex::scoped_lock lock_it(giant_lock_);
				bucket_iterator itc;
				db_backup.clear();
				const boost::posix_time::time_duration tRefresh = 
					boost::posix_time::minutes(PERIODIC);
				for (itc = contact_list.begin(); itc != contact_list.end(); ++itc) {
					if (itc->first == KEY_SIZE) continue;
					// contact key
					boost::posix_time::time_duration td = 
						update_time() - itc->second.ttl;
					if (td > tRefresh) {
						send_PING_nolock(itc->second.ep);
						contact_list.erase(itc);
						itc = contact_list.begin();
					} else {
						db_backup.insert(
							make_pair(itc->second.key, itc->second.ep));
					}
				}
			}
			{ // try to diversify the bucket list
				boost::mutex::scoped_lock lock_it(giant_lock_);
				for (unsigned int i = 0; i < (KEY_SIZE - 1); ++i) {
					if (contact_list.count(i)) continue;
					key_t skey = contact_list.random_key_in_bucket(i);
					iterativeFindNode_nolock(skey); 
				}
			}
			{ // flush databases
				boost::mutex::scoped_lock lock_it(giant_lock_);
				db_storage.flush();
				db_backup.flush();
				// search if storage DB need any cleaning
				db_key_data_item_iterator db_storage_ite = db_storage.begin();
				boost::posix_time::ptime now = update_time();
				bool changed = false;
				while (db_storage_ite != db_storage.end()) {
					data_item_t pt = db_storage_ite->second;
					boost::posix_time::time_duration time_elapsed = now - pt.time;
					// data is no more valid
					if (time_elapsed > pt.ttl) {
						db_key_data_item_iterator elem_to_delete = db_storage_ite;
						db_storage_ite++;
						db_storage.erase(elem_to_delete);
						changed = true;
					// republish
					} else {
						pt.ttl -= time_elapsed;
						iterativeStore_nolock(db_storage_ite->first, pt);
						db_storage_ite++;
					}
				}
				// save the changes (in case of changes)
				if (changed) db_storage.flush();
			}
			{
				boost::mutex::scoped_lock lock_it(giant_lock_);
				// call back later
				boost::posix_time::time_duration wait_time = 
					periodic_ + boost::posix_time::seconds(random() % 60);
				boost::posix_time::ptime now = update_time();
				dt_.expires_at(now + wait_time);
				dt_.async_wait(boost::bind(&miniDHT::periodic, this));
			}
		}

		void startNodeLookup(const token_t& t, const key_t& k) {
			std::list<contact_t> temp_list;
			{ // get the proximity list from the contact_list
				const map_key_key_t& map_proximity = 
					contact_list.build_proximity(k);
				const_map_key_key_iterator itp = map_proximity.begin();
				for (; itp != map_proximity.end(); ++itp)	{
					if (contact_list.find_key(itp->second) == contact_list.end())
						continue;
					contact_t c(itp->second, contact_list[itp->second]);
					temp_list.push_back(c);
				}
			}
			replyIterative(temp_list, t, k);
		}
		
		map_key_key_t build_proximity(
			const key_t& k, 
			const std::list<contact_t>& lc)
		{
			map_key_key_t map_proximity;
			const_list_contact_t_iterator itc;
			for (itc = lc.begin(); itc != lc.end(); ++itc)
				map_proximity[k ^ (key_t)(itc->first)] = itc->first;
			return map_proximity;
		}
		
	protected :

		void replyIterative(
			const std::list<contact_t>& lc, 
			const token_t& t, 
			const key_t& k)
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
			const std::list<contact_t>& lc, 
			const token_t& t, 
			const key_t& k)
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
					send_FIND_NODE(map_search[t].get_node_endpoint(), k, t);
				}
			}
		}
		
		void replyIterativeValueSearch(
			const std::list<contact_t>& lc, 
			const token_t& t, 
			const key_t& k)
		{
			map_search[t].update_list(lc);
			// iterativeFindValue
			for (	unsigned int i = 0; 
					i < ALPHA && !map_search[t].is_node_full(); 
					++i)
				send_FIND_VALUE(map_search[t].get_node_endpoint(), k, t);
		}
		
		void replyIterativeStoreSearch(
			const std::list<contact_t>& lc, 
			const token_t& t, 
			const key_t& k)
		{
			if (map_search[t].update_list(lc)) {
				// iterativeStore
				while (!map_search[t].is_value_full())
					send_STORE(
						map_search[t].get_value_endpoint(), 
						k, 
						map_search[t].buffer, 
						t);
				// free some local space
				map_search.erase(t);
			} else {
				for (	unsigned int i = 0; 
						i < ALPHA && !map_search[t].is_node_full(); 
						++i)
					send_FIND_NODE(map_search[t].get_node_endpoint(), k, t);
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
			message_t m;
			std::stringstream ss(std::string(msg.body(), msg.body_length()));
			try {
#ifdef SERIALIZE_XML
				boost::archive::xml_iarchive xia(ss);
				xia >> BOOST_SERIALIZATION_NVP(m);
#endif // SERIALIZE_XML
#ifdef SERIALIZE_BINARY
				boost::archive::binary_iarchive xia(ss);
				xia >> m;
#endif // SERIALIZE_BINARY
				switch (m.type) {
					case message_t::SEND_PING :
						handle_SEND_PING(m);
						contact_list.add_contact(m.from_id, ep);
						break;
					case message_t::REPLY_PING :
						handle_REPLY_PING(m);
						break;
					case message_t::SEND_STORE :
						handle_SEND_STORE(m);
						contact_list.add_contact(m.from_id, ep);
						break;
					case message_t::REPLY_STORE :
						handle_REPLY_STORE(m);
						break;
					case message_t::SEND_FIND_NODE :
						handle_SEND_FIND_NODE(m);
						contact_list.add_contact(m.from_id, ep);
						break;
					case message_t::REPLY_FIND_NODE :
						handle_REPLY_FIND_NODE(m);
						break;
					case message_t::SEND_FIND_VALUE :
						handle_SEND_FIND_VALUE(m);
						contact_list.add_contact(m.from_id, ep);
						break;
					case message_t::REPLY_FIND_VALUE :
						handle_REPLY_FIND_VALUE(m);
						break;
					case message_t::NONE :
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
		
		void handle_SEND_PING(const message_t& m) {
			reply_PING(sender_endpoint_, m.token);
		}
					
		void handle_REPLY_PING(const message_t& m) {
			const boost::posix_time::time_duration tTimeout = 
			boost::posix_time::minutes(1);
			{
				if (map_ping_ttl.find(m.token) != map_ping_ttl.end()) {
					boost::posix_time::time_duration td = 
						map_ping_ttl[m.token] - update_time();
					if (td < tTimeout) {
						contact_list.add_contact(
							m.from_id, 
							sender_endpoint_);
					} else {
						std::cout 
							<< "\tTimeout no recording." 
							<< std::endl;
					}
					map_ping_ttl.erase(map_ping_ttl.find(m.token));
				} else {
					std::cerr << "\ttoken [" << m.token
						<< "] unknown." << std::endl;
				}
			}
		}
		
		void handle_SEND_STORE(const message_t& m) {
			data_item_t di = m.data;
			di.time = update_time();
			insert_db(m.to_id, di);
			reply_STORE(sender_endpoint_, m.token, di);
		}
		
		void handle_REPLY_STORE(const message_t& m) {
			if (map_store_digest.find(m.token) != map_store_digest.end()) {
				if (m.digest != map_store_digest[m.token]) {
					std::cerr 
						<< "[" << socket_.local_endpoint()
						<< "] digest missmatch storage problem?" 
						<< std::endl;
					std::cerr 
						<< m.digest << " != " 
						<< map_store_digest[m.token] << std::endl;
				} 
				map_store_digest.erase(map_store_digest.find(m.token));
			} 
		}
		
		void handle_SEND_FIND_NODE(const message_t& m) {
			reply_FIND_NODE(sender_endpoint_, m.token,	m.to_id);
		}
		
		void handle_REPLY_FIND_NODE(const message_t& m) {
			{ // check if the search exist
				if (map_search.find(m.token) == map_search.end())
					return;
			}
			std::list<contact_t> lc;
			const_list_contact_t_iterator ite = m.contact_list.begin();
			for (;ite != m.contact_list.end(); ++ite) {
				contact_t c = (*ite);
				std::string address = c.ep.address().to_string();
				if ((address == std::string("0.0.0.0")) ||
					(address == std::string("127.0.0.1")) ||
					(address == std::string("localhost"))) {
					address = sender_endpoint_.address().to_string();
					unsigned short port = c.ep.port();
					boost::asio::ip::tcp::endpoint uep(
						boost::asio::ip::address::from_string(address),
						port);
					contact_list.add_contact(c.key, uep, false);
				} else {
					contact_list.add_contact(c.key, c.ep, false);
				}
				lc.push_back(c);				
			}
			replyIterative(lc, m.token, m.to_id);
		}
		
		void handle_SEND_FIND_VALUE(const message_t& m) {
			bool is_present = false;
			{ // lock database befor check
				is_present = db_storage.find(m.to_id) != db_storage.end();
			}
			if (is_present) {
				reply_FIND_VALUE(sender_endpoint_, m.token, m.to_id, m.hint);
			} else {
				reply_FIND_NODE(sender_endpoint_, m.token, m.to_id);
			}
		}
		
		void handle_REPLY_FIND_VALUE(const message_t& m) {
			if (map_search.find(m.token) != map_search.end()) {
				map_search[m.token].value_callback(m.data_item_list);
				map_search.erase(m.token);
			}
		}

		void send_MESSAGE(
			const message_t& m,
			const boost::asio::ip::tcp::endpoint& ep) 
		{
			std::string address = ep.address().to_string();
			unsigned short port = ep.port();
			std::stringstream ss("");
			try {
#ifdef SERIALIZE_BINARY
				boost::archive::binary_oarchive xoa(ss);
				xoa << m;
#endif // SERIZLIZE_BINARY
#ifdef SERIALIZE_XML
				boost::archive::xml_oarchive xoa(ss);
				xoa << BOOST_SERIALIZATION_NVP(m);
#endif // SERIALIZE_XML
				if (address == std::string("0.0.0.0"))
					address = std::string("127.0.0.1");
				boost::asio::ip::tcp::endpoint uep(
					boost::asio::ip::address::from_string(address), 
					port);
				map_endpoint_session_iterator ite = map_endpoint_session.find(uep);
				basic_message<PACKET_SIZE> msg;
				msg.body_length(ss.str().size());
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
			const boost::asio::ip::tcp::endpoint& ep,
			token_t token = random_bitset<TOKEN_SIZE>())
		{
			try {
				message_t m(message_t::SEND_PING, id_, token);	
				{
					map_ping_ttl[m.token] = update_time();
				}
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
			const key_t& to_id,
			const data_item_t& cbf,
			token_t token = random_bitset<TOKEN_SIZE>())
		{
			try {
				message_t m(message_t::SEND_STORE, id_, token);
				m.to_id = to_id;
				m.data = cbf;
				std::stringstream ss("");
				ss << cbf.title << ":" << cbf.data;
				digest_t digest = digest_from_string(ss.str());
				{
					map_store_digest[token] = digest;
				}
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
			const key_t& to_id,
			token_t token = random_bitset<TOKEN_SIZE>())
		{
			try {
				message_t m(message_t::SEND_FIND_NODE, id_, token);
				m.to_id = to_id;
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
			const key_t& to_id,
			token_t token = random_bitset<TOKEN_SIZE>(),
			const std::string& hint = std::string(""))
		{
			try {
				message_t m(message_t::SEND_FIND_VALUE, id_, token);	
				m.to_id = to_id;
				m.hint = hint;
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
			const token_t& tok)
		{
			try {
				message_t m(message_t::REPLY_PING, id_, tok);
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
			const token_t& tok,
			const data_item_t& cbf)
		{
			try {
				digest_t digest;
				std::stringstream wss("");
				wss << cbf.title << ":" << cbf.data;
				digest = digest_from_string(wss.str());
				message_t m(message_t::REPLY_STORE, id_, tok);
				m.digest = digest;
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
			const token_t& tok,
			const key_t& to_id)
		{
			try {
				message_t m(message_t::REPLY_FIND_NODE, id_, tok);
				m.to_id = to_id;
				for (bucket_iterator bit = contact_list.begin(); 
					bit != contact_list.end();) 
				{
					m.contact_list.push_back(bit->second);
					++bit;
					if ((m.contact_list.size() >= ALPHA) || 
						(bit == contact_list.end()))
					{
						send_MESSAGE(m, ep);
						m.contact_list.clear();
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
			const token_t& tok,
			const key_t& to_id,
			const std::string& hint = std::string(""))
		{
			try {
				message_t m(message_t::REPLY_FIND_VALUE, id_, tok);
				m.to_id = to_id;
				{
					size_t count = db_storage.count(to_id);
					db_key_data_item_iterator site = db_storage.find(to_id);
					for (unsigned int i = 0; i < count; ++i, ++site) {
						std::string value = site->second.title;
						if (value.find(hint) != std::string::npos)
							m.data_item_list.push_back(site->second);
					}
				}
				if (m.data_item_list.size() > 0) {
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

