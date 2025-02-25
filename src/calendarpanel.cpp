#include "calendarpanel.h"
#include <wx/anybutton.h>                                  // for wxBU_EXACTFIT
#include <wx/event.h>                                      // for wxEventTyp...
#include <wx/choice.h>                                     // for wxChoice
#include <wx/checkbox.h>                                   // for wxCheckBox
#include <wx/stattext.h>                                   // for wxStaticText
#include <wx/webview.h>                                    // for wxWebView
#include <wx/msgdlg.h>                                     // for wxMessageBox
#include <wx/fs_mem.h>                                     // for wxMemoryFSHandler
#include <wx/sizer.h>                                      // for wxSizerFlags
#include <wx/stringimpl.h>                                 // for wxEmptyString
#include <exception>                                       // for exception
#include "mainframe.h"                                     // for MainFrame
#include "main.h"                                          // for wxGetApp
#include <wx/window.h>                                     // for wxWindow

static const char* html_table_filename = "month_table.html";

CalendarPanel::SetButton::SetButton(wxWindow * parent, wxWindowID id, const wxString & label,
                                      const wxPoint & pos, const wxSize & size, long style,
                                      const wxValidator & validator, const wxString & name)
  : wxButton(parent, id, label, pos, size, style, validator, name)
{
  SetLabel(wxString::FromUTF8("применить"));
  SetToolTip(wxString::FromUTF8("установить указанную дату"));
  Bind(wxEVT_BUTTON, [this](wxCommandEvent&){ Click(); });
}

void CalendarPanel::SetButton::Click()
{
  auto p = static_cast<CalendarPanel*>(GetParent());
  std::string y = p->year_ctrl->GetValue().utf8_string();
  int8_t m = p->month_ctrl->GetSelection() + 1;
  int8_t d = p->day_ctrl->GetValue();
  if( !oxc::Date::check(y, m, d, p->calendar_mode) ) {
    auto date = wxGetApp().date();
    switch(p->calendar_mode){
      case oxc::Grigorian:
        p->calendar_type_ctrl->SetSelection(1);
        break;
      case oxc::Milankovic:
        p->calendar_type_ctrl->SetSelection(2);
        break;
      default:
        p->calendar_type_ctrl->SetSelection(0);
    }
    p->year_ctrl->ChangeValue( date->year(p->calendar_mode) );
    p->month_ctrl->SetSelection( date->month(p->calendar_mode)-1 );
    p->day_ctrl->SetValue( date->day(p->calendar_mode) );
    wxMessageBox( wxString::FromUTF8("указано неверное число года."),
                  wxString::FromUTF8("ошибка"),
                  wxOK|wxICON_ERROR|wxCENTRE );
  } else {
    p->reset_days_table(oxc::Date(y, m, d, p->calendar_mode));
  }
}

bool CalendarPanel::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
    long style, const wxString& name)
{
  if (!wxPanel::Create(parent, id, pos, size, style, name)) return false;
  // check date
  auto date = wxGetApp().date();
  const auto y = date->year(calendar_mode) ;
  const auto m = date->month(calendar_mode) ;
  const auto d = date->day(calendar_mode) ;
  const auto mon_length = oxc::month_length(m, oxc::is_leap_year(y, calendar_mode));
  // create controls
  calendar_ctrl = wxWebView::New(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(320, 240));
  wxMemoryFSHandler::AddFile(html_table_filename, wxEmptyString);
  calendar_ctrl->Bind(wxEVT_WEBVIEW_TITLE_CHANGED, [this](wxWebViewEvent& evt){
    int d;
    if( evt.GetString().ToInt(&d) ) {
      day_ctrl->SetValue( d );
      set_btn_ctrl->Click();
    }
  });
  calendar_type_ctrl = new wxChoice(this, wxID_ANY);
  calendar_type_ctrl->Append(wxString::FromUTF8("по юлианскому календарю"));
  calendar_type_ctrl->Append(wxString::FromUTF8("по григорианскому календарю"));
  calendar_type_ctrl->Append(wxString::FromUTF8("по ново-юлианскому календарю"));
  switch(calendar_mode){
    case oxc::Grigorian:
      calendar_type_ctrl->SetSelection(1);
      break;
    case oxc::Milankovic:
      calendar_type_ctrl->SetSelection(2);
      break;
    default:
      calendar_type_ctrl->SetSelection(0);
  }
  calendar_type_ctrl->Bind(wxEVT_CHOICE, [this](wxCommandEvent& event)
  {
    switch(event.GetSelection()){
      case 1:
        calendar_mode = oxc::Grigorian;
        break;
      case 2:
        calendar_mode = oxc::Milankovic;
        break;
      default:
        calendar_mode = oxc::Julian;
    }
  });
  day_ctrl = new wxSpinCtrl(this,
                              wxID_ANY,
                              wxEmptyString,
                              wxDefaultPosition,
                              wxDefaultSize,
                              wxSP_ARROW_KEYS,
                              1,
                              mon_length,
                              d );
  day_ctrl->Bind(wxEVT_RIGHT_DOWN, [](wxMouseEvent&){});
  month_ctrl = new MonthChoiceCtrl(this);
  month_ctrl->SetSelection(m - 1);
  month_ctrl->Bind(wxEVT_CHOICE, [this](wxCommandEvent& event)
  {
      int8_t m = event.GetSelection() + 1;
      const std::string y = year_ctrl->GetValue().utf8_string();
      const auto mon_length = oxc::month_length(m, oxc::is_leap_year(y, calendar_mode));
      day_ctrl->SetRange( 1, mon_length );
  });
  year_ctrl = new NumberTextCtrl( this, wxID_ANY, y );
  year_ctrl->SetToolTip( wxString::FromUTF8("значения от 3 до ...") );
  year_ctrl->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent& e)
  {
    if(int8_t m = month_ctrl->GetSelection() + 1; m==2) {
      const std::string y = year_ctrl->GetValue().utf8_string();
      const auto mon_length = oxc::month_length(m, oxc::is_leap_year(y, calendar_mode));
      day_ctrl->SetRange( 1, mon_length );
    }
    e.Skip();
  });
  wxStaticText* ylbl = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("года"), wxDefaultPosition,
      wxDefaultSize, wxALIGN_RIGHT);
  set_btn_ctrl = new SetButton(this);
  sys_btn_ctrl = new wxButton(this, wxID_ANY, wxString::FromUTF8("системное время"));
  sys_btn_ctrl->SetToolTip(wxString::FromUTF8("установить текущую дату"));
  sys_btn_ctrl->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
  {
    auto wxdt = wxDateTime::Now();
    auto date = std::make_unique<oxc::Date>( std::to_string(wxdt.GetYear()),
                                             static_cast<char>(wxdt.GetMonth())+1,
                                             wxdt.GetDay(),
                                             oxc::Grigorian );
    const auto y = date->year(calendar_mode) ;
    const auto m = date->month(calendar_mode) ;
    const auto d = date->day(calendar_mode) ;
    const auto mon_length = oxc::month_length(m, oxc::is_leap_year(y, calendar_mode));
    year_ctrl->ChangeValue(y);
    month_ctrl->SetSelection(m-1);
    day_ctrl->SetRange( 1, mon_length );
    day_ctrl->SetValue(d);
    reset_days_table(*date);
  });
  reset_days_table( *date );
  // create sizers
  auto* box_sizer = new wxBoxSizer(wxHORIZONTAL);
  auto* box_sizer2 = new wxBoxSizer(wxVERTICAL);
  box_sizer2->Add(calendar_ctrl, wxSizerFlags().Expand().Border(wxALL));
  auto* box_sizer5 = new wxBoxSizer(wxHORIZONTAL);
  auto* box_sizer3 = new wxBoxSizer(wxHORIZONTAL);
  box_sizer2->Add(box_sizer5, wxSizerFlags().Expand().Border(wxALL));
  box_sizer2->Add(box_sizer3, wxSizerFlags().Expand().Border(wxALL));
  box_sizer5->Add(calendar_type_ctrl, wxSizerFlags(1).Border(wxALL));
  box_sizer3->Add(day_ctrl, wxSizerFlags().Border(wxALL));
  box_sizer3->Add(month_ctrl, wxSizerFlags(1).Border(wxALL));
  auto* box_sizer4 = new wxBoxSizer(wxHORIZONTAL);
  box_sizer4->Add(year_ctrl, wxSizerFlags(1).Border(wxALL));
  box_sizer4->Add(ylbl, wxSizerFlags().Center().Border(wxALL));
  box_sizer2->Add(box_sizer4, wxSizerFlags().Expand().Border(wxALL));
  auto* grid_sizer = new wxGridSizer(1, 2, 0, 0);
  grid_sizer->Add(set_btn_ctrl, wxSizerFlags(1).Expand().Border(wxALL));
  grid_sizer->Add(sys_btn_ctrl, wxSizerFlags(1).Expand().Border(wxALL));
  box_sizer2->Add(grid_sizer, wxSizerFlags().Expand().Border(wxALL));
  box_sizer->Add(box_sizer2, wxSizerFlags(1).Expand().Border(wxALL));
  SetSizerAndFit(box_sizer);
  return true;
}

std::string CalendarPanel::html_month_table(const oxc::Date& date, const oxc::CalendarFormat cfmt,
      const bool clickevent) const
{
  auto calendar = wxGetApp().calendar();
  auto y = date.year(cfmt);
  auto m = date.month(cfmt);
  auto d = date.day(cfmt);
  int8_t days_count = oxc::month_length(m, oxc::is_leap_year(y, cfmt));
  int8_t first_day_dn = oxc::Date(y, m, 1, cfmt).weekday();
  int8_t c = 0;
  std::string html {"<table class='month_table'><thead><tr>"
                    "<th class='hmonth_cell' style='color: red'>Вс</th>"
                    "<th class='hmonth_cell'>Пн</th><th class='hmonth_cell'>Вт</th>"
                    "<th class='hmonth_cell'>Ср</th><th class='hmonth_cell'>Чт</th>"
                    "<th class='hmonth_cell'>Пт</th><th class='hmonth_cell'>Сб</th>"
                    "</tr></thead><tbody><tr>"};
  //add empty cells
  for(auto i=first_day_dn; i>0; --i) {
    html += "<td class='month_cell'></td>";
    if(++c==7) { c = 0; html += "</tr>"; }
  }
  //add cells with values
  for(int8_t i=1; i<=days_count; ++i) {
    oxc::Date dd(y, m, i, cfmt);
    auto wd = dd.weekday();
    bool red_day = calendar->is_date_of(dd, oxc::dvana10_per_prazd) ||
                    calendar->is_date_of(dd, oxc::dvana10_nep_prazd) ||
                    calendar->is_date_of(dd, oxc::vel_prazd) ||
                    calendar->is_date_of(dd, oxc::pasha) ;
    bool post_day = calendar->is_date_of(dd, oxc::post_vel) ||
                      calendar->is_date_of(dd, oxc::post_petr) ||
                      calendar->is_date_of(dd, oxc::post_usp) ||
                      calendar->is_date_of(dd, oxc::post_rojd) ||
                      calendar->is_date_of(dd, oxc::m8d29) ||
                      calendar->is_date_of(dd, oxc::m9d14) ||
                      calendar->is_date_of(dd, oxc::m1d5) ;
    bool full_day = calendar->is_date_of(dd, oxc::full7_svyatki) ||
                      calendar->is_date_of(dd, oxc::full7_mitar) ||
                      calendar->is_date_of(dd, oxc::full7_sirn) ||
                      calendar->is_date_of(dd, oxc::full7_pasha) ||
                      calendar->is_date_of(dd, oxc::full7_troica) ;
    html += "<td class='month_cell mc_notempty";
    if(red_day) html += " redday";
    if(wd==0) html += " sunday";
    if(post_day || (!full_day && (wd==3 || wd==5))) html += " postday";
    if(full_day) html += " fullday";
    html += '\'';
    if(clickevent) html += " onclick=\"document.title='"+std::to_string(i)+"';\"";
    if(clickevent && i==d) html += " id='current'";
    html += '>'+std::to_string(i)+"</td>";
    if(++c==7) { c = 0; html += "</tr>"; }
  }
  if(!html.ends_with("</tr>")) html += "</tr>";
  html += "</tbody></table>";
  return html;
}

std::string CalendarPanel::html_css() const
{
  return std::string {
    R"---(<style type='text/css'>
    .month_table {
      border-collapse: collapse;
      border-spacing: 0;
      width: 100%;
    }
    .hmonth_cell {
      background-color: silver;
      border-width: 1px;
      border-style: solid;
      border-color: #888;
      padding: 4px;
      text-align: center;
      vertical-align: middle;
    }
    .month_cell {
      border-width: 1px;
      border-style: solid;
      border-color: #888;
      padding: 4px;
      text-align: center;
      vertical-align: middle;
    }
    .mc_notempty {
      cursor: pointer;
    }
    .postday{
      background-color: #ECEAEA;
    }
    .fullday{
      background-color: #FCC7CB;
    }
    .sunday{
      color: red;
    }
    .redday {
      color: white !important;
      background-color: red !important;
    }
    #current {
      border-width: 3px;
      border-style: solid;
      border-color: orange;
    }
    </style>)---"
  };
}

void CalendarPanel::reset_days_table(const oxc::Date& newdate)
{
  auto date = wxGetApp().date();
  auto calendar = wxGetApp().calendar();
  std::string html = "<html><head><meta charset='utf-8'>" + wxString::FromUTF8(html_css())
                        + "</head><body>" + wxString::FromUTF8(html_month_table(newdate, calendar_mode, true))
                        + "</body></html>";
  wxMemoryFSHandler::RemoveFile(html_table_filename);
  wxMemoryFSHandler::AddFile(html_table_filename, html);
  calendar_ctrl->LoadURL(wxString{} << "memory:" << html_table_filename);
  //reset current date
  if(newdate != *date) *date = newdate;
  //print day data
  if(auto* p = static_cast<MainFrame*>(GetParent()); p) p->print_date_info();
}
