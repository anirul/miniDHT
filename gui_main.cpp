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
#include <wx/dataview.h>
#include <wx/spinctrl.h>
#include <wx/stdpaths.h>
#ifdef __WXMAC__
#include <ApplicationServices/ApplicationServices.h>
#endif // __WXMAC__
#include "gui_main.h" 
#include "gui_connect.h"
#include "gui_info.h"
#include "gui_network_status.h"
 
IMPLEMENT_APP(gui_main)

DECLARE_EVENT_TYPE(wxID_CONNECT, wxID_HIGHEST + 1)
DECLARE_EVENT_TYPE(wxID_UPLOAD, wxID_HIGHEST + 2)
DECLARE_EVENT_TYPE(wxID_DOWNLOAD, wxID_HIGHEST + 3)
DECLARE_EVENT_TYPE(wxID_NETWORK_STATUS, wxID_HIGHEST + 4)

DECLARE_EVENT_TYPE(wxID_TOOLBAR_CONNECT, wxHIGHEST + 11)
DECLARE_EVENT_TYPE(wxID_TOOLBAR_DOWNLOAD, wxHIGHEST + 12)
DECLARE_EVENT_TYPE(wxID_TOOLBAR_UPLOAD, wxHIGHEST + 13)
DECLARE_EVENT_TYPE(wxID_TOOLBAR_INFO, wxID_HIGHEST + 14)
DECLARE_EVENT_TYPE(wxID_TOOLBAR_CANCEL, wxID_HIGHEST + 15)
DECLARE_EVENT_TYPE(wxID_DATA_LIST_CTRL, wxID_HIGHEST + 20)

DEFINE_EVENT_TYPE(wxID_CONNECT)
DEFINE_EVENT_TYPE(wxID_UPLOAD)
DEFINE_EVENT_TYPE(wxID_DOWNLOAD)
DEFINE_EVENT_TYPE(wxID_NETWORK_STATUS)

DEFINE_EVENT_TYPE(wxID_TOOLBAR_CONNECT)
DEFINE_EVENT_TYPE(wxID_TOOLBAR_UPLOAD)
DEFINE_EVENT_TYPE(wxID_TOOLBAR_DOWNLOAD)
DEFINE_EVENT_TYPE(wxID_TOOLBAR_INFO)
DEFINE_EVENT_TYPE(wxID_TOOLBAR_CANCEL)
DEFINE_EVENT_TYPE(wxID_DATA_LIST_CTRL)
 
BEGIN_EVENT_TABLE(gui_main, wxApp)
	EVT_MENU(wxID_ABOUT, gui_main::OnAbout)
	EVT_MENU(wxID_PREFERENCES, gui_main::OnPrefs)
	EVT_MENU(wxID_EXIT, gui_main::OnQuit)
	EVT_MENU(wxID_CONNECT, gui_main::OnConnect)
	EVT_MENU(wxID_UPLOAD, gui_main::OnUpload)
	EVT_MENU(wxID_DOWNLOAD, gui_main::OnDownload)
	EVT_MENU(wxID_NETWORK_STATUS, gui_main::OnNetworkStatus)
END_EVENT_TABLE()

bool gui_main::OnInit() {
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	title_ = _("BitSmear");
	frame_ = new wxFrame(
		(wxFrame *)NULL, 
		-1,  
		title_, 
		wxPoint(50,50), 
		wxSize(800,600));
	Connect(
		wxID_ANY,
		wxEVT_TIMER,
		wxTimerEventHandler(gui_main::OnTimer),
		NULL,
		this);
	timer_.Start(250);	

#ifdef __WXMAC__
	wxApp::SetExitOnFrameDelete(false);
#endif // __WXMAC__
	wxMenuBar* menubar = new wxMenuBar();

	{ 	// Menubar
		wxMenu* file_menu = new wxMenu();

//		wxMenuBar::MacSetCommonMenuBar(menubar);
		file_menu->Append(wxID_ABOUT, _("About"));
		file_menu->Append(wxID_PREFERENCES, _("Preferences"));
		file_menu->Append(wxID_EXIT, _("Exit")); 

		file_menu->Append(wxID_CONNECT, _("Connect..."));
		file_menu->AppendSeparator();
		file_menu->Append(wxID_UPLOAD, _("Upload..."));
		file_menu->Append(wxID_DOWNLOAD, _("Download..."));

		wxMenu* tool_menu = new wxMenu();
		
		tool_menu->Append(wxID_NETWORK_STATUS, _("Network status..."));
		
		menubar->Append(file_menu, _("File"));
		menubar->Append(tool_menu, _("Tools"));
	}

	wxToolBar* toolbar = frame_->CreateToolBar(wxITEM_NORMAL | wxTB_TEXT);
	{	// toolbar
		wxImage::AddHandler(new wxPNGHandler);		
#ifdef __WXMAC__
		wxString path = wxStandardPathsCF::Get().GetResourcesDir() + _("/");
#endif // __WXMAC__
		wxBitmap download(path + _("Knob Download.png"), wxBITMAP_TYPE_PNG);
		wxBitmap upload(path + _("Knob Upload.png"), wxBITMAP_TYPE_PNG);
		wxBitmap connected(path + _("Knob Record On.png"), wxBITMAP_TYPE_PNG);
		wxBitmap disconnected(path + _("Knob Record Off.png"), wxBITMAP_TYPE_PNG);
		wxBitmap info(path + _("Knob Info.png"), wxBITMAP_TYPE_PNG);
		wxBitmap cancel(path + _("Knob Cancel.png"), wxBITMAP_TYPE_PNG);
		wxBitmap grey(path + _("Knob Grey.png"), wxBITMAP_TYPE_PNG);

		toolbar->AddTool(
			wxID_TOOLBAR_CONNECT, 
			_("Connect"), 
			connected, 
			disconnected, 
			wxITEM_NORMAL,
			_("Connect to a server"));
#ifndef __WXMAC__
		toolbar->AddSeparator();
#endif // __WXMAC__
		toolbar->AddTool(
			wxID_TOOLBAR_DOWNLOAD, 
			_("Download"), 
			download,
			grey,
			wxITEM_NORMAL,
			_("Download a file from a digest"));
		toolbar->AddTool(
			wxID_TOOLBAR_UPLOAD, 
			_("Upload"), 
			upload, 
			_("Upload a file"));
#ifndef __WXMAC__
		toolbar->AddSeparator();
#endif // __WXMAC__
		toolbar->AddTool(
			wxID_TOOLBAR_CANCEL,
			_("Cancel"),
			cancel,
			grey,
			wxITEM_NORMAL,
			_("Cancel selected action"));
		toolbar->AddTool(
			wxID_TOOLBAR_INFO,
			_("Info"),
			info,
			grey,
			wxITEM_NORMAL,
			_("Info about an action"));
		toolbar->Realize();

		frame_->Connect(
			wxID_TOOLBAR_CONNECT, 
			wxEVT_COMMAND_TOOL_CLICKED, 
			wxCommandEventHandler(gui_main::OnConnect));
		frame_->Connect(
			wxID_TOOLBAR_DOWNLOAD, 
			wxEVT_COMMAND_TOOL_CLICKED, 
			wxCommandEventHandler(gui_main::OnDownload));
		frame_->Connect(
			wxID_TOOLBAR_UPLOAD, 
			wxEVT_COMMAND_TOOL_CLICKED, 
			wxCommandEventHandler(gui_main::OnUpload));
		frame_->Connect(
			wxID_TOOLBAR_CANCEL, 
			wxEVT_COMMAND_TOOL_CLICKED, 
			wxCommandEventHandler(gui_main::OnCancel));
		frame_->Connect(
			wxID_TOOLBAR_INFO, 
			wxEVT_COMMAND_TOOL_CLICKED, 
			wxCommandEventHandler(gui_main::OnInfo));
	}
	{	// data view list control
		data_list_ctrl_ = new wxDataViewListCtrl(frame_, wxID_DATA_LIST_CTRL);
		data_list_ctrl_->AppendTextColumn("File");
		data_list_ctrl_->AppendTextColumn("Digest");
		data_list_ctrl_->AppendProgressColumn("Progress");
		data_list_ctrl_->AppendToggleColumn("Finish");
	}

	frame_->Centre();
	frame_->SetMenuBar(menubar);
	frame_->Show();

#ifdef __WXMAC__
	ProcessSerialNumber PSN;
	GetCurrentProcess(&PSN);
	TransformProcessType(&PSN, kProcessTransformToForegroundApplication);
#endif // __WXMAC__

	return true;
} 

void gui_main::OnConnect(wxCommandEvent& evt) {
	gui_connect dialog;
	if (dialog.ShowModal() == wxID_OK) {
		wxString temp;
		temp = _("TODO : Connect to server : ");
		temp += dialog.get_hostname();
		temp += _(":");
		temp += wxString::Format(wxT("%i"), dialog.get_port());
		wxMessageBox(temp);
	}
}

void gui_main::OnUpload(wxCommandEvent& evt) {
	// open file for upload
	wxMessageBox(_("TODO : Upload a file to the network."));
}

void gui_main::OnDownload(wxCommandEvent& evt) {
	// ask for a digest to be downloaded
	// if success start the download?
	wxMessageBox(_("TODO : Download from a digest."));
}

void gui_main::OnAbout(wxCommandEvent& evt) {
	wxMessageBox(_("TODO : About this application."));
}
 
void gui_main::OnPrefs(wxCommandEvent& evt) {
	wxMessageBox(_("TODO : Here are preferences."));
}

void gui_main::OnCancel(wxCommandEvent& evt) {
	// cancel some action
	wxMessageBox(_("TODO : Cancel action (Download or Upload)."));
}

void gui_main::OnInfo(wxCommandEvent& evt) {
	gui_info dialog;
	if (dialog.ShowModal() == wxID_OK)
		wxMessageBox(_("TODO : Info on some action."));
}

void gui_main::OnNetworkStatus(wxCommandEvent& evt) {
	gui_network_status dialog;
	if (dialog.ShowModal() == wxID_OK)
		wxMessageBox(_("TODO : Fillup network info."));
}

void gui_main::OnQuit(wxCommandEvent& evt) {
	wxMessageBox(_("TODO : Some cleanup?"));
	exit(0);
}

#ifdef __WXMAC__
void gui_main::MacOpenFile(const wxString& filename) {
	wxMessageBox(_("MacOpenFile(") + filename + _(")"));
}
#endif // __WXMAC__

void gui_main::OnTimer(wxTimerEvent& evt) {
	{
		static wxString moving_string[] = {
			_(".oOo.| %s |.oOo."),
			_("oOo..| %s |..oOo"),
			_("Oo..o| %s |o..oO"),
			_("o..oO| %s |Oo..o"),
			_("..oOo| %s |oOo..")
		};
		static unsigned int count = 0;
		frame_->SetTitle(
			wxString::Format(
				moving_string[count % 5], 
				title_));
		count++;
	}
}


