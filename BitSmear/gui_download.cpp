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
#include <boost/regex.hpp>
#include "gui_download.h"

gui_download::gui_download(const wxString& title) 
	:	wxDialog(
			NULL, 
			wxID_ANY, 
			title, 
			wxDefaultPosition, 
			wxSize(650, 170), 
			wxSTAY_ON_TOP | wxCAPTION),
		key_ctrl_(NULL),
		panel_(NULL),
		vbox_(NULL),
		hbox_(NULL),
		title_(title)
{
	panel_ = new wxPanel(this, -1);

	vbox_ = new wxBoxSizer(wxVERTICAL);
	hbox_ = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText *st1 = new wxStaticText(
		panel_,
		wxID_ANY,
		_("Key [A-Fa-f0-9]{64}"),
		wxPoint(15, 20));
	key_ctrl_ = new wxTextCtrl(
		panel_, 
		wxID_ANY, 
		_("00000000000000000000"\
		"00000000000000000000"\
		"00000000000000000000"\
		"0000"), 
		wxPoint(15, 50),
		wxSize(600, 20));
	key_ctrl_->SetMaxLength(64);

	wxButton *okButton = new wxButton(
		this, 
		wxID_OK, 
		_("Ok"), 
      wxDefaultPosition, 
		wxSize(70, 30));
	wxButton *closeButton = new wxButton(
		this, 
		wxID_CANCEL, 
		_("Cancel"), 
		wxDefaultPosition, 
		wxSize(70, 30));

	hbox_->Add(okButton, 1);
	hbox_->Add(closeButton, 1, wxLEFT, 5);

	vbox_->Add(panel_, 1);
	vbox_->Add(hbox_, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);

	this->Start(250);
	this->SetSizer(vbox_);
	this->Center();
}

gui_download::~gui_download() {
	if (key_ctrl_) delete key_ctrl_;
	key_ctrl_ = NULL;
	if (panel_) delete panel_;
	panel_ = NULL;
}

bool gui_download::validate() const {
	std::string s = std::string(this->get_key().mb_str());
	const boost::regex e("[A-Fa-f0-9]{64}");
	return regex_match(s, e);
}

void gui_download::Notify() {
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
}

