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

namespace miniDHT {

	enum search_type_t {
		NODE_SEARCH = 0,
		VALUE_SEARCH = 1,
		STORE_SEARCH = 2
	};

	template <unsigned int KEY_SIZE, unsigned int BUCKET_SIZE>  
	class search {
	
	public :
	
		typedef std::bitset<KEY_SIZE> key_t;
		typedef	std::list<contact<KEY_SIZE> > list_contact_t_t;
		typedef std::map<key_t, bool, less_bitset<KEY_SIZE> > map_key_bool_t;
		typedef typename std::list<contact<KEY_SIZE> >::iterator 
			list_contact_t_iterator;
		typedef typename std::list<contact<KEY_SIZE> >::const_iterator
			const_list_contact_t_iterator;
		typedef std::map<key_t, contact<KEY_SIZE>, less_bitset<KEY_SIZE> >
			map_key_contact_t_t;
		typedef typename
			std::map<key_t, contact<KEY_SIZE>, less_bitset<KEY_SIZE> >::iterator
			map_key_contact_t_iterator;
		
		// callback declaration
		typedef boost::function<void (std::list<key_t> k)> node_callback_t;
		typedef boost::function<void (const std::list<data_item_proto>& b)>  
			value_callback_t;

	public :

		list_contact_t_t short_list;
		map_key_bool_t map_node_busy;
		map_key_bool_t map_value_busy;
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
			bucket_nb = common_bits(destination, source);
		}
		search() : buffer() {}
		virtual ~search() {}
		
		bool is_bucket_full() {
			if (short_list_in_bucket() >= BUCKET_SIZE) return true;
			return is_node_full();
		}
		
		int nb_node_left() {
			int i = 0;
			list_contact_t_iterator ite;
			for (ite = short_list.begin(); ite != short_list.end(); ++ite)
				if (map_node_busy.find(ite->key) == map_node_busy.end())
					++i;
			return i;
		}
		
		bool is_node_full() {
			list_contact_t_iterator ite;
			for (ite = short_list.begin(); ite != short_list.end(); ++ite)
				if (map_node_busy.find(ite->key) == map_node_busy.end())
					return false;
			return true; 
		}

		bool is_value_full() {
			list_contact_t_iterator ite;
			for (ite = short_list.begin(); ite != short_list.end(); ++ite)
				if (map_value_busy.find(ite->key) == map_value_busy.end())
					return false;
			return true;
		}
		
		unsigned int short_list_in_bucket() {
			unsigned int nb = 0;
			list_contact_t_iterator ite;
			for (ite = short_list.begin(); ite != short_list.end(); ++ite)
				if (common_bits(ite->key, destination) == bucket_nb)
					++nb;
			return nb;
		}
		
		boost::asio::ip::tcp::endpoint get_node_endpoint() {
			list_contact_t_iterator ite;
			for (ite = short_list.begin(); ite != short_list.end(); ++ite) {
				if (map_node_busy.find(ite->key) == map_node_busy.end()) {
					map_node_busy[ite->key] = true;
					return ite->ep;
				}
			}
			throw std::string("no endpoint found!");
		}
		
		boost::asio::ip::tcp::endpoint get_value_endpoint() {
			list_contact_t_iterator ite;
			for (ite = short_list.begin(); ite != short_list.end(); ++ite) {
				if (map_value_busy.find(ite->key) == map_value_busy.end()) {
					map_value_busy[ite->key] = true;
					return ite->ep;
				}
			}
			throw std::string("no endpoint found!");
		}
		
		bool update_list(const list_contact_t_t& lc) {
			map_key_contact_t_t map_sorted;
			const_list_contact_t_iterator itc;
			for (itc = lc.begin(); itc != lc.end(); ++itc)
				map_sorted[itc->key ^ destination] = (*itc);
			list_contact_t_iterator itl;
			for (itl = short_list.begin(); itl != short_list.end(); ++itl)
				map_sorted[itl->key ^ destination] = (*itl);
			list_contact_t_t temp;
			map_key_contact_t_iterator itm;
			for (itm = map_sorted.begin(); itm != map_sorted.end(); ++itm) {
				temp.push_back(itm->second);
				if (temp.size() >= BUCKET_SIZE) break;
			}
			if ((short_list.size()) && (short_list == temp)) 
				return true;
			short_list = temp;
//			key_t k = map_sorted.begin()->first;
//			std::cout << k << "\t" << map_sorted.size() << std::endl;
			return is_bucket_full();
		}
		
		void call_node_callback() {
			std::list<key_t> lk;
			list_contact_t_iterator itc;
			for (itc = short_list.begin(); itc != short_list.end(); ++itc)
				lk.push_back(itc->key);
			node_callback(lk);
		}
	};

} // end namespace miniDHT

#endif // MINIDHT_SEARCH_HEADER_DEFINED

