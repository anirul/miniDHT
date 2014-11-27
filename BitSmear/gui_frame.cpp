/*
 * Copyright (c) 2011, mic
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

#include "gui_frame.h"


BEGIN_EVENT_TABLE(gui_frame, wxFrame)
EVT_CLOSE(gui_frame::onClose)
//EVT_DROP_FILES(gui_frame::OnDropFiles)
END_EVENT_TABLE()




gui_frame::gui_frame(wxWindow* parent, wxWindowID id, 
                 const wxString& title, const wxPoint& pos, 
                 const wxSize& size, long style, 
                 const wxString& name) {

    wxFrame::Create(parent, id,  title, pos, size, style, name);
    DragAcceptFiles(true);
    //SetDropTarget(new gui_drop());

}

//void gui_frame::OnDropFiles(wxDropFilesEvent& event) {
//    wxString* files = event.GetFiles();
//    for (int i = 0, n = event.GetNumberOfFiles(); i < n; i++) {
//        wxMessageBox(_("FRAME Drop file (") + files[i] + _(")"));
//    }
//}

void gui_frame::onClose(wxCloseEvent& evt)
{
    if (evt.CanVeto()) {
        evt.Veto();
        this->Hide();
    }
    else {
        evt.Skip(); // don't stop event, we still want window to close
    }
}
