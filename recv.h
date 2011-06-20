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

#ifndef RECV_HEADER_DEFINED
#define RECV_HEADER_DEFINED

#include "gui_action.h"

class dht_recv_file : public gui_action {
private :
	enum download_state_t {
		WAIT,
		ASKED,
		DOWNLOADED,
		DECRYPTED,
		WRITTEN
	};
	std::string file_name_;
	FILE* ofile_;
	size_t total_size_;
	size_t packet_total_;
	size_t packet_loaded_;
	bool end_;
	bool stop_;
	miniDHT::miniDHT<key_size, token_size>* pDht_;
	miniDHT::digest_t digest_;
	std::map<size_t, download_state_t> map_state_;
	std::map<size_t, std::string> map_load_;
	std::map<size_t, std::string> map_crypt_;
	std::map<std::string, size_t> map_key_string_id_;
	std::map<size_t, size_t> map_index_counter_;
public :
	dht_recv_file(
		const miniDHT::digest_t& digest,
		miniDHT::miniDHT<key_size, token_size>* pDht);
	~dht_recv_file();
protected :
	void download(size_t index);
	void received(size_t index);
	void decrypt(size_t index);
	void write(size_t index);
	void check();
	void found(const std::list<miniDHT::data_item_t>& b);
	std::string decode(const std::string& key, const std::string& data);
public : // from gui_action
	virtual void run_once(boost::asio::deadline_timer* t);
	virtual bool is_end() const { return end_; }
	virtual void stop() { stop_ = true; }
	virtual size_t get_packet_total() const { return packet_total_; }
	// WARNING this will be an estimate!
	virtual size_t get_file_size() const { return total_size_; }
	virtual size_t get_packet_loaded() const { return packet_loaded_; }
	virtual miniDHT::digest_t get_digest() const { return digest_; }
	virtual std::string get_filename() const { return file_name_; }
	virtual gui_action_type get_action_type() const
		{ return GUI_ACTION_DOWNLOAD; }
};

#endif // RECV_HEADER_DEFINED
