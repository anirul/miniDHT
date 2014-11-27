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
#include <wx/filefn.h>
#include <wx/listctrl.h>
#include <wx/spinctrl.h>
#include <wx/stdpaths.h>
#include <wx/filectrl.h>
#include <wx/sysopt.h>
#ifdef __WXMAC__
#include <ApplicationServices/ApplicationServices.h>
#endif // __WXMAC__
#include "gui_main.h" 
#include "gui_connect.h"
#include "gui_download.h"
#include "gui_about.h"
#include "gui_info.h"
#include "gui_network_status.h"
#include "gui_list_ctrl.h"
#include "gui_drop.h"
#include "gui_settings.h"
#include "gui_dht.h"
 
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

DEFINE_EVENT_TYPE(wxID_CONNECT)
DEFINE_EVENT_TYPE(wxID_UPLOAD)
DEFINE_EVENT_TYPE(wxID_DOWNLOAD)
DEFINE_EVENT_TYPE(wxID_NETWORK_STATUS)

DEFINE_EVENT_TYPE(wxID_TOOLBAR_CONNECT)
DEFINE_EVENT_TYPE(wxID_TOOLBAR_UPLOAD)
DEFINE_EVENT_TYPE(wxID_TOOLBAR_DOWNLOAD)
DEFINE_EVENT_TYPE(wxID_TOOLBAR_INFO)
DEFINE_EVENT_TYPE(wxID_TOOLBAR_CANCEL)
 
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
	frame_ = new gui_frame(
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
	wxSystemOptions::SetOption(_("mac.listctrl.always_use_generic"), 1);

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
	{	// set some path related stuff
        gui_settings* settings = gui_settings::instance();
        ressources_path_ = settings->find(gui_settings::RESSOURCES_PATH);
        temp_path_ = settings->find(gui_settings::TEMPFILE_PATH);
        download_path_ = settings->find(gui_settings::DOWNLOAD_PATH);
	}
	wxToolBar* toolbar = frame_->CreateToolBar(wxITEM_NORMAL | wxTB_TEXT);
	if (!wxDirExists(ressources_path_)) {
		ressources_path_ = _T("./");
		std::cout << "WARNING : Changed ressource path to \"./\"." << std::endl;
	}
	{	// toolbar
		wxImage::AddHandler(new wxPNGHandler);		
		wxBitmap download(
			ressources_path_ + _T("Knob Download.png"), 
			wxBITMAP_TYPE_PNG);
		wxBitmap upload(
			ressources_path_ + _T("Knob Upload.png"), 
			wxBITMAP_TYPE_PNG);
		wxBitmap connected(
			ressources_path_ + _T("Knob Record On.png"), 
			wxBITMAP_TYPE_PNG);
		wxBitmap disconnected(
			ressources_path_ + _T("Knob Record Off.png"), 
			wxBITMAP_TYPE_PNG);
		wxBitmap info(
			ressources_path_ + _T("Knob Info.png"), 
			wxBITMAP_TYPE_PNG);
		wxBitmap cancel(
			ressources_path_ + _T("Knob Cancel.png"), 
			wxBITMAP_TYPE_PNG);
		wxBitmap grey(
			ressources_path_ + _T("Knob Grey.png"), 
			wxBITMAP_TYPE_PNG);

		toolbar->AddTool(
			wxID_TOOLBAR_CONNECT, 
			_("Connect"), 
			connected, 
			disconnected, 
			wxITEM_NORMAL,
			_("Connect to a server"));
		toolbar->AddSeparator();
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
		toolbar->AddSeparator();
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

		this->Connect(
			wxID_TOOLBAR_CONNECT, 
			wxEVT_COMMAND_TOOL_CLICKED, 
			wxCommandEventHandler(gui_main::OnConnect));
		this->Connect(
			wxID_TOOLBAR_DOWNLOAD, 
			wxEVT_COMMAND_TOOL_CLICKED, 
			wxCommandEventHandler(gui_main::OnDownload));
		this->Connect(
			wxID_TOOLBAR_UPLOAD, 
			wxEVT_COMMAND_TOOL_CLICKED, 
			wxCommandEventHandler(gui_main::OnUpload));
		this->Connect(
			wxID_TOOLBAR_CANCEL, 
			wxEVT_COMMAND_TOOL_CLICKED, 
			wxCommandEventHandler(gui_main::OnCancel));
		this->Connect(
			wxID_TOOLBAR_INFO, 
			wxEVT_COMMAND_TOOL_CLICKED, 
			wxCommandEventHandler(gui_main::OnInfo));
	}
	{	// data view list control
		list_ctrl_ = new gui_list_ctrl(
			frame_, wxID_LIST_CTRL,
			wxDefaultPosition,
			wxDefaultSize,
			wxLC_REPORT | wxLC_SINGLE_SEL);
		list_ctrl_->InsertColumn(0, _("Progress"));
		list_ctrl_->SetColumnWidth(0, 100);
		list_ctrl_->InsertColumn(1, _("Digest"));
		list_ctrl_->SetColumnWidth(1, 350);
		list_ctrl_->InsertColumn(2, _("Bytes"));
		list_ctrl_->SetColumnWidth(2, 100);
		list_ctrl_->InsertColumn(3, _("File"));
		list_ctrl_->SetColumnWidth(3, 250);
        list_ctrl_->SetDropTarget(new gui_drop(boost::bind(&gui_main::OnDropFiles, this, _1)));
	}

	frame_->Centre();
	frame_->SetMenuBar(menubar);
	frame_->Show();

	try {
		gui_dht::instance(std::string(temp_path_.mb_str()))->start();
	} catch (std::exception& ex) {
		std::stringstream ss("Exception starting the DHT : ");
		ss << ex.what();
		wxMessageBox(_(ss.str().c_str()));
		return false;
	}

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
		gui_dht* pgdht = gui_dht::instance();
		if (!pgdht) {
			wxMessageBox(_("Impossible to connect to the DHT!"));
			return;
		}
		pgdht->ping(
			std::string(dialog.get_hostname().mb_str()),
			dialog.get_port());
	}
}

void gui_main::OnUpload(wxCommandEvent& evt) {
	// open file for upload
	 wxFileDialog open_file_dialog(
		NULL, 
		_("Open file to be uploaded!"), 
		wxEmptyString, 
		wxEmptyString,
      wxFileSelectorDefaultWildcardStr, 
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (open_file_dialog.ShowModal() == wxID_CANCEL) 
		return;
	if (!gui_dht::instance()) {
		wxMessageBox(
			_("Could not upload \n[") + 
			open_file_dialog.GetPath() + 
			_("]\nto the network, no network."));
		return;
	}
	gui_dht::instance()->start_upload(
		std::string(open_file_dialog.GetPath().fn_str()));
}

void gui_main::OnDownload(wxCommandEvent& evt) {
	// ask for a digest for download
	gui_download dialog;
	while (dialog.ShowModal() == wxID_OK) {
		// Check the digest and start download...
		if (!dialog.validate()) {
			wxMessageBox(_("Invalid digest should be \"[A-Fa-f0-9]{64}\"."));
		} else {
			miniDHT::digest_t digest;
			std::string s = std::string(dialog.get_key().mb_str());
			std::stringstream ss(s);
			ss >> digest;
			gui_dht::instance()->start_download(digest, std::string(download_path_.c_str()));
			return;
		}
	}
}

void gui_main::OnAbout(wxCommandEvent& evt) {
	// create the about dialog
	gui_about dialog;
	dialog.ShowModal();
}
 
void gui_main::OnPrefs(wxCommandEvent& evt) {
	wxMessageBox(_("TODO : Here are preferences."));
}

void gui_main::OnCancel(wxCommandEvent& evt) {
	// cancel some action
	long item = list_ctrl_->get_selected();
	if (item == -1) {
		wxMessageBox(_("ERROR : No item selected!"));
		return;
	}
	if (gui_dht::instance() == NULL) {
		wxMessageBox(_("ERROR : No DHT instance!"));
		return;
	}
	std::list<gui_action*> ls = gui_dht::instance()->get_action_list();
	std::list<gui_action*>::iterator ite = ls.begin();
	for (int i = 0; ite != ls.end(); ++ite, ++i)
		if (i == item)
			gui_dht::instance()->stop_action(
				dynamic_cast<gui_action*>(*ite));
}

void gui_main::OnInfo(wxCommandEvent& evt) {
	long item = list_ctrl_->get_selected();
	if (item == -1) {
		wxMessageBox(_("ERROR : No item selected!"));
		return;
	}
	gui_info dialog(item);
	dialog.ShowModal();
}

void gui_main::OnNetworkStatus(wxCommandEvent& evt) {
	gui_network_status dialog;
	dialog.ShowModal();
}

void gui_main::OnQuit(wxCommandEvent& evt) {
	gui_dht::instance(std::string(temp_path_.mb_str()))->release();
	exit(0);
}

#ifdef __WXMAC__
void gui_main::MacOpenFiles(const wxArrayString& filenames) {
    OnDropFiles(filenames);
}

void gui_main::MacOpenURL (const wxString& url) {
    wxMessageBox(_("MacOpenURL(") + url + _(")"));
}
#endif // __WXMAC__

bool gui_main::OnDropFiles(const wxArrayString& filenames) {
    int nbFilesAccepted = 0;
    int nbFiles = filenames.GetCount();
    for (int i = 0; i < nbFiles; i++) {
        if(wxDirExists(wxString(filenames[i].c_str())))
            continue;
        gui_dht::instance()->start_upload(std::string(filenames[i].c_str()));
        nbFilesAccepted++;
    }
    
    return (nbFilesAccepted==nbFiles); // can return false to 'fail' the drop
}

void gui_main::OnTimer(wxTimerEvent& evt) {
	{
		static wxString moving_string[] = {
			_T(".oOo.| %s |.oOo."),
			_T("oOo..| %s |..oOo"),
			_T("Oo..o| %s |o..oO"),
			_T("o..oO| %s |Oo..o"),
			_T("..oOo| %s |oOo..")
		};
		static unsigned int count = 0;
		frame_->SetTitle(
			wxString::Format(
				moving_string[count % 5], 
				title_));
		count++;
	}
	if (gui_dht::instance()) {
		std::list<gui_action*> ls = gui_dht::instance()->get_action_list();
		std::list<gui_action*>::iterator ite = ls.begin();
		while (list_ctrl_->GetItemCount() > ls.size())
			list_ctrl_->DeleteItem(0);
		while (list_ctrl_->GetItemCount() < ls.size())
			list_ctrl_->InsertItem(list_ctrl_->GetItemCount(), _T(".oOo."));
		for (int i = 0; ite != ls.end(); ++ite, ++i) {
			gui_action* p = dynamic_cast<gui_action*>(*ite);
			{	// progress
				wxString data;
				if (p->is_end()) {
					data = _("Finished");
				} else {
					double percent = 
						(double)p->get_packet_loaded() / 
						(double)p->get_packet_total();
					data = wxString::Format("%3.02f%%", (percent * 100.0));
				}
				if (data != list_ctrl_->GetItemText(i, 0))
					list_ctrl_->SetItem(i, 0, data);	
			}
			{	// digest
				std::stringstream ss("");
				ss << p->get_digest();
				wxString data = _(ss.str().c_str());
				if (data != list_ctrl_->GetItemText(i, 1))
					list_ctrl_->SetItem(i, 1, data);	
			}
			{ 	// bytes
				wxString data = wxString::Format("%lluB", p->get_file_size());
				if (data != list_ctrl_->GetItemText(i, 2))
					list_ctrl_->SetItem(i, 2, data);	
			}
			{	// file
				wxString data = _(p->get_filename().c_str());
				if (data != list_ctrl_->GetItemText(i, 3))
					list_ctrl_->SetItem(i, 3, data);	
			}
			{	// type (colour)
				const wxColour back_download = wxColour(0xaf, 0xff, 0xaf);
				const wxColour front_download = wxColour(0x00, 0x7f, 0x00);
				if (p->get_action_type() == GUI_ACTION_DOWNLOAD) {
					if (list_ctrl_->GetItemBackgroundColour(i) != back_download)
						list_ctrl_->SetItemBackgroundColour(i, back_download);
					if (list_ctrl_->GetItemTextColour(i) != front_download)
						list_ctrl_->SetItemTextColour(i, front_download);
				} 
				const wxColour back_upload = wxColour(0xaf, 0xaf, 0xff);
				const wxColour front_upload = wxColour(0x00, 0x00, 0x7f);
				if (p->get_action_type() == GUI_ACTION_UPLOAD) {
					if (list_ctrl_->GetItemBackgroundColour(i) != back_upload)
						list_ctrl_->SetItemBackgroundColour(i, back_upload);
					if (list_ctrl_->GetItemTextColour(i) != front_upload)
						list_ctrl_->SetItemTextColour(i, front_upload);
				}
			}
		}
	}
}


