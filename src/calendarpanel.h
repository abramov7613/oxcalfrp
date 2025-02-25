#pragma once
#include <cstdint>              // for int8_t
#include <wx/defs.h>            // for wxID_ANY, wxTAB_TRAVERSAL, wxWindowID
#include <wx/gdicmn.h>          // for wxPoint, wxSize (ptr only)
#include <wx/panel.h>           // for wxPanel
#include <wx/button.h>          // for wxButton
#include <wx/string.h>          // for wxString
#include <wx/tbarbase.h>        // for wxDefaultPosition, wxDefaultSize
#include <wx/spinctrl.h>        // for wxSpinCtrl
#include <string>               // for string
#include "numbertextctrl.h"     // for NumberText...
#include "monthchoicectrl.h"    // for MonthChoic...
#include "oxc.h"                // for CalendarFor...
class wxCheckBox;
class wxWindow;
class wxChoice;
class wxWebView;

class CalendarPanel final : public wxPanel
{
  class SetButton final : public wxButton
  {
  public:
    SetButton(wxWindow * parent, wxWindowID id = wxID_ANY, const wxString & label = wxEmptyString,
                  const wxPoint & pos = wxDefaultPosition, const wxSize & size = wxDefaultSize,
                  long style = 0, const wxValidator & validator = wxDefaultValidator,
                  const wxString & name = wxButtonNameStr);
    void Click();
  };
  friend class SetButton;
  oxc::CalendarFormat calendar_mode;
  SetButton* set_btn_ctrl;
  wxButton* sys_btn_ctrl;
  MonthChoiceCtrl* month_ctrl;
  wxChoice* calendar_type_ctrl;
  wxSpinCtrl* day_ctrl;
  NumberTextCtrl* year_ctrl;
  wxWebView* calendar_ctrl;

public:
  CalendarPanel() : calendar_mode(oxc::Julian) {}
  CalendarPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString &name = wxPanelNameStr)
   : calendar_mode(oxc::Julian)
  {
      Create(parent, id, pos, size, style, name);
  }
  bool Create(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
      const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString &name = wxPanelNameStr);
  std::string html_month_table(const oxc::Date& date, const oxc::CalendarFormat cfmt, const bool clickevent) const;
  std::string html_css() const;
  wxString GetYear() const { return year_ctrl->GetValue(); }
  char GetMonth() const { return month_ctrl->GetSelection() + 1; }
  char GetDay() const { return day_ctrl->GetValue(); }
  auto calendar_type() const { return calendar_mode; }

private:
  void reset_days_table(const oxc::Date& newdate);
};
