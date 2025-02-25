#pragma once
#include <wx/defs.h>          // for wxCLANG_IF_VALID_WARNING_1, wxWindowID
#include <wx/event.h>         // for wxDECLARE_EVENT_TABLE, wxKeyEvent (ptr ...
#include <wx/textctrl.h>      // for wxTextCtrl
#include <wx/string.h>        // for wxString
class wxWindow;

struct NumberTextCtrl : public wxTextCtrl
{
  NumberTextCtrl(wxWindow *parent, wxWindowID id, const wxString &value=wxEmptyString);
private:
  void OnChar(wxKeyEvent&);
  void OnMouseRightDwn(wxMouseEvent&);
  wxDECLARE_EVENT_TABLE();
};
