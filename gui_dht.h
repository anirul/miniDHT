/*
 * Copyright (c) 2011, Frederic DUBOUCHET
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
 * THIS SOFTWARE IS PROVIDED BY Frederic DUBOUCHET ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Frederic DUBOUCHET BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GUI_DHT_HEADER_DEFINED
#define GUI_DHT_HEADER_DEFINED

#include <list>
#include <string>
#include "miniDHT.h"
#include "gui_action.h"

class gui_dht {
	static gui_dht* instance_;
	miniDHT_t* mini_dht_;
	boost::thread* thread_;
	boost::asio::io_service io_service_;
	std::string path_;
	std::list<gui_action*> list_action_;
	gui_dht(const std::string& path)
		:	mini_dht_(NULL),
			thread_(NULL),
			path_(path) {}
public : // singleton stuff
	static gui_dht* instance(const std::string& path);
	// give an instance back only if already created!
	static gui_dht* instance();
	static void release();
public : // general DHT stuff
	void start(unsigned short port = 4242);
	void stop();
	void ping(const std::string& host, unsigned short port);
	std::list<miniDHT::contact_proto> status();
public : // action related
	void start_upload(const std::string& file);
	void start_download(
		const miniDHT::digest_t& digest, 
		const std::string& path = std::string("."));
	bool stop_action(gui_action* p_action);
	std::list<gui_action*> get_action_list();	
};

#endif // GUI_DHT_HEADER_DEFINED

