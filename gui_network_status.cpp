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
#include <wx/listctrl.h>
#include "gui_network_status.h"
#include "gui_dht.h"

gui_network_status::gui_network_status(const wxString& title)
	:	wxDialog(
			NULL,
			wxID_ANY,
			title,
			wxDefaultPosition,
			wxSize(600, 400),
			wxSTAY_ON_TOP | wxSYSTEM_MENU),
		title_(title),
		list_ctrl_(NULL)
{
	wxPanel *panel = new wxPanel(this, -1);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	
	list_ctrl_ = new wxListCtrl(
		panel, 
		wxID_ANY,
		wxDefaultPosition,
		wxSize(600, 400),
		wxLC_REPORT);

	list_ctrl_->InsertColumn(0, _("Id"));
	list_ctrl_->SetColumnWidth(0, 200);
	list_ctrl_->InsertColumn(1, _("TTL"));
	list_ctrl_->SetColumnWidth(1, 200);
	list_ctrl_->InsertColumn(2, _("Host"));
	list_ctrl_->SetColumnWidth(2, 200);

	wxButton *okButton = new wxButton(
		this, 
		wxID_OK, 
		wxT("Ok"), 
      wxDefaultPosition, 
		wxSize(70, 30));

	hbox->Add(okButton, 1);

	vbox->Add(panel, 1);
	vbox->Add(hbox, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);

	this->Start(250); 
	this->SetSizer(vbox);
	this->Center();
}

void gui_network_status::Notify() {
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
	std::list<miniDHT::contact_proto> ls = gui_dht::instance()->status();
	std::list<miniDHT::contact_proto>::iterator ite = ls.begin();
	while (list_ctrl_->GetItemCount() > ls.size())
		list_ctrl_->DeleteItem(0);
	while (list_ctrl_->GetItemCount() < ls.size())
		list_ctrl_->InsertItem(list_ctrl_->GetItemCount(), _("?"));
	for (int i = 0; ite != ls.end(); ++ite, ++i) {
		{
			std::stringstream ss("");
			ss << ite->key();
			wxString data = _(ss.str().c_str());
			if (data != list_ctrl_->GetItemText(i, 0))
				list_ctrl_->SetItem(i, 0, data);
		}
		{
			std::stringstream ss("");
			ss << boost::posix_time::from_time_t(ite->time());
			wxString data = _(ss.str().c_str());
			if (data != list_ctrl_->GetItemText(i, 1))
				list_ctrl_->SetItem(i, 1, data);
		}
		{
			std::stringstream ss("");
			ss << ite->ep().address() << ":" << ite->ep().port();
			wxString data = _(ss.str().c_str());
			if (data != list_ctrl_->GetItemText(i, 2))
				list_ctrl_->SetItem(i, 2, data);
		}
	}
}


