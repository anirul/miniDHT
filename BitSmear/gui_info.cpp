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

#include <wx/wx.h>
#include <string>
#include <sstream>
#include "gui_info.h"
#include "gui_dht.h"

gui_info::gui_info(long item, const wxString& title)
	: 	wxDialog(
			NULL,
			wxID_ANY,
			title,
			wxDefaultPosition,
			wxSize(300, 300),
			wxSTAY_ON_TOP | wxSYSTEM_MENU),
		title_(title),
		panel_(NULL),
		info_id_(NULL),
		info_name_(NULL),
		info_text_(NULL),
		item_index_(item)
{
	panel_ = new wxPanel(this, -1);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* st1 = new wxStaticText(
		panel_,
		wxID_ANY,
		_("ID"),
		wxPoint(15, 20));
	info_id_ = new wxTextCtrl(
		panel_,
		wxID_ANY,
		_("?"),
		wxPoint(65, 20),
		wxSize(200, 20),
		wxTE_READONLY);

	wxStaticText* st2 = new wxStaticText(
		panel_,
		wxID_ANY,
		_("Name"),
		wxPoint(15, 50));
	info_name_ = new wxTextCtrl(
		panel_,
		wxID_ANY,
		_("?"),
		wxPoint(65, 50),
		wxSize(200, 20),
		wxTE_READONLY);

	info_text_ = new wxStaticText(
		panel_,
		wxID_ANY,
		_("?"),
		wxPoint(15, 80),
		wxSize(200, 200));

	wxButton *okButton = new wxButton(
		this, 
		wxID_OK, 
		_("Ok"), 
      wxDefaultPosition, 
		wxSize(70, 30));

	hbox->Add(okButton, 1);

	vbox->Add(panel_, 1);
	vbox->Add(hbox, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);

	this->Start(250);
	this->SetSizer(vbox);
	this->Center();
}

gui_info::~gui_info() {
	if (info_id_) delete info_id_;
	info_id_ = NULL;
	if (info_name_) delete info_name_;
	info_name_ = NULL;
	if (info_text_) delete info_text_;
	info_text_ = NULL;
	if (panel_) delete panel_;
	panel_ = NULL;
}

void gui_info::Notify() {
	{
		static wxString moving_string[] = {
			_T(".oOo.| %s |.oOo."),
			_T("oOo..| %s |..oOo"),
			_T("Oo..o| %s |o..oO"),
			_T("o..oO| %s |Oo..o"),
			_T("..oOo| %s |oOo..")
		};
		static unsigned int count = 0;
		this->SetTitle(
			wxString::Format(
				moving_string[count % 5], 
				title_));
		count++;
	}
	if (gui_dht::instance()) {
		std::list<gui_action*> ls = gui_dht::instance()->get_action_list();
		std::list<gui_action*>::iterator ite = ls.begin();
		for (int i = 0; ite != ls.end(); ++ite, ++i) {
			if (i == item_index_) {
				gui_action* p = dynamic_cast<gui_action*>(*ite);
				{
					std::stringstream ss("");
					ss << p->get_digest();
					wxString id = _(ss.str().c_str());
					if (id != info_id_->GetValue())
						info_id_->SetValue(id);
				}
				{
					wxString name = _(p->get_filename().c_str());
					if (name != info_name_->GetValue())
						info_name_->SetValue(name);
				}
				{
					std::stringstream ss("");
					ss << p->get_filename() << std::endl;
					ss << ((p->get_action_type() == GUI_ACTION_DOWNLOAD) ?
						std::string("Download") : std::string("Upload"))
						<< std::endl;
					ss << p->get_file_size() << " Bytes" << std::endl;
					ss << p->get_packet_loaded() << " / " << p->get_packet_total()
						<< std::endl;
					wxString text = _(ss.str().c_str());
					delete info_text_;
					info_text_ = new wxStaticText(
						panel_,
						wxID_ANY,
						text,
						wxPoint(15, 80),
						wxSize(200, 200));
//					if (text != info_text_->GetValue())
//						info_text_->SetValue(text);
				}
			}
		}
	}	
}

