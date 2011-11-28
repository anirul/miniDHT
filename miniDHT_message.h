/*
 * Copyright (c) 2009-2010, anirul
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
 * DISCLAIMED. IN NO EVENT SHALL Frederic DUBOUCHET BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
#ifndef MINIDHT_MESSAGE_HEADER_DEFINED
#define MINIDHT_MESSAGE_HEADER_DEFINED
 
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>

#include "miniDHT_serialize.h"
#include "miniDHT_contact.h"
#include "miniDHT_const.h"

namespace miniDHT {
 
	template<unsigned int KEY_SIZE, unsigned int TOKEN_SIZE> 
	class message {
	public :
	
		enum message_type {
			NONE = 0,
			SEND_PING = 10,
			REPLY_PING = 11,
			SEND_STORE = 20,
			REPLY_STORE = 21,
			SEND_FIND_NODE = 30,
			REPLY_FIND_NODE = 31,
			SEND_FIND_VALUE = 40,
			REPLY_FIND_VALUE = 41
		};
	
	protected :
	
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			ar & BOOST_SERIALIZATION_NVP(type);
			ar & BOOST_SERIALIZATION_NVP(from_id);
			ar & BOOST_SERIALIZATION_NVP(to_id);
			ar & BOOST_SERIALIZATION_NVP(token);
			switch (type) {
				case NONE:
					break;
				case SEND_PING:
					break;
				case REPLY_PING:
					break;
				case SEND_FIND_NODE:
					break;
				case SEND_STORE:
					ar & BOOST_SERIALIZATION_NVP(data);
					break;
				case SEND_FIND_VALUE:
					ar & BOOST_SERIALIZATION_NVP(hint);
					break;
				case REPLY_STORE:
					ar & BOOST_SERIALIZATION_NVP(check_val);
					break;
				case REPLY_FIND_NODE:
					ar & BOOST_SERIALIZATION_NVP(contact_list);
					break;
				case REPLY_FIND_VALUE:
					ar & BOOST_SERIALIZATION_NVP(data_item_list);
					break;
				default:
					break;
			}
		}
		
	public :
	
		message(
			message_type t, 
			const std::bitset<KEY_SIZE>& fi,
			const std::bitset<TOKEN_SIZE>& tok)
			:	type(t), from_id(fi), token(tok) {}	
		message() : type(NONE) {}
			
	public :
	
		// header member
		message_type type;
		std::bitset<KEY_SIZE> from_id;
		std::bitset<KEY_SIZE> to_id;
		std::bitset<TOKEN_SIZE> token;
		uint16_t listen_port;
		// specific to message
		uint64_t check_val;
		data_item_t data;
		std::string hint;
		std::list<contact<KEY_SIZE> > contact_list;
		std::list<data_item_t> data_item_list;
	};
		 
} // end namespace miniDHT
 
#endif // MINIDHT_MESSAGE_HEADER_DEFINED

