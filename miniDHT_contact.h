/*
 * Copyright (c) 2010, anirul
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
 
#ifndef MINIDHT_CONTACT_HEADER_DEFINED
#define MINIDHT_CONTACT_HEADER_DEFINED

#include "miniDHT_const.h"

namespace miniDHT {

	template <size_t KEY_SIZE>
	class contact {
	protected :

		friend class boost::serialization::access;
		template <class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			ar & BOOST_SERIALIZATION_NVP(key);
			ar & BOOST_SERIALIZATION_NVP(ep);
			ar & BOOST_SERIALIZATION_NVP(ttl);
		}

	public :

		contact() {}
		contact(
			const std::bitset<KEY_SIZE>& b, 
			const boost::asio::ip::tcp::endpoint& e)
			:	key(b), ep(e) {}
		contact(
			const std::bitset<KEY_SIZE>& k, 
			const boost::asio::ip::tcp::endpoint& e, 
			const boost::posix_time::ptime& t) :
				key(k), ep(e), ttl(t) {}
	
	public :
		
		std::bitset<KEY_SIZE> key;
		boost::asio::ip::tcp::endpoint ep;
		boost::posix_time::ptime ttl;
	};
	
	template <size_t KEY_SIZE>
	bool operator==(
		const contact<KEY_SIZE>& a, 
		const contact<KEY_SIZE>& b) 
	{	
		// should be enougth
		if (a.key != b.key) return false;
		return true;
	}

	template <size_t KEY_SIZE>
	std::ostream& operator<< (
		std::ostream& os,
		const contact<KEY_SIZE>& c)
	{
		os 
			<< c.endpoint 
			<< "<<" << key_to_string<KEY_SIZE>(c.key) 
			<< ">> (" << c.ttl << ")";
		return os;
	}

} // end namespace miniDHT

#endif // MINIDHT_CONTACT_HEADER_DEFINED

