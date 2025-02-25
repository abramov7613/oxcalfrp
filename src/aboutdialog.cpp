#include "aboutdialog.h"
#include <wx/panel.h>           // for wxPanel
#include <wx/notebook.h>        // for wxNotebook
#include <wx/html/htmlwin.h>    // for wxHtmlWindow, wxHW_SCROLLBAR_NEVER
#include <wx/sizer.h>           // for wxBoxSizer, wxSizerFlags, wxStdDialog...
#include "timestamp.h"          // for build_time_str...

bool AboutDialog::Create(wxWindow* parent, wxWindowID id, const wxString& title,
    const wxPoint& pos, const wxSize& size, long style, const wxString &name)
{
  static const char* about_body {"<html><body>"
      "<p align=center><img src=\"memory:ICXC_NIKA.png\">"
      "<p align=center><font size=+1><b>Православный "
      "Церковный Календарь</font></b><br><small>для "
      "исследовательских целей %s</small><br>"
      "<small>сборка %s</small><br>"
      "<small>commit id: %s</small></body></html>"
  };
  static const char* license_body {
    "<html><body><p align=center>Copyright (C) 2025  Vladimir Abramov"
    "<p align=justify>This program is free software: you can redistribute it and/or modify "
    "it under the terms of the GNU General Public License as published by "
    "the Free Software Foundation, either version 3 of the License, or "
    "(at your option) any later version."
    "<p align=justify>This program is distributed in the hope that it will be useful, "
    "but WITHOUT ANY WARRANTY; without even the implied warranty of "
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
    "GNU General Public License for more details."
    "<p align=justify>You should have received a copy of the GNU General Public License "
    "along with this program. If not, see https://www.gnu.org/licenses/</body></html>"
  };
  static const char* thanx_body {
    "<html><body><p><table><tr><td><u><font color=blue>abramov7613@yandex.ru</font></u></td>"
    "<td>Владимир Абрамов, автор проекта</td></tr>"
    "<tr><td><u><font color=blue>wxwidgets.org</font></u></td><td>Julian Smart</td></tr>"
    "<tr><td></td><td>Vadim Zeitlin</td></tr>"
    "<tr><td></td><td>Stefan Csomor</td></tr>"
    "<tr><td></td><td>Robert Roebling</td></tr>"
    "<tr><td></td><td>and other members of the wxWidgets project</td></tr>"
    "<tr><td><u><font color=blue>sqlite.org</font></u></td><td>D. Richard Hipp</td></tr>"
    "</table></body></html>"
  };
  if (!wxDialog::Create(parent, id, title, pos, size, style, name)) return false;
  // controls
  notebook = new wxNotebook(this, wxID_ANY);
  auto* about = new wxPanel(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  auto* license = new wxPanel(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  auto* thanx = new wxPanel(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  notebook->AddPage(about, wxString::FromUTF8("О Программе"));
  notebook->AddPage(thanx, wxString::FromUTF8("Благодарности"));
  notebook->AddPage(license, wxString::FromUTF8("Лицензия"));
  about_html = new wxHtmlWindow(about, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_NEVER);
  license_html = new wxHtmlWindow(license);
  thanx_html = new wxHtmlWindow(thanx);
  about_html->SetPage( wxString::Format(wxString::FromUTF8(about_body), git_tag_str, build_time_str, git_hash_str) );
  about_html->SetHTMLBackgroundColour(GetBackgroundColour());
  license_html->SetPage(wxString::FromUTF8(license_body));
  thanx_html->SetPage(wxString::FromUTF8(thanx_body));
  thanx_html->SetHTMLBackgroundColour(GetBackgroundColour());
  // sizers
  SetMinSize(wxSize(450, 520));
  auto* dlg_sizer = new wxBoxSizer(wxVERTICAL);
  dlg_sizer->Add(notebook, wxSizerFlags(1).Expand().Border(wxALL));
  auto* tab1sizer = new wxBoxSizer(wxVERTICAL);
  tab1sizer->Add(about_html, wxSizerFlags(1).Expand().Border(wxALL));
  auto* tab2sizer = new wxBoxSizer(wxVERTICAL);
  tab2sizer->Add(license_html, wxSizerFlags(1).Expand().Border(wxALL));
  auto* tab3sizer = new wxBoxSizer(wxVERTICAL);
  tab3sizer->Add(thanx_html, wxSizerFlags(1).Expand().Border(wxALL));
  dlg_sizer->Add(CreateStdDialogButtonSizer(wxCLOSE), wxSizerFlags().Right().Border(wxALL));
  about->SetSizer(tab1sizer);
  license->SetSizer(tab2sizer);
  thanx->SetSizer(tab3sizer);
  SetSizer(dlg_sizer);
  Fit();
  Centre(wxBOTH);
  return true;
}
