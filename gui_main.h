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

#ifndef MINIDHT_GUI_MAIN_HEADER_DEFINED
#define MINIDHT_GUI_MAIN_HEADER_DEFINED

#include "gui_frame.h"
#include "gui_list_ctrl.h"

class gui_main : public wxApp {
	bool OnInit();
	gui_frame* frame_;
	gui_list_ctrl* list_ctrl_;
	wxString title_;
	wxString temp_path_;
	wxString ressources_path_;
	wxString download_path_;
	wxTimer timer_;
public:
	DECLARE_EVENT_TABLE()
	void OnAbout(wxCommandEvent& evt);
	void OnPrefs(wxCommandEvent& evt);
	void OnQuit(wxCommandEvent& evt);
	void OnConnect(wxCommandEvent& evt);
	void OnUpload(wxCommandEvent& evt);
	void OnDownload(wxCommandEvent& evt);
	void OnCancel(wxCommandEvent& evt);
	void OnInfo(wxCommandEvent& evt);
	void OnNetworkStatus(wxCommandEvent& evt);
	void OnClose(wxCloseEvent& evt);
	void OnTimer(wxTimerEvent& evt);
    bool OnDropFiles(const wxArrayString& filenames);
#ifdef __WXMAC__
    void MacOpenURL (const wxString& url);
    void MacOpenFiles(const wxArrayString& fileNames);
#endif // __WXMAC__
};

#endif // MINIDHT_GUI_MAIN_HEADER_DEFINED

