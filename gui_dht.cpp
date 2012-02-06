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
#include "send.h"
#include "recv.h"

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
//		FIXME crash in delete
//		delete instance_;
		instance_ = NULL;
	}
}

void gui_dht::start(unsigned short port) {
	if (mini_dht_) stop();
	boost::asio::ip::tcp::endpoint ep(
		boost::asio::ip::tcp::v4(), 
		port);
	mini_dht_ = new miniDHT_t(io_service_, ep, path_);
	thread_ = new boost::thread(
		boost::bind(
			&boost::asio::io_service::run, 
			&io_service_));
}

void gui_dht::stop() {
	while (list_action_.size())
		stop_action(dynamic_cast<gui_action*>(*list_action_.begin()));
	io_service_.stop();
	thread_->join();
	delete thread_;
	delete mini_dht_;
}

void gui_dht::ping(const std::string& hostname, unsigned short port) {
	mini_dht_->send_PING(miniDHT::create_endpoint_proto(hostname, port));
}

std::list<miniDHT::contact_proto> gui_dht::status() {
	return mini_dht_->nodes_description();
}

void gui_dht::start_upload(const std::string& file) {
	dht_send_file* p = new dht_send_file(file, mini_dht_);
	boost::asio::deadline_timer* t = new boost::asio::deadline_timer(
		io_service_, 
		boost::posix_time::seconds(1));
	t->async_wait(boost::bind(&dht_send_file::run_once, p, t));
	list_action_.push_back(dynamic_cast<gui_action*>(p));
}

void gui_dht::start_download(
	const miniDHT::digest_t& digest, 
	const std::string& path) 
{
	dht_recv_file* p = new dht_recv_file(digest, mini_dht_, path);
	boost::asio::deadline_timer* t = new boost::asio::deadline_timer(
		io_service_,
		boost::posix_time::seconds(1));
	t->async_wait(boost::bind(&dht_recv_file::run_once, p, t));
	list_action_.push_back(dynamic_cast<gui_action*>(p));	
}

bool gui_dht::stop_action(gui_action* p_action) {
	if (!p_action)
		return false;
	std::list<gui_action*>::iterator ite = list_action_.begin();
	// no find in list
	for (; ite != list_action_.end(); ++ite)
		if (p_action == (*ite)) break;
	if (ite == list_action_.end())
		return false;
	p_action->stop();
	list_action_.remove(p_action);
// FIXME this is here because if delete -> crash...
//	delete p_action;
	return true;
}

std::list<gui_action*> gui_dht::get_action_list() {
	return list_action_;
}

