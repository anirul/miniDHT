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

#ifndef MINIDHT_GUI_LIST_CTRL_HEADER_DEFINED
#define MINIDHT_GUI_LIST_CTRL_HEADER_DEFINED

#include "gui_frame.h"

DECLARE_EVENT_TYPE(wxID_LIST_CTRL, wxID_HIGHEST + 20)

class gui_list_ctrl : public wxListCtrl {
protected :
	long updated_;
	long selected_;
	wxListItemAttr attr_;
public :
	gui_list_ctrl(
		wxWindow* parent,
		const wxWindowID id,
		const wxPoint& pos,
		const wxSize& size,
		long style)
		: 	wxListCtrl(parent, id, pos, size, style),
			updated_(-1),
			selected_(-1),
			attr_(*wxBLUE, *wxLIGHT_GREY, wxNullFont) { }
    //void OnDropFiles(wxDropFilesEvent& event);
    void OnDrag(wxListEvent &event);
	void OnSelected(wxListEvent& event);
	void OnDeselected(wxListEvent& event);
    void OnActivated(wxListEvent& event);
	long get_selected() const { return selected_; }
private :
	wxDECLARE_NO_COPY_CLASS(gui_list_ctrl);
	DECLARE_EVENT_TABLE()
};

#endif // MINIDHT_GUI_LIST_CTRL_HEADER_DEFINED

