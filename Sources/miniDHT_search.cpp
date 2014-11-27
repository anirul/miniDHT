/*
 * Copyright (c) 2009-2012, anirul
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
 * THIS SOFTWARE IS PROVIDED BY anirul ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL anirul BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <list>
#include <map>
#include <string>
#include <boost/function.hpp>

#include "miniDHT_search.h"

namespace miniDHT {

	search::search(
		const search::key_t& src, 
		const search::key_t& dest, 
		search_type_t t) : 
			ttl(update_time()), 
			destination(dest), 
			source(src), 
			search_type(t),
			node_callback_valid(false),
			buffer()
	{
		short_list.clear();
		bucket_nb = common_bits(
			string_to_key<KEY_SIZE>(destination), 
			string_to_key<KEY_SIZE>(source));
	}

	search::search() : buffer() {}

	search::~search() {}

	bool search::is_bucket_full() const {
		if (short_list_in_bucket() >= BUCKET_SIZE) return true;
		return is_node_full();
	}

	int search::nb_node_left() const {
		int i = 0;
		std::list<contact_proto>::const_iterator ite;
		for (ite = short_list.begin(); ite != short_list.end(); ++ite) {
			assert(ite->ep().address() != std::string(""));
			assert(ite->ep().port() != std::string(""));
			if (map_node_busy.find(ite->key()) == map_node_busy.end())
				++i;
		}
		return i;
	}

	bool search::is_node_full() const {
		std::list<contact_proto>::const_iterator ite;
		for (ite = short_list.begin(); ite != short_list.end(); ++ite) {
			assert(ite->ep().address() != std::string(""));
			assert(ite->ep().port() != std::string(""));
			if (map_node_busy.find(ite->key()) == map_node_busy.end())
				return false;
		}
		return true; 
	}

	bool search::is_value_full() const {
		std::list<contact_proto>::const_iterator ite;
		for (ite = short_list.begin(); ite != short_list.end(); ++ite) {
			assert(ite->ep().address() != std::string(""));
			assert(ite->ep().port() != std::string(""));
			if (map_value_busy.find(ite->key()) == map_value_busy.end())
				return false;
		}
		return true;
	}

	unsigned int search::short_list_in_bucket() const {
		unsigned int nb = 0;
		std::list<contact_proto>::const_iterator ite;
		for (ite = short_list.begin(); ite != short_list.end(); ++ite) {
			assert(ite->ep().address() != std::string(""));
			assert(ite->ep().port() != std::string(""));
			if (common_bits(
					string_to_key<KEY_SIZE>(ite->key()), 
					string_to_key<KEY_SIZE>(destination)) 
					== bucket_nb)
				++nb;
		}
		return nb;
	}

	endpoint_proto search::get_node_endpoint() {
		std::list<contact_proto>::const_iterator ite;
		for (ite = short_list.begin(); ite != short_list.end(); ++ite) {
			assert(ite->ep().address() != std::string(""));
			assert(ite->ep().port() != std::string(""));
			if (map_node_busy.find(ite->key()) == map_node_busy.end()) {
				map_node_busy[ite->key()] = true;
				return ite->ep();
			}
		}
		throw std::string("no endpoint found!");
	}

	search::key_t search::get_node_key() {
		std::list<contact_proto>::const_iterator ite;
		for (ite = short_list.begin(); ite != short_list.end(); ++ite) {
			assert(ite->key() != std::string(
				"00000000000000000000000000000000"\
				"00000000000000000000000000000000"));
			assert(ite->ep().address() != std::string(""));
			assert(ite->ep().port() != std::string(""));			
			if (map_node_busy.find(ite->key()) == map_node_busy.end()) {
				map_node_busy[ite->key()] = true;
				return ite->key();
			}
		}
		throw std::string("no key found!");
	}

	endpoint_proto search::get_value_endpoint() {
		std::list<contact_proto>::iterator ite;
		for (ite = short_list.begin(); ite != short_list.end(); ++ite) {
			assert(ite->ep().address() != std::string(""));
			assert(ite->ep().port() != std::string(""));
			if (map_value_busy.find(ite->key()) == map_value_busy.end()) {
				map_value_busy[ite->key()] = true;
				return ite->ep();
			}
		}
		throw std::runtime_error("no endpoint found!");
	}
		
	search::key_t search::get_value_key() {
		std::list<contact_proto>::iterator ite;
		for (ite = short_list.begin(); ite != short_list.end(); ++ite) {
			assert(ite->key() != std::string(
				"00000000000000000000000000000000"\
				"00000000000000000000000000000000"));
			assert(ite->ep().address() != std::string(""));
			assert(ite->ep().port() != std::string(""));
			if (map_value_busy.find(ite->key()) == map_value_busy.end()) {
				map_value_busy[ite->key()] = true;
				return ite->key();
			}
		}
		throw std::runtime_error("no key found!");
	}

	bool search::update_list(const std::list<contact_proto>& lc) {
		if (lc.size() == 0) return is_bucket_full();
		map_key_contact_proto_t map_sorted;
		std::list<contact_proto>::const_iterator itc;
		std::bitset<KEY_SIZE> dest_bs = string_to_key<KEY_SIZE>(destination);
		for (itc = lc.begin(); itc != lc.end(); ++itc) {
			std::bitset<KEY_SIZE> source_bs = 
				string_to_key<KEY_SIZE>(itc->key());
			std::bitset<KEY_SIZE> result = source_bs ^ dest_bs;
			map_sorted[result.to_string()] = (*itc);
		}
		std::list<contact_proto>::iterator itl;
		for (itl = short_list.begin(); itl != short_list.end(); ++itl) {
			std::bitset<KEY_SIZE> source_bs = 
				string_to_key<KEY_SIZE>(itl->key());
			std::bitset<KEY_SIZE> result = source_bs ^ dest_bs;
			map_sorted[result.to_string()] = (*itl);
		}
		std::list<contact_proto> temp;
		map_key_contact_proto_iterator itm;
		for (itm = map_sorted.begin(); itm != map_sorted.end(); ++itm) {
			temp.push_back(itm->second);
			if (temp.size() >= BUCKET_SIZE) break;
		}
		if (short_list.size() && (short_list == temp)) 
			return true;
		short_list = temp;
//		search::key_t k = map_sorted.begin()->first;
//		std::cout << k << "\t" << map_sorted.size() << std::endl;
		return is_bucket_full();
	}
		
	void search::call_node_callback() {
		std::list<search::key_t> lk;
		std::list<contact_proto>::iterator itc;
		for (itc = short_list.begin(); itc != short_list.end(); ++itc)
			lk.push_back(itc->key());
		node_callback(lk);
	}

} // end of namespace miniDHT

