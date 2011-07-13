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

#include <wx/wx.h>
#include <wx/listctrl.h>
#include "gui_list_ctrl.h"

DEFINE_EVENT_TYPE(wxID_LIST_CTRL)

BEGIN_EVENT_TABLE(gui_list_ctrl, wxListCtrl)
	EVT_LIST_ITEM_SELECTED(wxID_LIST_CTRL, gui_list_ctrl::OnSelected)
	EVT_LIST_ITEM_DESELECTED(wxID_LIST_CTRL, gui_list_ctrl::OnDeselected)
END_EVENT_TABLE()

void gui_list_ctrl::OnSelected(wxListEvent& event) {
	if (GetWindowStyle() & wxLC_REPORT) {
		wxListItem info;
		info.m_itemId = event.m_itemIndex;
		info.m_col = 1;
		info.m_mask = wxLIST_MASK_TEXT;
		if (GetItem(info)) {
			selected_ = event.m_itemIndex;
		} else {
			wxMessageBox(_("Error in item selection."));
		}
	}
}

void gui_list_ctrl::OnDeselected(wxListEvent& event) {
	selected_ = -1;
}

