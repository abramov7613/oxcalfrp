#pragma once
#include <wx/defs.h>      // for wxWindowID, wxID_ANY
#include <wx/gdicmn.h>    // for wxPoint, wxSize (ptr only)
#include <wx/string.h>    // for wxString
#include <wx/tbarbase.h>  // for wxDefaultPosition, wxDefaultSize
#include <wx/dialog.h>    // for wxDialog
class wxHtmlWindow;
class wxNotebook;
class wxWindow;

class AboutDialog : public wxDialog
{
public:
    AboutDialog() {}
    AboutDialog(wxWindow *parent, wxWindowID id = wxID_ANY, const wxString& title =
        wxString::FromUTF8("О Программе"), const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE,
        const wxString &name = wxDialogNameStr)
    {
        Create(parent, id, title, pos, size, style, name);
    }
    bool Create(wxWindow *parent, wxWindowID id, const wxString& title, const wxPoint& pos,
                  const wxSize& size, long style, const wxString &name);
protected:
  wxHtmlWindow* about_html;
  wxHtmlWindow* license_html;
  wxHtmlWindow* thanx_html;
  wxNotebook* notebook;
};
