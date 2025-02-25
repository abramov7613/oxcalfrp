#pragma once
#include <wx/defs.h>       // for wxCLANG_IF_VALID_WARNING_1, wxWindowID
#include <wx/event.h>      // for wxCommandEvent (ptr only), wxDECLARE_EVENT...
#include <wx/gdicmn.h>     // for wxPoint, wxSize (ptr only)
#include <wx/frame.h>      // for wxFrame
#include <wx/string.h>     // for wxString
#include <memory>          // for shared_ptr
#include <string>          // for string
class AboutDialog;
class wxWebView;
class wxMenuBar;
class wxStatusBar;
class wxWindow;
class CalendarPanel;

class MainFrame : public wxFrame
{
    friend class MyApp;
    wxMenuBar* m_menubar;
    wxStatusBar* m_statusBar;
    CalendarPanel* cal_ctrl;
    wxWebView* html_ctrl;
    AboutDialog* about_dialog;
    wxDECLARE_EVENT_TABLE();

public:
    MainFrame(wxWindow *parent,
                const wxWindowID id,
                const wxString& title,
                const wxPoint& pos,
                const wxSize& size);
    void OnOpen(wxCommandEvent&);
    void OnSave(wxCommandEvent&);
    void OnQuit(wxCommandEvent&);
    void OnAbout(wxCommandEvent&);
    void OnOutYearInfo(wxCommandEvent&);
    void print_to_html_ctrl(const std::string& s, const bool append=false);
    void print_date_info();
};
