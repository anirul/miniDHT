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
 * DISCLAIMED. IN NO EVENT SHALL anirul BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SEND_HEADER_DEFINED
#define SEND_HEADER_DEFINED

const unsigned int key_size = 256;
const unsigned int token_size = 32;
const unsigned int wait_settle = 5;
const size_t buf_size = 4096;

enum upload_state_t {
	WAIT,
	LOADED,
	CRYPTED,
	UPLOADED,
	CHECKED
};

class dht_send_file {
private :
	std::string file_name_;
	FILE* ifile_;
	size_t total_size_;
	size_t packet_total_;
	bool end_;
	miniDHT::miniDHT<key_size, token_size>* pDht_;
	miniDHT::digest_t digest_;
	std::map<size_t, upload_state_t> map_state_;
	std::map<size_t, std::string> map_load_;
	std::map<size_t, std::string> map_crypt_;
	std::map<std::string, size_t> map_key_string_id_;
	std::map<size_t, size_t> map_index_counter_;
public :
	dht_send_file(
		const std::string& file_name, 
		miniDHT::miniDHT<key_size, token_size>* pDht);
	~dht_send_file();
	void load(size_t index);
	void crypt(size_t index);
	void upload(size_t index);
	void check(size_t index);
	void received(size_t index);
	void found(const std::list<miniDHT::data_item_t>& b);
	void run_once(boost::asio::deadline_timer* t);
	bool is_end() const { return end_; }
	std::string encode(const std::string& key, const std::string& data);
};

#endif // SEND_HEADER_DEFINED
