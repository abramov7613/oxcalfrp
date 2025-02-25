#include "monthchoicectrl.h"
#include <wx/arrstr.h>

MonthChoiceCtrl::MonthChoiceCtrl(wxWindow *parent)
: wxChoice(parent,
		wxID_ANY,
		wxDefaultPosition,
		wxDefaultSize,
		wxArrayString {
			wxString::FromUTF8("январь"),
			wxString::FromUTF8("февраль"),
			wxString::FromUTF8("март"),
			wxString::FromUTF8("апрель"),
			wxString::FromUTF8("май"),
			wxString::FromUTF8("июнь"),
			wxString::FromUTF8("июль"),
			wxString::FromUTF8("август"),
			wxString::FromUTF8("сентябрь"),
			wxString::FromUTF8("октябрь"),
			wxString::FromUTF8("ноябрь"),
			wxString::FromUTF8("декабрь") },
		0,
		wxDefaultValidator,
		wxChoiceNameStr)
{
}
