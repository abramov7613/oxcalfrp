#include "mainframe.h"
#include <wx/filedlg.h>                                    // for wxFileDialog
#include <wx/frame.h>                                      // for wxFrameBase::sm_eventTable
#include <wx/menu.h>                                       // for wxMenu, wxMenuBar
#include <wx/webview.h>                                    // for wxWebView
#include <wx/msgdlg.h>                                     // for wxMessageBox
#include <wx/sizer.h>                                      // for wxBoxSizer, wxSizerFlags
#include <wx/stringimpl.h>                                 // for wxEmptyString
#include <wx/msgdlg.h>                                     // for wxMessageBox
#include <wx/webviewfshandler.h>                           // for wxWebViewFSHandler
#include <wx/sharedptr.h>                                  // for wxSharedPtr
#include <wx/fs_mem.h>                                     // for wxMemoryFSHandler
#include <wx/txtstrm.h>                                    // for wxTextInputStream
#include <wx/wfstream.h>                                   // for wxFileOutputStream
#include <exception>                                       // for exception
#include <optional>                                        // for optional
#include <algorithm>                                       // for minmax_element
#include <vector>                                          // for vector
#include "numbertextctrl.h"                                // for YearInputDialog
#include "aboutdialog.h"                                   // for AboutDialog
#include "calendarpanel.h"                                 // for CalendarPanel
#include "yearinputdialog.h"                               // for YearInputDialog
#include "main.h"                                          // for wxGetApp, MyApp
#include "oxc.h"                                           // for year_month_day
#include "sqlite_modern_cpp.h"                             // for database

// IDs for the controls and the menu commands
enum
{
  MENU_ID_OPENFILE = 2,
  MENU_ID_SAVEFILE,
  MENU_ID_OUTYEARINFO,
  MENU_ID_EXIT,
  MENU_ID_ABOUT
  //...
};

static const char* html_ctrl_content_filename = "html_ctrl_content.html";

MainFrame::MainFrame(wxWindow *parent, const wxWindowID id, const wxString& title, const wxPoint& pos,
      const wxSize& size) : wxFrame(parent, id, title, pos, size)
{
  // create menu & statusbar
  m_statusBar = CreateStatusBar();
  wxMenu *main_menu = new wxMenu;
  wxMenu *help_menu = new wxMenu;
  main_menu->Append(MENU_ID_OPENFILE, wxString::FromUTF8("Открыть"));
  main_menu->Append(MENU_ID_SAVEFILE, wxString::FromUTF8("Сохранить"));
  main_menu->Append(MENU_ID_OUTYEARINFO, wxString::FromUTF8("Вывести календарь на год"));
  main_menu->Append(MENU_ID_EXIT, wxString::FromUTF8("Выход"));
  help_menu->Append(MENU_ID_ABOUT, wxString::FromUTF8("О Программе"));
  m_menubar = new wxMenuBar;
  m_menubar->Append(main_menu, wxString::FromUTF8("Меню"));
  m_menubar->Append(help_menu, wxString::FromUTF8("Справка"));
  SetMenuBar(m_menubar);
  // create controls
  html_ctrl = wxWebView::New(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(420, 340));
  wxFileSystem::AddHandler(new wxMemoryFSHandler);
  wxMemoryFSHandler::AddFile(wxString(html_ctrl_content_filename), wxEmptyString);
  html_ctrl->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
  html_ctrl->EnableContextMenu(false);
  cal_ctrl = new CalendarPanel(this);
  about_dialog = new AboutDialog(this);
  // create sizers
  wxBoxSizer* box_sizer = new wxBoxSizer(wxHORIZONTAL);
  box_sizer->Add(cal_ctrl, wxSizerFlags().Expand().Border(wxALL));
  box_sizer->Add(html_ctrl, wxSizerFlags(1).Expand().Border(wxALL));
  SetSizerAndFit(box_sizer);
  Centre(wxBOTH);
}

void MainFrame::OnOpen(wxCommandEvent&)
{
  wxFileDialog dialog(this, wxString::FromUTF8("Открыть ..."), wxEmptyString, wxEmptyString,
                        wxString::FromUTF8("html files|*.html|htm files|*.htm"), wxFD_OPEN|wxFD_FILE_MUST_EXIST);
  if(dialog.ShowModal() != wxID_CANCEL) html_ctrl->LoadURL(wxString::FromUTF8("file:") + dialog.GetPath());
}

void MainFrame::OnSave(wxCommandEvent&)
{
  wxFileDialog dialog(this, wxString::FromUTF8("Сохранить ..."), wxEmptyString, wxEmptyString,
                        wxEmptyString, wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
  if(dialog.ShowModal() != wxID_CANCEL) {
    auto path = dialog.GetPath();
    if(!path.EndsWith(".html") && !path.EndsWith(".htm")) path += ".html";
    wxFileOutputStream output_stream(path);
    if (!output_stream.IsOk()) {
      wxMessageBox( wxString::FromUTF8("невозможно сохранить файл ") + path,
            wxString::FromUTF8("ошибка"), wxOK|wxICON_ERROR|wxCENTRE );
    } else {
      wxTextOutputStream file(output_stream);
      file << html_ctrl->GetPageSource() << '\n';
    }
  }
}

void MainFrame::OnQuit(wxCommandEvent&)
{
  Close();
}

void MainFrame::OnAbout(wxCommandEvent&)
{
  about_dialog->Centre();
  about_dialog->ShowModal();
}

void MainFrame::OnOutYearInfo(wxCommandEvent&)
{
  auto calendar = wxGetApp().calendar();
  auto calendar_fmt = cal_ctrl->calendar_type();
  std::string year;
  YearInputDialog dialog(this, cal_ctrl->GetYear(), calendar_fmt);
  if (dialog.ShowModal() == wxID_OK) {
    year = dialog.GetYear().utf8_string();
    calendar_fmt = dialog.calendar_type();
    if( !oxc::Date::check(year, 1, 1, calendar_fmt) ) {
      wxMessageBox( wxString::FromUTF8("указано неверное число года."),
                    wxString::FromUTF8("ошибка"),
                    wxOK|wxICON_ERROR|wxCENTRE );
      return ;
    }
  } else {
    return ;
  }
  auto pascha_date = calendar->pascha(year, calendar_fmt);
  std::string calendar_type_str, date_fmt, range_fmt1, range_fmt2, range_suff;
  switch(calendar_fmt){
    case oxc::Grigorian:
      calendar_type_str = "григорианскому";
      date_fmt = "%Gd %GM %GY г. (%Jd %JM %JY г. по ст. ст.)";
      range_fmt1 = "%Gd %GM %GY г.";
      range_fmt2 = "%Jd %JM %JY г.";
      range_suff = " по ст. ст.";
      break;
    case oxc::Milankovic:
      calendar_type_str = "ново-юлианскому";
      date_fmt = "%Md %MM %MY г. (%Jd %JM %JY г. по ст. ст.)";
      range_fmt1 = "%Md %MM %MY г.";
      range_fmt2 = "%Jd %JM %JY г.";
      range_suff = " по ст. ст.";
      break;
    default:
      calendar_type_str = "юлианскому";
      date_fmt = "%Jd %JM %JY г. (%Md %MM %MY г. по н. ст.)";
      range_fmt1 = "%Jd %JM %JY г.";
      range_fmt2 = "%Md %MM %MY г.";
      range_suff = " по н. ст.";
  }
  auto get_descr_str = [&date_fmt, &year, calendar, calendar_fmt](const std::string& prefix, oxc::oxc_const c){
    std::string res ;
    for(const auto& e: calendar->get_alldates_with(year, c, calendar_fmt))
      res += prefix + calendar->get_description_for_date(e, date_fmt);
    return res;
  };
  auto get_dates_range_str = [&range_fmt1, &range_fmt2, &range_suff, &year, calendar, calendar_fmt](oxc::oxc_const c) {
    std::string result ;
    auto v = calendar->get_alldates_with(year, c, calendar_fmt);
    if(v.empty()) return result;
    auto subrangestr = [&range_fmt1, &range_fmt2, &range_suff](auto begin, auto end)->std::string{
      auto [min, max] = std::minmax_element(begin, end);
      return "с " + min->format(range_fmt1) + " по " + max->format(range_fmt1)
              + " (" + min->format(range_fmt2) + " по " + max->format(range_fmt2) + range_suff + ")";
    };
    for(auto begin = v.begin(); begin != v.end() ;) {
      auto x = std::adjacent_find( begin, v.end(),
                                    [](const auto& lhs, const auto& rhs){ return lhs.inc_by_days() != rhs; } );
      if(x==v.end()) {
        result += subrangestr(begin, x);
        break;
      } else {
        auto i = std::next(x);
        result += subrangestr(begin, i) + "; " ;
        begin = i;
      }
    }
    return result;
  };
  std::string text { "<html><head><meta charset='utf-8'>" + cal_ctrl->html_css() };
  text += "<style type='text/css'>.Yt{text-align:center;vertical-align:top;}</style>"
          "</head><body><p><h2 style='text-align:center;'>" ;
  text += year + " год (по " + calendar_type_str + " календарю)</h2>"
          "<p><table style='border-collapse:separate;border-spacing:1.5em;width:100%;'><tr>";
  text += "<td class='Yt'>" + oxc::Date::month_name(1, false) + "<br>";
  text += cal_ctrl->html_month_table({year, 1, 1, calendar_fmt}, calendar_fmt, false) + "</td>";
  text += "<td class='Yt'>" + oxc::Date::month_name(2, false) + "<br>";
  text += cal_ctrl->html_month_table({year, 2, 1, calendar_fmt}, calendar_fmt, false) + "</td>";
  text += "<td class='Yt'>" + oxc::Date::month_name(3, false) + "<br>";
  text += cal_ctrl->html_month_table({year, 3, 1, calendar_fmt}, calendar_fmt, false) + "</td></tr><tr>";
  text += "<td class='Yt'>" + oxc::Date::month_name(4, false) + "<br>";
  text += cal_ctrl->html_month_table({year, 4, 1, calendar_fmt}, calendar_fmt, false) + "</td>";
  text += "<td class='Yt'>" + oxc::Date::month_name(5, false) + "<br>";
  text += cal_ctrl->html_month_table({year, 5, 1, calendar_fmt}, calendar_fmt, false) + "</td>";
  text += "<td class='Yt'>" + oxc::Date::month_name(6, false) + "<br>";
  text += cal_ctrl->html_month_table({year, 6, 1, calendar_fmt}, calendar_fmt, false) + "</td></tr><tr>";
  text += "<td class='Yt'>" + oxc::Date::month_name(7, false) + "<br>";
  text += cal_ctrl->html_month_table({year, 7, 1, calendar_fmt}, calendar_fmt, false) + "</td>";
  text += "<td class='Yt'>" + oxc::Date::month_name(8, false) + "<br>";
  text += cal_ctrl->html_month_table({year, 8, 1, calendar_fmt}, calendar_fmt, false) + "</td>";
  text += "<td class='Yt'>" + oxc::Date::month_name(9, false) + "<br>";
  text += cal_ctrl->html_month_table({year, 9, 1, calendar_fmt}, calendar_fmt, false) + "</td></tr><tr>";
  text += "<td class='Yt'>" + oxc::Date::month_name(10, false) + "<br>";
  text += cal_ctrl->html_month_table({year, 10, 1, calendar_fmt}, calendar_fmt, false) + "</td>";
  text += "<td class='Yt'>" + oxc::Date::month_name(11, false) + "<br>";
  text += cal_ctrl->html_month_table({year, 11, 1, calendar_fmt}, calendar_fmt, false) + "</td>";
  text += "<td class='Yt'>" + oxc::Date::month_name(12, false) + "<br>";
  text += cal_ctrl->html_month_table({year, 12, 1, calendar_fmt}, calendar_fmt, false) + "</td></tr>"
          "<tr><td colspan='3'><table><tr><td class='month_cell redday'>17</td>"
          "<td> - Двунадесятые или Великие праздники</td></tr><tr>"
          "<td class='month_cell fullday'>17</td>"
          "<td> - Сплошные седмицы</td></tr><tr>"
          "<td class='month_cell postday'>17</td>"
          "<td> - Постные дни</td></tr></table></td><td></td><td></td></tr></table>"
          "<p><h4 style='color:red;text-align:center;'>" ;
  if(pascha_date) {
    text += calendar->get_description_for_date(pascha_date, date_fmt) + "</h4>";
  } else {
    text += "В этом году православная пасха не празднуется!</h4>";
  }
  text += "<p><h4>Двунадесятые переходящие праздники:</h4><ul>" ;
  if(auto s = get_descr_str("<li>", oxc::dvana10_per_prazd); !s.empty()) text += s;
  text += "</ul><p><h4>Двунадесятые непереходящие праздники:</h4><ul>";
  if(auto s = get_descr_str("<li>", oxc::dvana10_nep_prazd); !s.empty()) text += s;
  text += "</ul><p><h4>Великие праздники:</h4><ul>";
  if(auto s = get_descr_str("<li>", oxc::vel_prazd); !s.empty()) text += s;
  text += "</ul><p><h4>Многодневные посты:</h4><ul>";
  if(auto s = get_dates_range_str(oxc::post_vel); !s.empty()) text += "<li>Великий пост - " + s;
  if(auto s = get_dates_range_str(oxc::post_petr); !s.empty()) text += "<li>Петров пост - " + s;
  if(auto s = get_dates_range_str(oxc::post_usp); !s.empty()) text += "<li>Успенский пост - " + s;
  if(auto s = get_dates_range_str(oxc::post_rojd); !s.empty()) text += "<li>Рождественский пост - " + s;
  text += "</ul><p><h4>Однодневные посты:</h4><ul>";
  if(auto d1 = calendar->get_date_with(year, oxc::m1d5, calendar_fmt); d1)
    text += "<li>" + calendar->get_description_for_date(d1, date_fmt);
  if(auto d2 = calendar->get_date_with(year, oxc::m8d29, calendar_fmt); d2)
    text += "<li>" + calendar->get_description_for_date(d2, date_fmt);
  if(auto d3 = calendar->get_date_with(year, oxc::m9d14, calendar_fmt); d3)
    text += "<li>" + calendar->get_description_for_date(d3, date_fmt);
  text += "<li>Среда и пятница в течение всего года, за исключением сплошных седмиц и Святок.";
  text += "</ul><p><h4>Сплошные седмицы:</h4><ul>";
  if(auto s = get_dates_range_str(oxc::full7_svyatki); !s.empty()) text += "<li>Святки - " + s;
  if(auto s = get_dates_range_str(oxc::full7_mitar); !s.empty()) text += "<li>Мытаря и фарисея - " + s;
  if(auto s = get_dates_range_str(oxc::full7_sirn); !s.empty()) text += "<li>Сырная (масленица) - " + s;
  if(auto s = get_dates_range_str(oxc::full7_pasha); !s.empty()) text += "<li>Пасхальная (Светлая) - " + s;
  if(auto s = get_dates_range_str(oxc::full7_troica); !s.empty()) text += "<li>Троицкая - " + s;
  text += "</ul></body></html>" ;
  print_to_html_ctrl(text);
}

void MainFrame::print_to_html_ctrl(const std::string& s, const bool append)
{
  wxString str;
  if(append){
    wxFileSystem virtual_fs;
    wxFSFile* f = virtual_fs.OpenFile(wxString{} << "memory:" << html_ctrl_content_filename);
    wxInputStream* stream = f->GetStream();
    wxTextInputStream text (*stream);
    while(!stream->Eof()) {
      str += wxString(text.ReadLine() + wxT("\n"));
    }
  }
  str += wxString::FromUTF8(s);
  wxMemoryFSHandler::RemoveFile(wxString(html_ctrl_content_filename));
  wxMemoryFSHandler::AddFile(wxString(html_ctrl_content_filename), str);
  html_ctrl->LoadURL(wxString{} << "memory:" << html_ctrl_content_filename);
}

void MainFrame::print_date_info()
{
  static const char* tt =
    R"---(<html><head><meta charset='utf-8'>
    <style type='text/css'>
    table {
      border-collapse: collapse;
      border-spacing: 0;
      width: 100%;
    }
    th {
      background-color: silver;
      border-width: 1px;
      border-style: solid;
      border-color: #888;
      padding: 4px;
      text-align: center;
      vertical-align: middle;
    }
    td {
      border-width: 1px;
      border-style: solid;
      border-color: #888;
      padding: 4px;
      text-align: center;
      vertical-align: middle;
    }
    </style>
    </head><body><p>
    <table><thead><tr>
      <th>тип календаря</th>
      <th>число</th>
      <th>месяц</th>
      <th>год</th>
    </tr></thead><tbody><tr>
      <td>григорианский</td>
      <td>%Gd</td>
      <td>%GF</td>
      <td>%GY</td>
    </tr><tr>
      <td>ново-юлианский</td>
      <td>%Md</td>
      <td>%MF</td>
      <td>%MY</td>
    </tr><tr>
      <td>юлианский</td>
      <td>%Jd</td>
      <td>%JF</td>
      <td>%JY</td>
    </tr></tbody></table><p>
    )---";
  std::string buf;
  std::vector<std::string> names_list;
  auto date = wxGetApp().date();
  auto calendar = wxGetApp().calendar();
  sqlite::database& db = *wxGetApp().database();
  std::string text = date->format(tt);
  int n50 = calendar->date_n50(*date);
  int glas = calendar->date_glas(*date);
  if(auto s = calendar->get_description_for_date(*date, ""); !s.empty()) buf += s + ' ';
  if(auto wd = date->format("%WD"); buf.find(wd)==buf.npos) {
    buf += wd + ' ';
    if(n50>0) buf += std::to_string(n50) + " по пятидесятнице. ";
  }
  if(glas>0) buf += "глас " + std::to_string(glas) + ". ";
  text += buf + "<hr>";
  db << "SELECT name FROM names JOIN dates ON names.name_id=dates.name_id WHERE month=? AND day=?;"
  << date->month(oxc::Julian) << date->day(oxc::Julian) >> [&names_list](std::string s)
  {
    while(true){
      auto i = std::find(s.begin(), s.end(), '\'');
      if(i==s.end()) break;
      else s.erase(i);//TODO: сделать расстановky ударений
    }
    names_list.push_back(std::move(s));
  };
  for(auto i: calendar->date_properties(*date)) if(i>5000) names_list.push_back(oxc::property_title(i));
  std::sort(names_list.begin(), names_list.end());
  for(const auto& s: names_list) text += s + "<br>" ;
  text += "</body></html>" ;
  print_to_html_ctrl(text);
}

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(MENU_ID_OPENFILE, MainFrame::OnOpen)
    EVT_MENU(MENU_ID_SAVEFILE, MainFrame::OnSave)
    EVT_MENU(MENU_ID_EXIT, MainFrame::OnQuit)
    EVT_MENU(MENU_ID_ABOUT, MainFrame::OnAbout)
    EVT_MENU(MENU_ID_OUTYEARINFO, MainFrame::OnOutYearInfo)
wxEND_EVENT_TABLE()
