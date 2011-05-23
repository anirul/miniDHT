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
#ifdef __WXMAC__
#include <ApplicationServices/ApplicationServices.h>
#endif // __WXMAC__
#include "gui.h" 
 
IMPLEMENT_APP(MyApp)
 
BEGIN_EVENT_TABLE(MyApp, wxApp)
	EVT_MENU(wxID_ABOUT, MyApp::OnAbout)
	EVT_MENU(wxID_PREFERENCES, MyApp::OnPrefs)
	EVT_MENU(wxID_EXIT, MyApp::OnQuit)
END_EVENT_TABLE()
 
bool MyApp::OnInit() {
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	frame = new wxFrame(
		(wxFrame *)NULL, 
		-1,  
		wxT(".oO BitSmear Oo."), 
		wxPoint(50,50), 
		wxSize(800,600));

#ifdef __WXMAC__
	wxApp::SetExitOnFrameDelete(false);
#endif // __WXMAC__

//	wxMenuBar::MacSetCommonMenuBar(menubar);
	wxMenuBar* menubar = new wxMenuBar();
	wxMenu* main_menu = new  wxMenu();

	main_menu->Append(wxID_ABOUT, _("About"));
	main_menu->Append(wxID_HELP, _("Help"));
	main_menu->Append(wxID_PREFERENCES, _("Preferences"));
	main_menu->Append(wxID_EXIT, _("Exit")); 

	main_menu->Append(wxID_ANY, _("Upload..."));
	main_menu->Append(wxID_ANY, _("Download..."));

	menubar->Append(main_menu, _("File"));

	frame->SetMenuBar(menubar);
	frame->Show();

#ifdef __WXMAC__
	ProcessSerialNumber PSN;
	GetCurrentProcess(&PSN);
	TransformProcessType(&PSN, kProcessTransformToForegroundApplication);
#endif // __WXMAC__

	return true;
} 

void MyApp::OnUpload(wxCommandEvent& evt) {
	// open file for upload
}

void MyApp::OnDownload(wxCommandEvent& evt) {
	// ask for a digest to be downloaded
	// if success start the download?
}

void MyApp::OnAbout(wxCommandEvent& evt) {
	wxMessageBox(_("About this application... not much to say for such a simple example, really"));
}
 
void MyApp::OnPrefs(wxCommandEvent& evt) {
	wxMessageBox(_("Here are preferences. Not much to configure in this simple example =)"));
}

void MyApp::OnQuit(wxCommandEvent& evt) {
	// should do some cleanup
	exit(0);
}

void MyApp::MacOpenFile(const wxString& filename) {
	wxMessageBox(_("MacOpenFile(") + filename + _(")"));
}

