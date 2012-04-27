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

#include "miniDHT.h"

namespace miniDHT {

	miniDHT::miniDHT(
		boost::asio::io_service& io_service, 
		const boost::asio::ip::tcp::endpoint& ep,
		const std::string& path,
		size_t max_records)
		:	periodic_(boost::posix_time::minutes(PERIODIC)),
			id_(key_to_string(local_key<KEY_SIZE>(ep.port(), path))),
			periodic_io_(),
			max_records_(max_records),
			io_service_(io_service),
			acceptor_(io_service, ep),
			dt_(periodic_io_, boost::posix_time::seconds(random() % 120)),
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

	miniDHT::~miniDHT() {
		periodic_io_.stop();
//		periodic_thread_->join();
		delete periodic_thread_;
	}

	void miniDHT::iterativeStore(
		const key_t& k, 
		const data_item_proto& b) 
	{
		boost::mutex::scoped_lock lock_it(giant_lock_);
		iterativeStore_nolock(k, b);
	}

	void miniDHT::iterativeStore_nolock(
		const key_t& k, 
		const data_item_proto& b)
	{
		token_t t = random_bitset<TOKEN_SIZE>().to_ulong();
		{
			map_search[t] = search_t(id_, k, STORE_SEARCH);
			// save it temporary in the local DB
			map_search[t].buffer = b;
		}
		startNodeLookup(t, k);
	}

	void miniDHT::iterativeFindNode(const std::string& k) {
		boost::mutex::scoped_lock lock_it(giant_lock_);
		try {
			iterativeFindNode_nolock(k);
		} catch (std::exception& ex) {
			giant_lock_.unlock();
			throw ex;
		}
	}

	void miniDHT::iterativeFindNode_nolock(
		const key_t& k)
	{
		token_t t = random_bitset<TOKEN_SIZE>().to_ulong();
		map_search[t] = search_t(id_, k, NODE_SEARCH);
		map_search[t].node_callback_valid = false;
		startNodeLookup(t, k);
	}

	void miniDHT::iterativeFindValue(
		const key_t& k, 
		const value_callback_t& c,
		const std::string& hint) 
	{
		boost::mutex::scoped_lock lock_it(giant_lock_);
		try {
			token_t t = random_bitset<TOKEN_SIZE>().to_ulong();
			{
				map_search[t] = search_t(id_, k, VALUE_SEARCH);
				map_search[t].value_callback = c;
				map_search[t].hint = hint;
			}
			startNodeLookup(t, k);
		} catch (std::exception& ex) {
			giant_lock_.unlock();
			throw ex;
		}
	}

	std::list<contact_proto> miniDHT::nodes_description() {
		boost::mutex::scoped_lock lock_it(giant_lock_);
		try {
			std::list<contact_proto> ls;
			unsigned int last_bucket = 0;
			bucket_iterator ite = contact_list.begin();
			for (; ite != contact_list.end(); ++ite) {
				unsigned int new_bucket = ite->first;
				assert((ite->first) ? ite->first >= last_bucket : true);
				last_bucket = ite->first;
				ls.push_back(ite->second);
			}
			return ls;
		} catch (std::exception& ex) {
			giant_lock_.unlock();
			throw ex;
		}
	}

	size_t miniDHT::storage_size() { 
		boost::mutex::scoped_lock lock_it(giant_lock_);
		try {
			return db_storage.size(); 
		} catch (std::exception& ex) {
			giant_lock_.unlock();
			throw ex;
		}
	}

	size_t miniDHT::bucket_size() { 
		boost::mutex::scoped_lock lock_it(giant_lock_);
		try {
			return db_backup.size(); 
		} catch (std::exception& ex) {
			giant_lock_.unlock();
			throw ex;
		}
	}

	const std::string& miniDHT::get_local_key() const { 
		return id_; 
	}

	const boost::asio::ip::tcp::endpoint miniDHT::get_local_endpoint() { 
		boost::mutex::scoped_lock lock_it(giant_lock_);
		try {
			return socket_.local_endpoint(); 
		} catch (std::exception& ex) {
			giant_lock_.unlock();
			throw ex;
		} 
	}

	size_t miniDHT::storage_wait_queue() const { 
		return map_store_check_val.size(); 
	}

	void miniDHT::set_max_record(size_t val) { 
		boost::mutex::scoped_lock lock_it(giant_lock_);
		try {
			max_records_ = val; 
		} catch (std::exception& ex) {
			giant_lock_.unlock();
			throw ex;
		}
	}

	size_t miniDHT::get_max_record() const { 
		return max_records_; 
	}

	void miniDHT::restore_from_backup(
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
				endpoint_proto epp;
				epp.set_address(endpoint_pair.first);
				epp.set_port(endpoint_pair.second);
				send_PING_nolock(epp);
			} catch (std::exception& e) {
				std::cerr 
					<< "Contacting Host -> " << endpoint_pair.first 
					<< ":" << endpoint_pair.second 
					<< " -> exception : " << e.what() << std::endl;
			}
		}
	}
		
	void miniDHT::insert_db(const key_t& k, const data_item_proto& d) {
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
	void miniDHT::periodic() {
		try { // check timeout on key
			boost::mutex::scoped_lock lock_it(giant_lock_);
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
					send_PING_nolock(itc->second.key());
					db_backup.remove(itc->second.key());
					contact_list.erase(itc);
					itc = contact_list.begin();
					continue;
				} 
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
		} catch (std::exception& ex) {
			giant_lock_.unlock();
			throw ex;
		}
		periodic_thread_->yield();
		try { // try to diversify the bucket list
			boost::mutex::scoped_lock lock_it(giant_lock_);
			for (unsigned int i = 0; i < (KEY_SIZE - 1); ++i) {
				if (contact_list.count(i)) continue;
				key_t skey = contact_list.random_key_in_bucket(i);
				iterativeFindNode_nolock(skey); 
			}
		} catch (std::exception& ex) {
			giant_lock_.unlock();
			throw ex;
		}
		periodic_thread_->yield();
		try { 
			// search if storage DB need any cleaning
			std::list<data_item_header_t> ldh;
			std::list<data_item_header_t>::iterator ite;
			{
				boost::mutex::scoped_lock lock_it(giant_lock_);
				db_storage.list_headers(ldh);
			}
			boost::posix_time::ptime check_time = update_time();
			for (ite = ldh.begin(); ite != ldh.end(); ++ite) {
				boost::posix_time::time_duration time_elapsed = 
					check_time - boost::posix_time::from_time_t(ite->time);
				if (time_elapsed > boost::posix_time::seconds(ite->ttl)) {
					// data is no more valid
					boost::mutex::scoped_lock lock_it(giant_lock_);
					db_storage.remove(ite->key, ite->title);
				} else {
					// republish
					boost::mutex::scoped_lock lock_it(giant_lock_);
					data_item_proto item;
					db_storage.find(ite->key, ite->title, item);
					item.set_ttl(item.ttl() - time_elapsed.total_seconds());
					iterativeStore_nolock(ite->key, item);
				}
				periodic_thread_->yield();
				usleep(100000);
			}
		} catch (std::exception& ex) {
			giant_lock_.unlock();
			throw ex;
		}
		try {
			boost::mutex::scoped_lock lock_it(giant_lock_);
			// call back later
			boost::posix_time::time_duration wait_time = 
				periodic_ + boost::posix_time::seconds(random() % 60);
			boost::posix_time::ptime now = update_time();
			dt_.expires_at(now + wait_time);
			dt_.async_wait(boost::bind(&miniDHT::periodic, this));
		} catch (std::exception& ex) {
			giant_lock_.unlock();
			throw ex;
		}
	}

	void miniDHT::startNodeLookup(
		const token_t& t, 
		const key_t& k) 
	{
		std::list<contact_proto> temp_list;
		// get the proximity list from the contact_list
		const std::map<key_t, key_t>& map_proximity = 
			contact_list.build_proximity(k);
		std::map<key_t, key_t>::const_iterator itp = map_proximity.begin();
		for (; itp != map_proximity.end(); ++itp)	{
			if (contact_list.find_key(itp->second) == contact_list.end())
				continue;
			contact_proto c;
			c.set_key(itp->second);
			const endpoint_proto& ep = contact_list[itp->second];
			c.mutable_ep()->set_address(ep.address());
			c.mutable_ep()->set_port(ep.port());
			assert(c.ep().address() != std::string(""));
			assert(c.ep().port() != std::string(""));
			temp_list.push_back(c);
		}
		replyIterative(temp_list, t, k);
	}

	std::map<std::string, std::string> miniDHT::build_proximity(
		const key_t& k, 
		const std::list<contact_proto>& lc)
	{
		std::map<key_t, key_t> map_proximity;
		std::list<contact_proto>::const_iterator itc;
		std::bitset<KEY_SIZE> search_key = string_to_key<KEY_SIZE>(k);
		for (itc = lc.begin(); itc != lc.end(); ++itc) {
			std::bitset<KEY_SIZE> loop_key = string_to_key<KEY_SIZE>(itc->key());
			std::bitset<KEY_SIZE> result = search_key ^ loop_key;
			map_proximity[result.to_string()] = itc->key();
		}
		return map_proximity;
	}

	void miniDHT::replyIterative(
		const std::list<contact_proto>& lc, 
		const token_t& t, 
		const key_t& k)
	{
		search_t local_search = map_search[t];
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

	void miniDHT::replyIterativeNodeSearch(
		const std::list<contact_proto>& lc, 
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
				send_FIND_NODE(map_search[t].get_node_key(), k, t);
			}
		}
	}
		
	void miniDHT::replyIterativeValueSearch(
		const std::list<contact_proto>& lc, 
		const token_t& t, 
		const key_t& k)
	{
		map_search[t].update_list(lc);
		// iterativeFindValue
		for (	unsigned int i = 0; 
				i < ALPHA && !map_search[t].is_node_full(); 
				++i)
			send_FIND_VALUE(map_search[t].get_node_key(), k, t);
	}
		
	void miniDHT::replyIterativeStoreSearch(
		const std::list<contact_proto>& lc, 
		const token_t& t, 
		const key_t& k)
	{
		assert(k != std::string(
			"00000000000000000000000000000000"\
			"00000000000000000000000000000000"));
		if (map_search[t].update_list(lc)) {
			// iterativeStore
			while (!map_search[t].is_value_full())
				send_STORE(
					map_search[t].get_value_key(), 
					k, 
					map_search[t].buffer, 
					t);
				// free some local space
				map_search.erase(t);
			} else {
				for (	unsigned int i = 0; 
						i < ALPHA && !map_search[t].is_node_full(); 
						++i)
				send_FIND_NODE(map_search[t].get_node_key(), k, t);
		}
	}

	void miniDHT::start_accept() {
		session<PACKET_SIZE>* new_session = new session<PACKET_SIZE>(
			io_service_,
			boost::bind(
				&miniDHT::handle_receive,
				this,
				_1, 
				_2),
			boost::bind(
				&miniDHT::handle_disconnect,
				this,
				_1),
			map_ep_proto_session);
		acceptor_.async_accept(
			new_session->socket(),
			boost::bind(
				&miniDHT::handle_accept,
				this,
				new_session,
				boost::asio::placeholders::error));
	}

	void miniDHT::handle_accept(
		session<PACKET_SIZE>* ps,
		const boost::system::error_code& error) 
	{
		try {
			boost::mutex::scoped_lock lock_it(giant_lock_);
			if (!error) {
				ps->start();
			}
			start_accept();
		} catch (std::exception& ex) {
			giant_lock_.unlock();
			throw ex;
		}
	}
		
	void miniDHT::handle_receive(
		const boost::asio::ip::tcp::endpoint& ep,
		const basic_message<PACKET_SIZE>& msg)	
	{
		boost::mutex::scoped_lock lock_it(giant_lock_);
		std::stringstream ss(std::string(msg.body(), msg.body_length()));
		try {
			sender_endpoint_ = ep;
			endpoint_proto epp = endpoint_to_proto(ep);
			message_proto m;
			m.ParseFromIstream(&ss);
			switch (m.type()) {
				case message_proto::SEND_PING :
					handle_SEND_PING(m);
					contact_list.add_contact(m.from_id(), epp);
					break;
				case message_proto::REPLY_PING :
					handle_REPLY_PING(m);
					break;
				case message_proto::SEND_STORE :
					handle_SEND_STORE(m);
					contact_list.add_contact(m.from_id(), epp);
					break;
				case message_proto::REPLY_STORE :
					handle_REPLY_STORE(m);	
					break;
				case message_proto::SEND_FIND_NODE :
					handle_SEND_FIND_NODE(m);
					contact_list.add_contact(m.from_id(), epp);
					break;
				case message_proto::REPLY_FIND_NODE :
					handle_REPLY_FIND_NODE(m);
					break;
				case message_proto::SEND_FIND_VALUE :
					handle_SEND_FIND_VALUE(m);
					contact_list.add_contact(m.from_id(), epp);
					break;
				case message_proto::REPLY_FIND_VALUE :
					handle_REPLY_FIND_VALUE(m);
					break;
				case message_proto::NONE :
				default :
					break;
			}
		} catch (std::exception& ex) {
			giant_lock_.unlock();
			std::cerr 
				<< "[" << socket_.local_endpoint()
				<< "] error in deserializing message (dump)." 
				<< std::endl
				<< "\texception : " << ex.what() << std::endl;
			std::cerr
				<< ss.str() << std::endl;
			throw ex;
		}
	}

	void miniDHT::handle_disconnect(const boost::asio::ip::tcp::endpoint& ep) {
		contact_list.remove_contact(endpoint_to_proto(ep));
	}

	void miniDHT::handle_SEND_PING(const message_proto& m) {
		reply_PING(m.from_id(), m.token());
	}

	void miniDHT::handle_REPLY_PING(const message_proto& m) {
		const boost::posix_time::time_duration tTimeout = 
			boost::posix_time::minutes(1);
		{
			if (map_ping_ttl.find(m.token()) != map_ping_ttl.end()) {
				boost::posix_time::time_duration td = 
					map_ping_ttl[m.token()] - update_time();
				if (td < tTimeout) {
					endpoint_proto epp = endpoint_to_proto(sender_endpoint_);
					contact_list.add_contact(m.from_id(), epp);
				} else {
					std::cerr 
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
		
	void miniDHT::handle_SEND_STORE(const message_proto& m) {
		data_item_proto di(m.data_item());
		di.set_time(to_time_t(update_time()));
		insert_db(m.to_id(), di);
		reply_STORE(m.from_id(), m.token(), di);
	}
		
	void miniDHT::handle_REPLY_STORE(const message_proto& m) {
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
		
	void miniDHT::handle_SEND_FIND_NODE(const message_proto& m) {
		reply_FIND_NODE(m.from_id(), m.token());
	}
		
	void miniDHT::handle_REPLY_FIND_NODE(const message_proto& m) {
		{ // check if the search exist
			if (map_search.find(m.token()) == map_search.end())
				return;
		}
		std::list<contact_proto> lc;
		for (int i = 0; i < m.contact_list_size(); ++i) {
			contact_proto c = m.contact_list(i);
			assert(c.ep().address() != std::string());
			assert(c.ep().port() != std::string());
			// contact_list.add_contact(c.key(), c.ep(), false);
			send_PING_nolock(c.key());
			lc.push_back(c);				
		}
		replyIterative(lc, m.token(), m.to_id());
	}
		
	void miniDHT::handle_SEND_FIND_VALUE(const message_proto& m) {
		bool is_present = false;
		is_present = (db_storage.count(m.to_id()) != 0);
		if (is_present) {
			reply_FIND_VALUE(
				m.from_id(), 
				m.token(), 
				m.hint());
		} else {
			reply_FIND_NODE(
				m.from_id(), 
				m.token());
		}
	}
		
	void miniDHT::handle_REPLY_FIND_VALUE(const message_proto& m) {
		if (map_search.find(m.token()) != map_search.end()) {
			std::list<data_item_proto> ld;
			for (int i = 0; i < m.data_item_list_size(); ++i)
				ld.push_back(m.data_item_list(i));
			map_search[m.token()].value_callback(ld);
			map_search.erase(m.token());
		}
	}

	void miniDHT::send_MESSAGE(const message_proto& m) {
		assert(m.to_id() != std::string(
			"00000000000000000000000000000000"\
			"00000000000000000000000000000000"));
		bucket::iterator ite = contact_list.find_key(m.to_id());
		if (ite == contact_list.end())
			throw std::runtime_error("No route to key : " + m.to_id());
		send_MESSAGE(m, ite->second.ep());
	}

	void miniDHT::send_MESSAGE(
		const message_proto& m,
		const endpoint_proto& epp) 
	{
		assert(epp.address() != std::string(""));
		assert(epp.port() != std::string(""));
		std::stringstream ss("");
		try {
			m.SerializeToOstream(&ss);
			boost::asio::ip::tcp::endpoint uep;
			uep = proto_to_endpoint(epp, io_service_);
			map_ep_proto_session_iterator ite = 
				map_ep_proto_session.find(
					endpoint_to_proto(uep));
			basic_message<PACKET_SIZE> msg(ss.str().size());
			std::memcpy(msg.body(), &(ss.str())[0], ss.str().size());
			msg.listen_port(listen_port_);
			msg.encode_header();
			if (ite == map_ep_proto_session.end())	{
				session<PACKET_SIZE>* new_session = new session<PACKET_SIZE>(
					io_service_,
					boost::bind(
						&miniDHT::handle_receive,
						this,
						_1, 
						_2),
					boost::bind(
						&miniDHT::handle_disconnect,
						this,
						_1),
					map_ep_proto_session);
				new_session->connect(uep);
				new_session->deliver(msg);
			} else {
				ite->second->deliver(msg);
			}
		} catch (std::exception& e) {
			std::cerr 
				<< "Exception in send_MESSAGE(" << epp.address()
				<< ":" << epp.port() << " - " 
				<< ss.str().size()
				<< ") : " << e.what() 
				<< std::endl;				
		}
	}
		
	void miniDHT::send_PING(
		const endpoint_proto& epp,
		const token_t& t)
	{
		assert(epp.address() != std::string(""));
		assert(epp.port() != std::string(""));
		boost::mutex::scoped_lock lock_it(giant_lock_);
		try {
			send_PING_nolock(epp, t);
		} catch (std::exception& ex) {
			giant_lock_.unlock();
			throw ex;
		}
	}

	void miniDHT::send_PING(
		const key_t& to_id,
		const token_t& t)
	{
		assert(to_id != std::string(
			"00000000000000000000000000000000"\
			"00000000000000000000000000000000"));
		boost::mutex::scoped_lock lock_it(giant_lock_);
		try {
			send_PING_nolock(to_id, t);
		} catch (std::exception& ex) {
			giant_lock_.unlock();
			throw ex;
		}
	}

	void miniDHT::send_PING_nolock(
		const endpoint_proto& epp,
		const token_t& t)
	{
		assert(epp.address() != std::string(""));
		assert(epp.port() != std::string(""));
		try {
			message_proto m;
			m.set_type(message_proto::SEND_PING);
			m.set_from_id(id_);
			m.set_token(t);	
			map_ping_ttl[m.token()] = update_time();
			send_MESSAGE(m, epp);
		} catch (std::exception& e) {
			std::cerr 
				<< "Exception in send_PING_nolock(" 
				<< epp.address() << ":" << epp.port()
				<< ") : " << e.what() 
				<< std::endl;				
		}
	}

	void miniDHT::send_PING_nolock(
		const key_t& to_id,
		const token_t& t)
	{
		assert(to_id != std::string(
			"00000000000000000000000000000000"\
			"00000000000000000000000000000000"));
		try {
			message_proto m;
			m.set_type(message_proto::SEND_PING);
			m.set_from_id(id_);
			m.set_to_id(to_id);
			m.set_token(t);
			map_ping_ttl[m.token()] = update_time();
			send_MESSAGE(m);
		} catch (std::exception& ex) {
			std::cerr
				<< "Exception in send_PING_nolock() : " 
				<< ex.what() 
				<< std::endl;				
		}
	}

	void miniDHT::send_STORE(
		const key_t& to_id,
		const key_t& query_id,
		const data_item_proto& cbf,
		const token_t& t)
	{
		assert(to_id != std::string(
			"00000000000000000000000000000000"\
			"00000000000000000000000000000000"));
		try {
			message_proto m;
			m.set_type(message_proto::SEND_STORE);
			m.set_from_id(id_);
			m.set_token(t);
			m.set_to_id(to_id);
			m.set_query_id(query_id);
			m.mutable_data_item()->set_ttl(cbf.ttl());
			m.mutable_data_item()->set_time(cbf.time());
			m.mutable_data_item()->set_title(cbf.title());
			m.mutable_data_item()->set_data(cbf.data());
			map_store_check_val[t] = cbf.data().size();
			send_MESSAGE(m);
		} catch (std::exception& e) {
			std::cerr 
				<< "Exception in send_STORE() : " 
				<< e.what() 
				<< std::endl;				
		}
	}

	void miniDHT::send_FIND_NODE(
		const key_t& to_id,
		const key_t& query_id,
		const token_t& t)
	{
		assert(to_id != std::string(
			"00000000000000000000000000000000"\
			"00000000000000000000000000000000"));
		try {
			message_proto m;
			m.set_type(message_proto::SEND_FIND_NODE);
			m.set_from_id(id_);
			m.set_token(t);
			m.set_to_id(to_id);
			m.set_query_id(query_id);
			send_MESSAGE(m);
		} catch (std::exception& e) {
			std::cerr 
				<< "Exception in send_FIND_NODE() : " 
				<< e.what() 
				<< std::endl;				
		}
	}

	void miniDHT::send_FIND_VALUE(
		const key_t& to_id,
		const key_t& query_id,
		const token_t& t,
		const std::string& hint)
	{
		assert(to_id != std::string(
			"00000000000000000000000000000000"\
			"00000000000000000000000000000000"));
		try {
			message_proto m;
			m.set_type(message_proto::SEND_FIND_VALUE);
			m.set_from_id(id_);
			m.set_token(t);
			m.set_to_id(to_id);
			m.set_query_id(query_id);
			m.set_hint(hint);
			send_MESSAGE(m);
		} catch (std::exception& e) {
			std::cerr 
				<< "Exception in send_FIND_VALUE() : " 
				<< e.what() 
				<< std::endl;				
		}
	}
	
	void miniDHT::reply_PING(
		const key_t& to_id,
		const token_t& t)
	{
		assert(to_id != std::string(
			"00000000000000000000000000000000"\
			"00000000000000000000000000000000"));
		try {
			message_proto m;
			m.set_type(message_proto::REPLY_PING);
			m.set_from_id(id_);
			m.set_to_id(to_id);
			m.set_token(t);
			send_MESSAGE(m);
		} catch (std::exception& e) {
			std::cerr 
				<< "Exception in reply_PING() : " 
				<< e.what() 
				<< std::endl;				
		}
	}

	void miniDHT::reply_STORE(
		const key_t& to_id,
		const token_t& t,
		const data_item_proto& cbf)
	{
		assert(to_id != std::string(
			"00000000000000000000000000000000"\
			"00000000000000000000000000000000"));
		try {
			digest_t digest;
			message_proto m;
			m.set_type(message_proto::REPLY_STORE);
			m.set_from_id(id_);
			m.set_to_id(to_id);
			m.set_token(t);
			m.set_check_val(cbf.data().size());
			send_MESSAGE(m);
		} catch (std::exception& ex) {
			std::cerr 
				<< "Exception in reply_STORE() : " << ex.what() 
				<< std::endl;				
		}
	}

	void miniDHT::reply_FIND_NODE(
		const key_t& to_id,
		const token_t& t)
	{
		assert(to_id != std::string(
			"00000000000000000000000000000000"\
			"00000000000000000000000000000000"));
		try {
			message_proto m;
			m.set_type(message_proto::REPLY_FIND_NODE);
			m.set_from_id(id_);
			m.set_token(t);
			m.set_to_id(to_id);
			for (bucket_iterator bit = contact_list.begin(); 
				bit != contact_list.end();) 
			{
				(*m.add_contact_list()) = bit->second;
				++bit;
				if ((m.contact_list_size() >= ALPHA) || 
					(bit == contact_list.end()))
				{
					send_MESSAGE(m);
					m.clear_contact_list();
				}
			}
		} catch (std::exception& e) {
			std::cerr 
				<< "\t\t\tException in reply_FIND_NODE() : " << e.what() 
				<< std::endl;
		}
	}

	void miniDHT::reply_FIND_VALUE(
		const key_t& to_id,
		const token_t& t,
		const std::string& hint)
	{
		assert(to_id != std::string(
			"00000000000000000000000000000000"\
			"00000000000000000000000000000000"));
		try {
			message_proto m;
			m.set_type(message_proto::REPLY_FIND_VALUE);
			m.set_from_id(id_);
			m.set_token(t);
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
				send_MESSAGE(m);
			}
		} catch (std::exception& e) {
			std::cerr 
				<< "Exception in reply_FIND_VALUE() : " << e.what() 
				<< std::endl;				
		}
	}

} // end namespace miniDHT

