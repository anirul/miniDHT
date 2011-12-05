/*
 * Copyright (c) 2011, anirul
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

#include "miniDHT_const.h"

namespace miniDHT {

	bool operator==(const digest_t& a, const digest_t& b) {
		for (unsigned int i = 0; i < DIGEST_LENGTH; ++i)
			if (a.c[i] != b.c[i]) return false;
		return true;
	}

	bool operator!=(const digest_t& a, const digest_t& b) {
		return (!(a == b));
	}
	
	std::ostream& operator<< (
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
	
	std::istream& operator>> (
		std::istream& is,
		digest_t& digest) 
	{
		for (unsigned int i = 0; i < DIGEST_LENGTH; ++i) {
			char hex_hash[3];
			memset(hex_hash, 0, 3);
			is >> hex_hash[0];
			is >> hex_hash[1];
			digest.c[i] = strtoul(hex_hash, NULL, 16);
		}
		return is;
	}
	
	boost::posix_time::ptime update_time() {
		boost::posix_time::ptime t(
			boost::posix_time::microsec_clock::universal_time());
		return t;
	}
	
	void digest_sum(digest_t& digest, const void* p, size_t s) {
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
    
	int digest_file(digest_t& digest, const char* path) {
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
	
	bool operator==(
		const data_item_proto& l, 
		const data_item_proto& r) 
	{
		return ((l.title() == r.title()) && (l.data() == r.data()));
	}
	
	
	std::pair<std::string, std::string> string_to_endpoint_pair(
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

	std::string endpoint_to_string(
		const endpoint_proto& ep)
	{
		std::stringstream ss("");
		ss << ep.address() << ":" << ep.port();
		return ss.str();
	}

	std::string endpoint_to_string(
		const boost::asio::ip::tcp::endpoint& ep) 
	{
		std::stringstream ss("");
		ss << ep.address().to_string();
		ss << ":" << ep.port();
		return ss.str();
	}

	endpoint_proto endpoint_to_proto(
		const boost::asio::ip::tcp::endpoint& ep) 
	{
		endpoint_proto epp;
		epp.set_address(ep.address().to_string());
		std::stringstream ss("");
		ss << ep.port();
		epp.set_port(ss.str());
		assert(epp.address() != std::string(""));
		assert(epp.port() != std::string(""));
		return epp;
	}

	boost::asio::ip::tcp::endpoint proto_to_endpoint(
		const endpoint_proto& epp,
		boost::asio::io_service& io)
	{
		boost::asio::ip::tcp::resolver resolver(io);
		boost::asio::ip::tcp::resolver::query query(
			boost::asio::ip::tcp::v4(), 
			epp.address(), 
			epp.port());
		boost::asio::ip::tcp::resolver::iterator iterator =
			resolver.resolve(query);
		return (*iterator);
	}

	endpoint_proto create_endpoint_proto(
		const std::string& a,
		const std::string& p) 
	{
		endpoint_proto epp;
		epp.set_address(a);
		epp.set_port(p);
		return epp;
	}

	endpoint_proto create_endpoint_proto(
		const std::string& a,
		unsigned short p)
	{
		std::stringstream ss("");
		ss << p;
		return create_endpoint_proto(a, ss.str());
	}
	
	time_t to_time_t(const boost::posix_time::ptime& t) {
		using namespace boost::posix_time;
		ptime epoch(boost::gregorian::date(1970,1,1));
		time_duration::sec_type x = (t - epoch).total_seconds();
		return time_t(x);
	}

	bool operator==(const contact_proto& cp1, const contact_proto& cp2) {
		if (cp1.key() != cp2.key()) return false;
		if (cp1.ep() != cp2.ep()) return false;
		if (cp1.has_time() != cp2.has_time()) return false;
		if (cp1.has_time() && cp2.has_time()) 
			if (cp1.time() != cp2.time()) return false;
		return true;
	}

	bool operator!=(const endpoint_proto& epp1, const endpoint_proto& epp2) {
		return !(epp1 == epp2);
	}

	bool operator==(const endpoint_proto& epp1, const endpoint_proto& epp2) {
		assert(epp1.address() != std::string(""));
		assert(epp2.address() != std::string(""));
		assert(epp1.port() != std::string(""));
		assert(epp2.port() != std::string(""));
		if (epp1.address() != epp2.address()) return false;
		if (epp1.port() != epp2.port()) return false;
		return true;
	}

	bool operator<(
		const endpoint_proto& epp1, 
		const endpoint_proto& epp2) 
	{
		if (epp1.address() != epp2.address())
			return epp1.address() < epp2.address();
		return epp1.port() < epp2.port();
	}

} // end of namespace miniDHT
