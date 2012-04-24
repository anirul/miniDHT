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
 
#ifndef MINIDHT_SEARCH_HEADER_DEFINED
#define MINIDHT_SEARCH_HEADER_DEFINED

#include "miniDHT_const.h"

namespace miniDHT {

	enum search_type_t {
		NODE_SEARCH = 0,
		VALUE_SEARCH = 1,
		STORE_SEARCH = 2
	};

	class search {
	
	public :
	
		typedef std::string key_t;
		typedef std::map<key_t, bool> map_key_bool_t;
		typedef std::map<key_t, contact_proto> map_key_contact_proto_t;
		typedef 
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
			search_type_t t);
		search();
		virtual ~search();	
		bool is_bucket_full() const;
		int nb_node_left() const;
		bool is_node_full() const;
		bool search::is_value_full() const;
		unsigned int short_list_in_bucket() const;
		endpoint_proto get_node_endpoint();
		endpoint_proto get_value_endpoint();
		bool update_list(const std::list<contact_proto>& lc);
		void call_node_callback();
	};

} // end namespace miniDHT

#endif // MINIDHT_SEARCH_HEADER_DEFINED

