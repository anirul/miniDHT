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
 
#ifndef MINIDHT_BUCKET_HEADER_DEFINED
#define MINIDHT_BUCKET_HEADER_DEFINED

#include <map>
#include <bitset>
#include <ctime>
#include <boost/asio.hpp>

namespace miniDHT {
	
	template <unsigned int BUCKET_SIZE, unsigned int KEY_SIZE>
	class bucket : public std::multimap<unsigned int, contact_proto> {
	public :
	
		typedef std::string key_t;
		typedef typename 
			std::multimap<unsigned int, contact_proto>::iterator 
			iterator;
				
	protected :
		
		boost::posix_time::ptime now_;
		key_t local_key_;
		std::map<key_t, key_t> map_proximity_;
		bool changed_;
		
	public :
		
		bucket(const key_t& k) : 
			local_key_(k), changed_(true) {}
		
		endpoint_proto operator[](
			const key_t& k) 
		{
			unsigned int common = common_bits<KEY_SIZE>(
				string_to_key<KEY_SIZE>(local_key_), 
				string_to_key<KEY_SIZE>(k));
			iterator ite = this->find(common);
			for (unsigned int i = 0; i < this->count(common); ++i) {
				if (ite->second.key() == k) 
					return ite->second.ep();
				++ite;
			}
			throw std::string("unknown key");
		}
		
		void add_contact(
			const key_t& k, 
			const endpoint_proto& ep,
			bool update_ttl = true) 
		{
			// avoid adding self to contact list
			if (k == local_key_) return;
			now_ = update_time();
			unsigned int common = common_bits<KEY_SIZE>(
				string_to_key<KEY_SIZE>(local_key_), 
				string_to_key<KEY_SIZE>(k));
			contact_proto c;
			c.set_key(k);
			c.mutable_ep()->set_address(ep.address());
			c.mutable_ep()->set_port(ep.port());
			assert(c.ep().address() != std::string(""));
			assert(c.ep().port() != std::string(""));
			// check if the IP is not already in
			for (iterator ite = this->begin(); ite != this->end(); ++ite)
				if (ite->second.ep() == ep)
					this->erase(ite);
			// insert 
			c.set_time(to_time_t(now_));
			std::pair<unsigned int, contact_proto> p(common, c);
			iterator ite = find_key(k);
			if (ite == this->end()) {
				size_t item_count = this->count(common);
				if (item_count >= BUCKET_SIZE) {
					ite = this->find(common);
					std::map<uint64_t, iterator> map_time_ite;
					for (unsigned int i = 0; i < item_count; ++i)
						map_time_ite[ite->second.time()] = ite++;
					this->erase(map_time_ite.begin()->second);
				} 
				this->insert(p);
				changed_ = true;
			} else {
				if (update_ttl)
					ite->second.set_time(to_time_t(now_));
			}
		}
		
		iterator find_key(const key_t& k) {
			unsigned int common = common_bits<KEY_SIZE>(
				string_to_key<KEY_SIZE>(local_key_), 
				string_to_key<KEY_SIZE>(k));
			iterator ite = this->find(common);
			for (unsigned int i = 0; i < this->count(common); ++i) {
				if (ite->second.key() == k)
					return ite;
				++ite;
			}
			return this->end();
		}
		
		const std::map<std::string, std::string>& build_proximity(
			const key_t& k) 
		{
			if (!changed_) return map_proximity_;
			map_proximity_.clear();
			iterator itc;
			std::bitset<KEY_SIZE> search_key = string_to_key<KEY_SIZE>(k);
			for (itc = this->begin(); itc != this->end(); ++itc) {
				std::bitset<KEY_SIZE> loop_key = 
					string_to_key<KEY_SIZE>(itc->second.key());
				// calculate the xor distance, not to preserve order
				std::bitset<KEY_SIZE> result = ~(search_key ^ loop_key);
				map_proximity_[result.to_string()] = itc->second.key();
			}
			changed_ = false;
			return map_proximity_;
		}
		
		std::string random_key_in_bucket(unsigned int bn) {
			std::bitset<KEY_SIZE> ret = string_to_key<KEY_SIZE>(local_key_);
			for (unsigned int i = bn; i < KEY_SIZE; ++i)
				ret[i] = (random() % 2) != 0;
			return key_to_string(ret);
		}
	};
		
} // end namespace miniDHT

#endif // MINIDHT_BUCKET_HEADER_DEFINED

