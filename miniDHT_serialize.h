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
 
#ifndef MINIDHT_SERIALIZE_HEADER_DEFINED
#define MINIDHT_SERIALIZE_HEADER_DEFINED

#include <boost/date_time/posix_time/time_serialize.hpp>

namespace boost {
	namespace serialization {
		
		// serialize std::bitset
		template <class Archive, size_t SIZE>
		void save(
			Archive& ar, 
			const std::bitset<SIZE>& b, 
			const unsigned int version)
		{
			std::string str = b.to_string();
			ar << BOOST_SERIALIZATION_NVP(str);
		}
		template <class Archive, size_t SIZE>
		void load(
			Archive& ar,
			std::bitset<SIZE>& b,
			const unsigned int version)
		{
			std::string str;
			ar & BOOST_SERIALIZATION_NVP(str);
			b = std::bitset<SIZE>(str);
		}
		template <class Archive, size_t SIZE>
		void serialize(
			Archive& ar, 
			std::bitset<SIZE>& b, 
			const unsigned int version)
		{
			boost::serialization::split_free(ar, b, version);
		}
		
		// serialize boost::asio::ip::tcp::endpoint
		template <class Archive>
		void save(
			Archive& ar, 
			const boost::asio::ip::tcp::endpoint& ep,
			const unsigned int version)
		{
			unsigned short port = ep.port();
			std::string address = ep.address().to_string();
			ar << BOOST_SERIALIZATION_NVP(address);
			ar << BOOST_SERIALIZATION_NVP(port);
		}
		template <class Archive>
		void load(
			Archive& ar,
			boost::asio::ip::tcp::endpoint& ep,
			const unsigned int version)
		{
			unsigned short port;
			std::string address;
			ar >> BOOST_SERIALIZATION_NVP(address);
			ar >> BOOST_SERIALIZATION_NVP(port);
			ep = boost::asio::ip::tcp::endpoint(
				boost::asio::ip::address::from_string(address), 
				port);
		}
		template <class Archive>
		void serialize(
			Archive& ar,
			boost::asio::ip::tcp::endpoint& ep,
			const unsigned int version)
		{
			boost::serialization::split_free(ar, ep, version);
		}
		
	} // end of namespace serialization
} // end of namespace boost

#endif // MINIDHT_SERIALIZE_HEADER_DEFINED

