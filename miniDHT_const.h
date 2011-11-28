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
#include "miniDHT_proto.pb.h"
#include <boost/date_time/posix_time/posix_time.hpp>

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
	
	inline bool operator==(const digest_t& a, const digest_t& b) {
		for (unsigned int i = 0; i < DIGEST_LENGTH; ++i)
			if (a.c[i] != b.c[i]) return false;
		return true;
	}

	inline bool operator!=(const digest_t& a, const digest_t& b) {
		return (!(a == b));
	}
	
	inline std::ostream& operator<< (
		std::ostream& os,
		const digest_t& digest)
	{
		char temp[(DIGEST_LENGTH * 2) + 1];
		memset(temp, 0, (DIGEST_LENGTH * 2) + 1);
		for (unsigned int i = 0; i < DIGEST_LENGTH; ++i)
			sprintf(&temp[i*2], "%02x", (int)digest.c[i]);
		os << temp;
		return os;
	}
	
	inline std::istream& operator>> (
		std::istream& is,
		digest_t& digest) 
	{
		for (unsigned int i = 0; i < DIGEST_LENGTH; ++i) {
			std::string hex_hash = "";
			is.width(2);
			is >> hex_hash;
			digest.c[i] = strtoul(hex_hash.c_str(), NULL, 16);
		}
		return is;
	}
	
	const size_t digest_size = sizeof(digest_t);

	inline boost::posix_time::ptime update_time() {
		boost::posix_time::ptime t(
			boost::posix_time::microsec_clock::universal_time());
		return t;
	}
	
	inline void digest_sum(digest_t& digest, const void* p, size_t s) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
		EVP_MD_CTX* ctx = EVP_MD_CTX_create();
		unsigned int digest_size = DIGEST_LENGTH;
		EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
		EVP_DigestUpdate(ctx, p, s);
		EVP_DigestFinal_ex(ctx, digest.c, &digest_size);
		EVP_MD_CTX_destroy(ctx);
#pragma clang diagnostic pop
	}
    
	inline int digest_file(digest_t& digest, const char* path) {
		const size_t buf_size = 32768;
		FILE* file = fopen(path, "rb");
		if (!file) return -534;
		size_t bytes_read = 0;
		char buffer[buf_size];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
		EVP_MD_CTX* ctx = EVP_MD_CTX_create();
		unsigned int digest_size = DIGEST_LENGTH;
		EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
		while ((bytes_read = fread(buffer, 1, buf_size, file))) {
			EVP_DigestUpdate(ctx, buffer, bytes_read);
		}
		EVP_DigestFinal_ex(ctx, digest.c, &digest_size);
		EVP_MD_CTX_destroy(ctx);
#pragma clang diagnostic pop
		fclose(file);
		return 0;
	}
	
	inline bool operator==(
		const data_item_proto& l, 
		const data_item_proto& r) 
	{
		return ((l.title() == r.title()) && (l.data() == r.data()));
	}
	
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

	inline std::pair<std::string, std::string> string_to_endpoint_pair(
		const std::string& str) 
	{
		std::pair<std::string, std::string> out;
		size_t sep = str.find_last_of(':');
		if (sep == std::string::npos)
			throw std::runtime_error("malformed IP!");
		if (str[0] == '[') { // IPv6
			size_t pos = str.find(']');
			if (pos == std::string::npos) 
				throw std::runtime_error("malformed IP!");
			out.first = str.substr(1, pos);
		} else { // IPv4
			out.first = str.substr(0, sep);
		}
		out.second = str.substr(sep + 1, str.size() - 1);
		return out;
	}

	inline std::string endpoint_to_string(
		const endpoint_proto& ep)
	{
		std::stringstream ss("");
		ss << ep.address() << ":" << ep.port();
		return ss.str();
	}

	inline std::string endpoint_to_string(
		const boost::asio::ip::tcp::endpoint& ep) 
	{
		std::stringstream ss("");
		ss << ep.address().to_string();
		ss << ":" << ep.port();
		return ss.str();
	}
	
	inline time_t to_time_t(const boost::posix_time::ptime& t) {
		using namespace boost::posix_time;
		ptime epoch(boost::gregorian::date(1970,1,1));
		time_duration::sec_type x = (t - epoch).total_seconds();
		return time_t(x);
	}

} // end of namespace miniDHT

#endif // MINIDHT_CONST_HEADER_DEFINED

