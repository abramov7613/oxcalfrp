#include "numbertextctrl.h"
#include <wx/tbarbase.h>      // for wxDefaultPosition, wxDefaultSize
#include <wx/validate.h>      // for wxDefaultValidator, wxValidator (ptr only)
#include <wx/gdicmn.h>        // for wxPoint, wxSize (ptr only)

NumberTextCtrl::NumberTextCtrl(wxWindow *parent, wxWindowID id, const wxString &value)
  : wxTextCtrl(parent, id, value, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, wxTextCtrlNameStr)
{
}

void NumberTextCtrl::OnChar(wxKeyEvent& event)
{
  const int keyCode = event.GetKeyCode();
  if (keyCode >= '0' && keyCode <= '9') event.Skip();
  switch (keyCode) {
    case WXK_BACK :
    case WXK_LEFT :
    case WXK_RIGHT :
    case WXK_TAB :
    case WXK_DELETE :
    case WXK_END :
    case WXK_HOME :
        event.Skip();
        break;
  };
}

void NumberTextCtrl::OnMouseRightDwn(wxMouseEvent&)
{
}

wxBEGIN_EVENT_TABLE(NumberTextCtrl, wxTextCtrl)
  EVT_CHAR(NumberTextCtrl::OnChar)
  EVT_RIGHT_DOWN(NumberTextCtrl::OnMouseRightDwn)
wxEND_EVENT_TABLE()
