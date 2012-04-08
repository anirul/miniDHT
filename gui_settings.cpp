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

#include <gui_settings.h>
#include <wx/wx.h>
#include <wx/stdpaths.h>

gui_settings::gui_settings() {
    std::string ressources_path;
    std::string temp_path;
    std::string download_path;
#ifdef __WXMAC__
    ressources_path = wxStandardPathsCF::Get().GetResourcesDir() + _T("/");
    temp_path = wxStandardPathsCF::Get().GetUserDataDir() + _T("/");
    download_path = wxStandardPathsCF::Get().GetDocumentsDir() + _T("/");
    if(!wxDirExists(temp_path)) {
        if(!wxMkdir(temp_path, 0755)) {
            std::cerr 
            << "WARNING : failed to create directory " 
            << temp_path << std::endl;
        } else {
            std::cout 
            << "INFO : created directory " 
            << temp_path << std::endl;
        }
    }
#else
    ressources_path = wxStandardPaths::Get().GetResourcesDir() + _T("/");
    temp_path = wxStandardPaths::Get().GetTempDir() + _T("/");
    download_path = wxStandardPaths::Get().GetDocumentsDir() + _T("/");
#endif // __WXMAC__

    settings_map_.insert(std::make_pair(DOWNLOAD_PATH, download_path));
    settings_map_.insert(std::make_pair(TEMPFILE_PATH, temp_path));
    settings_map_.insert(std::make_pair(RESSOURCES_PATH, ressources_path));
}
    
    
gui_settings::gui_settings(std::string pref_file) {
    // FIXME !
}

gui_settings* gui_settings::getInstance() {
    static gui_settings* instance_ = NULL;
    if(!instance_)
        instance_ = new gui_settings();
    
    return instance_;
}

std::string gui_settings::getSetting(gui_settings_type setting) {
    std::string value("");
    std::map<gui_settings_type, std::string>::iterator it;
    it = settings_map_.find(setting);
    if(it != settings_map_.end()) {
        value = it->second;
    }
    return value;
}
