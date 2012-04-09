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
#include "gui_dht.h"
#include "gui_settings.h"

using namespace miniDHT;

#ifdef __WXMAC__
    #include <Carbon/Carbon.h>
#endif

#ifdef __WXGTK__
#include <unistd.h>
#endif

DEFINE_EVENT_TYPE(wxID_LIST_CTRL)

BEGIN_EVENT_TABLE(gui_list_ctrl, wxListCtrl)
	EVT_LIST_ITEM_SELECTED(wxID_LIST_CTRL, gui_list_ctrl::OnSelected)
	EVT_LIST_ITEM_DESELECTED(wxID_LIST_CTRL, gui_list_ctrl::OnDeselected)
    EVT_LIST_ITEM_ACTIVATED(wxID_LIST_CTRL, gui_list_ctrl::OnActivated)
//    EVT_DROP_FILES(gui_list_ctrl::OnDropFiles)
    EVT_LIST_BEGIN_DRAG(wxID_LIST_CTRL, gui_list_ctrl::OnDrag)
END_EVENT_TABLE()

//void gui_list_ctrl::OnDropFiles(wxDropFilesEvent& event) {
//    wxString* files = event.GetFiles();
//    for (int i = 0, n = event.GetNumberOfFiles(); i < n; i++) {
//        wxMessageBox(_("LIST Drop file (") + files[i] + _(")"));
//    }
//}

void gui_list_ctrl::OnDrag(wxListEvent &event) {
    std::list<gui_action*> ls = gui_dht::instance()->get_action_list();
    std::list<gui_action*>::iterator ite = ls.begin();
    
    gui_action* p = NULL;
    
    for (int i = 0; ite != ls.end(); ++ite, ++i) {
        if (i == event.m_itemIndex) {
            p = dynamic_cast<gui_action*>(*ite);
            break;
        }
    }
    
    if(!p)
        return;
    
    std::stringstream ss("");
    ss << p->get_digest();
    wxTextDataObject dragData(wxString(ss.str().c_str()));
    
    wxDropSource dragSource(this);
	dragSource.SetData(dragData);
	wxDragResult result = dragSource.DoDragDrop(TRUE);
}

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

void gui_list_ctrl::OnActivated(wxListEvent& event) {
    wxListItem item = event.GetItem();
    
    std::list<gui_action*> ls = gui_dht::instance()->get_action_list();
    std::list<gui_action*>::iterator ite = ls.begin();
    
    gui_action* p = NULL;
    
    for (int i = 0; ite != ls.end(); ++ite, ++i) {
        if (i == event.m_itemIndex) {
            p = dynamic_cast<gui_action*>(*ite);
            break;
        }
    }
    
    if(!p)
        return;
    
    
    gui_settings* settings = gui_settings::instance();
    std::string absolutPath = settings->find(gui_settings::DOWNLOAD_PATH);
    absolutPath.append(p->get_filename());
    
#ifdef __WXMAC__
    CFStringRef filename = CFStringCreateWithCString (
                                           kCFAllocatorDefault,
                                           absolutPath.c_str(),
                                           CFStringGetSystemEncoding()
                                           );
    
    CFURLRef urlFile = CFURLCreateWithString (kCFAllocatorDefault,
                                    filename,
                                    NULL
                                    );
    FSRef fileRef;
    CFURLGetFSRef(urlFile, &fileRef);
    FSRef fileApp;
    
    OSStatus err = LSOpenFSRef(&fileRef, &fileApp);
    if(err != noErr) {
        char msg[128];
        sprintf(msg, "open URL returned error : %ld\n%s", (long)err, CFStringGetCStringPtr(CFURLCopyPath(urlFile), CFStringGetSystemEncoding()));
        wxMessageBox(_(msg));
        return;
    }
    CFURLRef urlApp = CFURLCreateFromFSRef(kCFAllocatorDefault, &fileApp);
#elif __WXGTK__
    
	// Code only executed by child process
        char cmd[256];
	sprintf(cmd, "/usr/bin/xdg-open %s", absolutPath.c_str());
	system(cmd);
	//execl("/usr/bin/xdg-open", "/usr/bin/xdg-open", absolutPath.c_str());
    
#endif
}


void gui_list_ctrl::OnDeselected(wxListEvent& event) {
	selected_ = -1;
}

