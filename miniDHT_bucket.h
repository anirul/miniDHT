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
	class bucket : public std::multimap<unsigned int, contact<KEY_SIZE> > {
	public :
	
		typedef std::bitset<KEY_SIZE> key_t;
		typedef std::map<key_t, key_t, less_bitset<KEY_SIZE> > map_key_key_t;
		typedef typename
			std::map<key_t, key_t, less_bitset<KEY_SIZE> >::iterator
			map_key_key_iterator;
		typedef typename 
			std::multimap<unsigned int, contact<KEY_SIZE> >::iterator 
			iterator;
				
	protected :
		
		boost::posix_time::ptime now_;
		key_t local_key_;
		map_key_key_t map_proximity_;
		bool changed_;
		
	public :
		
		bucket(const std::bitset<KEY_SIZE>& k) : 
			local_key_(k), changed_(true) {}
		
		boost::asio::ip::udp::endpoint operator[](
			const std::bitset<KEY_SIZE>& k) 
		{
			unsigned int common = common_bits<KEY_SIZE>(local_key_, k);
			iterator ite = this->find(common);
			for (unsigned int i = 0; i < this->count(common); ++i) {
				if (ite->second.key == k)
					return ite->second.ep;
				++ite;
			}
			throw std::string("unknown key");
		}
		
		void add_contact(
			const std::bitset<KEY_SIZE>& k, 
			const boost::asio::ip::udp::endpoint& ep,
			bool update_ttl = true) 
		{
			now_ = update_time();
			unsigned int common = common_bits<KEY_SIZE>(local_key_, k);
			contact<KEY_SIZE> c(k, ep, now_);
			std::pair<unsigned int, contact<KEY_SIZE> > p(common, c);
			iterator ite = find_key(k);
			if (ite == this->end()) {
				size_t item_count = this->count(common);
				if (item_count >= BUCKET_SIZE) {
					ite = this->find(common);
					std::map<boost::posix_time::ptime, iterator> map_time_ite;
					for (unsigned int i = 0; i < item_count; ++i)
						map_time_ite[ite->second.ttl] = ite;
					this->erase(map_time_ite.begin()->second);
				} 
				this->insert(p);
				changed_ = true;
			} else {
				if (update_ttl)
					ite->second.ttl = now_;
			}
		}
		
		iterator find_key(const std::bitset<KEY_SIZE>& k) {
			unsigned int common = common_bits<KEY_SIZE>(local_key_, k);
			iterator ite = this->find(common);
			for (unsigned int i = 0; i < this->count(common); ++i) {
				if (ite->second.key == k)
					return ite;
				++ite;
			}
			return this->end();
		}
		
		const map_key_key_t& build_proximity(const key_t& k) {
			if (!changed_) return map_proximity_;
			map_proximity_.clear();
			iterator itc;
			for (itc = this->begin(); itc != this->end(); ++itc)
				map_proximity_[k ^ (key_t)(itc->second.key)] = itc->second.key;
			changed_ = false;
			return map_proximity_;
		}
		
		key_t random_key_in_bucket(unsigned int bn) {
			key_t ret = local_key_;
			for (unsigned int i = bn; i < KEY_SIZE; ++i)
				ret[i] = (random() % 2) != 0;
			return ret;
		}
	};
		
} // end namespace miniDHT

#endif // MINIDHT_BUCKET_HEADER_DEFINED

