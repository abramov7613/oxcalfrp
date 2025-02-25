#include "main.h"
#include <bits/utility.h>                                    // for _Nth_typ...
#include <stddef.h>                                          // for NULL
#include <wx/datetime.h>                                     // for wxDateTime
#include <wx/image.h>                                        // for wxImage
#include <wx/imagjpeg.h>                                     // for wxJPEGHa...
#include <wx/imagpng.h>                                      // for wxPNGHan...
#include <wx/language.h>                                     // for wxLANGUA...
#include <wx/string.h>                                       // for wxString
#include <wx/tbarbase.h>                                     // for wxDefaul...
#include <wx/app.h>                                          // for wxAppCon...
#include <wx/zipstrm.h>                                      // for wxZipInp...
#include <wx/txtstrm.h>                                      // for wxTextInp...
#include <wx/mstream.h>                                      // for wxMemory...
#include <wx/fs_mem.h>                                       // for wxMemoryFSH...
#include <wx/filename.h>                                     // for wxFileName
#include <algorithm>                                         // for max, tra...
#include <array>                                             // for array
#include <cstdint>                                           // for uint16_t
#include <iterator>                                          // for back_ins...
#include <optional>                                          // for optional
#include <string>                                            // for string
#include <string_view>                                       // for operator...
#include <tuple>                                             // for get, mak...
#include <type_traits>                                       // for remove_r...
#include <utility>                                           // for make_pair
#include <vector>                                            // for vector...
#include <stdexcept>                                         // for runtime_er...
#include "fs_mem_rc.h"                                       // for wxMemory...
#include "mainframe.h"                                       // for MainFrame
#include "calendarpanel.h"                                   // for CalendarPanel
#include "resource_holder.hpp"                               // for ResourceHolder
#include "sqlite_modern_cpp.h"                               // for database

MyApp::MyApp()
  : wxApp(),
    db_(std::make_shared<sqlite::database>(":memory:")),
    resource_holder_(std::make_shared<ResourceHolder>()),
    calendar_(std::make_shared<oxc::OrthodoxCalendar>())
{
  auto d = wxDateTime::Now();
  date_ = std::make_shared<oxc::Date>(std::to_string(d.GetYear()),
                                      static_cast<char>(d.GetMonth())+1,
                                      d.GetDay(),
                                      oxc::Grigorian);
}

oxc::CalendarFormat MyApp::calendar_type() const
{
  auto* ptr = static_cast<MainFrame*>(GetMainTopWindow());
  return ptr->cal_ctrl->calendar_type();
}

bool MyApp::OnInit()
{
  sqlite::database& db = *db_;
  oxc::Date* date = date_.get();
  wxImage::AddHandler(new wxPNGHandler);
  wxImage::AddHandler(new wxJPEGHandler);
  wxFileSystem::AddHandler(new wxMemoryFSRCHandler);
  wxFileSystem::AddHandler(new wxMemoryFSHandler);
  // load embedded exe resources
  for(auto file_list = resource_holder_->ListFiles(); const auto& f: file_list) {
    auto byte_arr = resource_holder_->Gather(f);
    if( !byte_arr.empty() ) {
      wxFileName p (wxString::FromUTF8(f));
      if(p.GetExt() == "zip") {
        wxMemoryInputStream is (byte_arr.data(), byte_arr.size());
        wxZipInputStream zip (is);
        std::unique_ptr<wxZipEntry> entry ;
        while (entry.reset(zip.GetNextEntry()), entry.get() != nullptr) {
          if(!entry->IsDir()) {
            wxMemoryOutputStream os ;
            zip.Read(os);
            auto* buf = os.GetOutputStreamBuffer();
            wxString name = wxFileName(entry->GetName()).GetFullName();
            wxMemoryFSHandler::AddFile(name, buf->GetBufferStart(), buf->GetBufferSize());
          }
        }
      } else {
        wxMemoryFSRCHandler::AddFile(p.GetFullName(), byte_arr.data(), byte_arr.size());
      }
    }
  }
  // set locale
  locale.Init(wxLANGUAGE_DEFAULT, wxLOCALE_LOAD_DEFAULT);
  // load sqlite database
  wxFileSystem virtual_fs;
  wxFSFile* sqlite_db_file = virtual_fs.OpenFile("memory:sqlite_db.sql");
  if(!sqlite_db_file)
    throw std::runtime_error("ошибка загрузки файла базы данных.");
  auto* is (sqlite_db_file->GetStream());
  wxTextInputStream txt(*is);
  while(!is->Eof()) {
    auto s = txt.ReadLine();
    if(!s.IsEmpty()) { db << s.utf8_string(); }
  }
  // Create the main frame window
  MainFrame* frame = new MainFrame( NULL, -1, wxString::FromUTF8("Церковный календарь диц"),
                                      wxDefaultPosition, wxDefaultSize );
  // show main window
  frame->Show(true);
  return true;
}

wxIMPLEMENT_APP(MyApp);
