#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/stattext.h>
#include "yearinputdialog.h"
#include "numbertextctrl.h"

YearInputDialog::YearInputDialog(wxWindow* parent, const wxString& year, const oxc::CalendarFormat cfmt,
      wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString &name)
  : wxDialog(parent, id, title, pos, size, style, name), fmt_(cfmt), year_(year)
{
  // create controls
  auto* ok_btn = new wxButton(this, wxID_ANY, wxString::FromUTF8("Ok"));
  auto* no_btn = new wxButton(this, wxID_ANY, wxString::FromUTF8("Отмена"));
  auto* m_static_text = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("тип календаря"));
  auto* m_static_text2 = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("число года"));
  m_text_ctrl = new NumberTextCtrl(this, wxID_ANY, year);
  m_choice = new wxChoice(this, wxID_ANY);
  m_choice->Append(wxString::FromUTF8("юлианский"));
  m_choice->Append(wxString::FromUTF8("ново-юлианский"));
  m_choice->Append(wxString::FromUTF8("григорианский"));
  m_choice->SetSelection(static_cast<int>(cfmt));
  ok_btn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){ PushOkBtn(); });
  no_btn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){ EndModal(wxID_CANCEL); });
  Bind(wxEVT_CHAR_HOOK, [this](wxKeyEvent& event){
    switch ( event.GetKeyCode() ) {
      case WXK_ESCAPE:
        EndModal(wxID_CANCEL);
        break;
      case WXK_RETURN:
        PushOkBtn();
        break;
      default: {}
    }
    event.Skip();
  });
  // create sizers
  auto* grid_sizer = new wxGridSizer(2, 1, 0, 0);
  auto* box_sizer = new wxBoxSizer(wxVERTICAL);
  auto* box_sizer2 = new wxBoxSizer(wxHORIZONTAL);
  auto* box_sizer3 = new wxBoxSizer(wxHORIZONTAL);
  auto* box_sizer4 = new wxBoxSizer(wxHORIZONTAL);
  box_sizer3->Add(m_static_text2, wxSizerFlags().Center().Border(wxALL));
  box_sizer3->Add(m_text_ctrl, wxSizerFlags(1).Expand().Border(wxALL));
  box_sizer->Add(box_sizer3, wxSizerFlags().Expand().Border(wxALL));
  box_sizer4->Add(m_static_text, wxSizerFlags().Center().Border(wxALL));
  box_sizer4->Add(m_choice, wxSizerFlags(1).Expand().Border(wxALL));
  box_sizer->Add(box_sizer4, wxSizerFlags().Expand().Border(wxALL));
  grid_sizer->Add(box_sizer, wxSizerFlags().Expand().Border(wxALL));
  box_sizer2->Add(ok_btn, wxSizerFlags().Border(wxALL));
  box_sizer2->Add(no_btn, wxSizerFlags().Border(wxALL));
  grid_sizer->Add(box_sizer2, wxSizerFlags().Right().Bottom().Border(wxALL));
  SetSizer(grid_sizer);
  Centre(wxBOTH);
}

void YearInputDialog::PushOkBtn()
{
  year_ = m_text_ctrl->GetValue();
  switch(m_choice->GetSelection()){
    case 0:
      fmt_ = oxc::Julian;
      break;
    case 1:
      fmt_ = oxc::Milankovic;
      break;
    case 2:
      fmt_ = oxc::Grigorian;
      break;
    default: {}
  }
  EndModal(wxID_OK);
}
