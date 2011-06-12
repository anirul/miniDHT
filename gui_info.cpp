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
#include "gui_info.h"

gui_info::gui_info(const wxString& title)
	: 	wxDialog(
			NULL,
			wxID_ANY,
			title,
			wxDefaultPosition,
			wxSize(250, 250),
			wxSTAY_ON_TOP | wxSYSTEM_MENU),
		title_(title)
{
	wxPanel *panel = new wxPanel(this, -1);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

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

void gui_info::Notify() {
	{
		static wxString moving_string[] = {
			_(".oOo.| %s |.oOo."),
			_("oOo..| %s |..oOo"),
			_("Oo..o| %s |o..oO"),
			_("o..oO| %s |Oo..o"),
			_("..oOo| %s |oOo..")
		};
		static unsigned int count = 0;
		this->SetTitle(
			wxString::Format(
				moving_string[count % 5], 
				title_));
		count++;
	}
}

