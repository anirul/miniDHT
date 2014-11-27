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

#ifndef GUI_ACTION_HEADER_DEFINED
#define GUI_ACTION_HEADER_DEFINED

const unsigned int key_size = 256;
const unsigned int token_size = 32;
const unsigned int wait_settle = 5;
const unsigned int buf_size = 512 * 1024; // 512k

typedef miniDHT::miniDHT miniDHT_t;

enum gui_action_type {
	GUI_ACTION_DOWNLOAD = 0,
	GUI_ACTION_UPLOAD = 1
};

class gui_action {
public :
	virtual void run_once(boost::asio::deadline_timer* t) = 0;
	virtual bool is_end() const = 0;
	virtual void stop() = 0;
	virtual size_t get_packet_total() const = 0;
	virtual size_t get_file_size() const = 0;
	virtual size_t get_packet_loaded() const = 0;
	virtual miniDHT::digest_t get_digest() const = 0;
	virtual std::string get_filename() const = 0;
	virtual gui_action_type get_action_type() const = 0;
};

#endif // GUI_ACTION_HEADER_DEFINED

