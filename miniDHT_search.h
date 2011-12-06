/*
 * Copyright (c) 2009, anirul
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
 
#ifndef MINIDHT_SEARCH_HEADER_DEFINED
#define MINIDHT_SEARCH_HEADER_DEFINED

#include "miniDHT_const.h"

namespace miniDHT {

	enum search_type_t {
		NODE_SEARCH = 0,
		VALUE_SEARCH = 1,
		STORE_SEARCH = 2
	};

	template <unsigned int KEY_SIZE, unsigned int BUCKET_SIZE>  
	class search {
	
	public :
	
		typedef std::string key_t;
		typedef std::map<key_t, bool> map_key_bool_t;
		typedef std::map<key_t, contact_proto> map_key_contact_proto_t;
		typedef typename
			std::map<key_t, contact_proto>::iterator 
			map_key_contact_proto_iterator;
		
		// callback declaration
		typedef boost::function<void (const std::list<key_t>& k)> 
			node_callback_t;
		typedef boost::function<void (const std::list<data_item_proto>& b)>  
			value_callback_t;

	protected :

		std::list<contact_proto> short_list;
		map_key_bool_t map_node_busy;
		map_key_bool_t map_value_busy;

	public :

		boost::posix_time::ptime ttl;
		key_t destination;
		key_t source;
		search_type_t search_type;
		bool node_callback_valid;
		node_callback_t node_callback;
		value_callback_t value_callback;
		data_item_proto buffer;
		unsigned int bucket_nb;
		std::string hint;
	
	public :
				
		search(
			const key_t& src, 
			const key_t& dest, 
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
		search() : buffer() {}
		virtual ~search() {}
		
		bool is_bucket_full() const {
			if (short_list_in_bucket() >= BUCKET_SIZE) return true;
			return is_node_full();
		}
		
		int nb_node_left() const {
			int i = 0;
			std::list<contact_proto>::const_iterator ite;
			for (ite = short_list.begin(); ite != short_list.end(); ++ite) {
				assert(ite->ep().address() != std::string(""));
				assert(ite->ep().port() != std::string(""));
				if (map_node_busy.find(string_to_key<KEY_SIZE>(ite->key())) == 
						map_node_busy.end())
					++i;
			}
			return i;
		}
		
		bool is_node_full() const {
			std::list<contact_proto>::const_iterator ite;
			for (ite = short_list.begin(); ite != short_list.end(); ++ite) {
				assert(ite->ep().address() != std::string(""));
				assert(ite->ep().port() != std::string(""));
				if (map_node_busy.find(ite->key()) == map_node_busy.end())
					return false;
			}
			return true; 
		}

		bool is_value_full() const {
			std::list<contact_proto>::const_iterator ite;
			for (ite = short_list.begin(); ite != short_list.end(); ++ite) {
				assert(ite->ep().address() != std::string(""));
				assert(ite->ep().port() != std::string(""));
				if (map_value_busy.find(ite->key()) == map_value_busy.end())
					return false;
			}
			return true;
		}
		
		unsigned int short_list_in_bucket() const {
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
		
		endpoint_proto get_node_endpoint() {
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
		
		endpoint_proto get_value_endpoint() {
			std::list<contact_proto>::iterator ite;
			for (ite = short_list.begin(); ite != short_list.end(); ++ite) {
				assert(ite->ep().address() != std::string(""));
				assert(ite->ep().port() != std::string(""));
				if (map_value_busy.find(ite->key()) == map_value_busy.end()) {
					map_value_busy[ite->key()] = true;
					return ite->ep();
				}
			}
			throw std::string("no endpoint found!");
		}
		
		bool update_list(const std::list<contact_proto>& lc) {
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
//			key_t k = map_sorted.begin()->first;
//			std::cout << k << "\t" << map_sorted.size() << std::endl;
			return is_bucket_full();
		}
		
		void call_node_callback() {
			std::list<key_t> lk;
			std::list<contact_proto>::iterator itc;
			for (itc = short_list.begin(); itc != short_list.end(); ++itc)
				lk.push_back(itc->key());
			node_callback(lk);
		}
	};

} // end namespace miniDHT

#endif // MINIDHT_SEARCH_HEADER_DEFINED

