/*
 * Copyright (c) 2009-2011, anirul
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
 * DISCLAIMED. IN NO EVENT SHALL Frederic DUBOUCHEDT BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MINIDHT_CONST_HEADER_DEFINED
#define MINIDHT_CONST_HEADER_DEFINED

#include <openssl/evp.h>
#ifdef WIN32
#define srandom srand
#define random rand
#endif
#include <ctime>
#include <bitset>
#include <iostream>
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include "miniDHT_proto.pb.h"

namespace miniDHT {

	const unsigned int DIGEST_LENGTH = 32;

	class digest_t {
	public :
		digest_t() {}
		union {
			unsigned char c[DIGEST_LENGTH];
			unsigned short s[DIGEST_LENGTH >> 1];
			unsigned long l[DIGEST_LENGTH >> 2];
		};
	};	

	const size_t digest_size = sizeof(digest_t);
	
	bool operator==(const digest_t& a, const digest_t& b);
	bool operator!=(const digest_t& a, const digest_t& b);
	std::ostream& operator<< (std::ostream& os, const digest_t& digest);
	std::istream& operator>> (std::istream& is, digest_t& digest);
	boost::posix_time::ptime update_time();
	void digest_sum(digest_t& digest, const void* p, size_t s);
	int digest_file(digest_t& digest, const char* path);	
	std::pair<std::string, std::string> string_to_endpoint_pair(
		const std::string& str);
	std::string endpoint_to_string(const endpoint_proto& ep);
	std::string endpoint_to_string(
		const boost::asio::ip::tcp::endpoint& ep);
	time_t to_time_t(const boost::posix_time::ptime& t);
	endpoint_proto endpoint_to_proto(
		const boost::asio::ip::tcp::endpoint& ep);
	boost::asio::ip::tcp::endpoint proto_to_endpoint(
		const endpoint_proto& epp,
		boost::asio::io_service& io);
	endpoint_proto create_endpoint_proto(
		const std::string& a, 
		const std::string& p);
	endpoint_proto create_endpoint_proto(
		const std::string& a,
		unsigned short p);
	bool operator==(const contact_proto& cp1, const contact_proto& cp2);
	bool operator==(const endpoint_proto& epp1, const endpoint_proto& epp2);
	bool operator!=(const endpoint_proto& epp1, const endpoint_proto& epp2);
	bool operator<(const endpoint_proto& epp1, const endpoint_proto& epp2);
	bool operator==(const data_item_proto& l, const data_item_proto& r);

	template <typename T>
	digest_t digest_from_string(const std::basic_string<T>& str) {
		digest_t digest;
		digest_sum(digest, str.c_str(), str.size());
		return digest;
	}
	
	template <size_t SIZE>
	std::string key_to_string(const std::bitset<SIZE>& b) {
		int i = SIZE;
		std::stringstream ret("");
		const size_t rest = SIZE % 8;
		if (rest) throw "Key (SIZE % 8) != 0";
		for (i -= 8; i >= 0; i -= 8) {
			std::bitset<8> byte;
			for (int j = 0; j < 8; ++j)
				byte[j] = b[i + j];
			if (byte.to_ulong() < 16)
				ret << "0";
			ret << std::hex << byte.to_ulong();
		}
		return ret.str();
	}

	template <size_t SIZE>
	std::bitset<SIZE> digest_to_bitset(const digest_t& digest) {
		std::bitset<SIZE> ret;
		for (unsigned int i = 0; i < SIZE; ++i)
			ret[i] = ((digest.c[(i / 8) % DIGEST_LENGTH]) & (0x01 << (i % 8))) != 0;
		return ret;
	}
	
	template <size_t SIZE>
	std::bitset<SIZE> string_to_key(const std::string& in) {
		digest_t d;
		std::stringstream ss(in);
		ss >> d;
		return digest_to_bitset<SIZE>(d);
	}
	
	template <size_t SIZE, typename T>
	std::bitset<SIZE> digest_key_from_string(const std::basic_string<T>& str) {
		return digest_to_bitset<SIZE>(digest_from_string(str));
	}
	
	template <size_t SIZE>
	std::bitset<SIZE> key_from_port(unsigned short port) {
		std::stringstream host("");
		host << boost::asio::ip::host_name();
		host << ":";
		host << port;
		return digest_key_from_string<SIZE>(host.str());		
	}
	
	template <size_t SIZE>
	std::bitset<SIZE> local_key(unsigned short port) {
		std::stringstream file_name("");
		file_name << "localhost.uid.";
		file_name << port;
		file_name << ".txt";
		std::ifstream ifs(file_name.str().c_str());
		if (ifs.is_open()) {
			std::string uid_gen_str;
			ifs >> uid_gen_str;
			ifs.close();
			return digest_key_from_string<SIZE>(uid_gen_str);
		} else {
			std::stringstream random_str("");
			random_str << boost::asio::ip::host_name();
			random_str << ":";
			random_str << port;
			random_str << ":";
			random_str << boost::posix_time::to_iso_string(update_time());
			std::ofstream ofs(file_name.str().c_str());
			ofs << random_str.str();
			ofs.close();
			return local_key<SIZE>(port);
		}

	}
	
	template <size_t SIZE>
	bool operator< (const std::bitset<SIZE>& a, const std::bitset<SIZE>& b) {
		if (a == b) return false;
		for (unsigned int pos = 1; pos <= SIZE; ++pos) {
			if (a[SIZE - pos]) { // a
				if (!b[SIZE - pos]) // !b
					return false;
			} else { // !a
				if (b[SIZE - pos]) // b
					return true;
			}
		}
		return false; // avoid warning
	}
	
	template <size_t SIZE>
	bool operator <= (const std::bitset<SIZE>& a, const std::bitset<SIZE>& b) {
		return (a < b) || (a == b);
	}
	
	template <size_t SIZE>
	class less_bitset {
	public :
		bool operator()(
			const std::bitset<SIZE>& id1, 
			const std::bitset<SIZE>& id2) const
		{
			return id1 < id2;
		}
	};

	class less_endpoint_proto {
		bool operator()(
			const endpoint_proto& ep1,
			const endpoint_proto& ep2) const
		{
			if (ep1.address() != ep2.address())
				return ep1.address() < ep2.address();
			return ep1.port() < ep2.port();
		}
	};
	
	template <size_t SIZE>
	unsigned int common_bits(
		const std::bitset<SIZE>& k1, 
		const std::bitset<SIZE>& k2) 
	{
		const int last_bit = SIZE - 1;
		for (unsigned int i = 0; i < SIZE; ++i)
			if (k1[last_bit - i] != k2[last_bit - i]) 
				return i;
		return SIZE;
	}
	
	template <size_t SIZE>
	std::bitset<SIZE> random_bitset() {
		std::bitset<SIZE> bs;
		for (unsigned int i = 0; i < SIZE; ++i)
			bs[i] = (random() % 2) != 0;
		return bs;
	}

} // end of namespace miniDHT

#endif // MINIDHT_CONST_HEADER_DEFINED

