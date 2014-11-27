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
 
#ifndef MINIDHT_BUCKET_HEADER_DEFINED
#define MINIDHT_BUCKET_HEADER_DEFINED

#include <map>
#include <bitset>
#include <ctime>
#include <boost/asio.hpp>
#include "miniDHT_const.h"

namespace miniDHT {
	
	class bucket : public std::multimap<unsigned int, contact_proto> {
	public :
	
		typedef std::string key_t;
		typedef 
			std::multimap<unsigned int, contact_proto>::iterator 
			iterator;
				
	protected :
		
		boost::posix_time::ptime now_;
		key_t local_key_;
		std::map<key_t, key_t> map_proximity_;
		bool changed_;
		
	public :
		
		bucket(const key_t& k);
		endpoint_proto operator[](const key_t& k);
		void remove_contact(const endpoint_proto& ep);
		void add_contact(const key_t& k, const endpoint_proto& ep);
		iterator find_key(const key_t& k);
		const std::map<std::string, std::string>& build_proximity(
			const key_t& k);
		std::string random_key_in_bucket(unsigned int bn);
	};
		
} // end namespace miniDHT

#endif // MINIDHT_BUCKET_HEADER_DEFINED

