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

#include "gui_dht.h"

gui_dht* gui_dht::instance_ = NULL;

gui_dht* gui_dht::instance(const std::string& path) {
	if (!instance_)
		instance_ = new gui_dht(path);
	return instance_;
}

gui_dht* gui_dht::instance() {
	if (instance_) return instance_;
	return NULL;
}

void gui_dht::release() {
	if (instance_) {
		instance_->stop();
		delete instance_;
		instance_ = NULL;
	}
}

void gui_dht::start(short port) {
	if (mini_dht_) stop();
	mini_dht_ = new miniDHT_t(io_service_, port, path_);
	thread_ = new boost::thread(
		boost::bind(
			&boost::asio::io_service::run, 
			&io_service_));
}

void gui_dht::stop() {
	io_service_.stop();
	thread_->join();
	delete thread_;
	delete mini_dht_;
}

void gui_dht::ping(const std::string& hostname, short port) {
	std::stringstream ss_port("");
	ss_port << port;
	boost::asio::ip::udp::resolver resolver(io_service_);
	boost::asio::ip::udp::resolver::query query(
		boost::asio::ip::udp::v4(), 
		hostname, 
		ss_port.str());
	boost::asio::ip::udp::resolver::iterator ite = 
		resolver.resolve(query);
	mini_dht_->send_PING(*ite);
}

std::list<miniDHT_t::contact_t> gui_dht::status() {
	return mini_dht_->nodes_description();
}

