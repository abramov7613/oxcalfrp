#pragma once
#include <wx/dialog.h>
#include <wx/gdicmn.h>
#include <wx/string.h>
#include "oxc.h"

class NumberTextCtrl;
class wxChoice;

class YearInputDialog : public wxDialog
{
  oxc::CalendarFormat fmt_ ;
  wxString year_ ;
  NumberTextCtrl* m_text_ctrl ;
  wxChoice* m_choice ;
  void PushOkBtn() ;
public:
  YearInputDialog(wxWindow *parent, const wxString& year, const oxc::CalendarFormat cfmt, wxWindowID id = wxID_ANY,
        const wxString& title = wxString::FromUTF8("Введите"), const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxSize(460, 300), long style = wxDEFAULT_DIALOG_STYLE|wxWANTS_CHARS,
        const wxString &name = wxDialogNameStr);

  auto calendar_type() const { return fmt_; }
  auto GetYear() const { return year_; }
};
