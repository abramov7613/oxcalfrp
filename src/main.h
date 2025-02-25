#pragma once
#include <wx/app.h>              // for wxApp
#include <wx/intl.h>             // for wxLocale
#include <wx/filesys.h>          // for wxFileSy...
#include <memory>                // for shared_ptr
#include "oxc.h"                 // for Date...

namespace sqlite { class database; }
class ResourceHolder;

class MyApp : public wxApp
{
  std::shared_ptr<sqlite::database> db_;
  std::shared_ptr<ResourceHolder> resource_holder_;
  std::shared_ptr<oxc::OrthodoxCalendar> calendar_;
  std::shared_ptr<oxc::Date> date_;
  wxLocale locale;
public:
  MyApp();
  std::shared_ptr<sqlite::database> database() const { return db_; }
  std::shared_ptr<ResourceHolder> resource_holder() const { return resource_holder_; }
  std::shared_ptr<oxc::OrthodoxCalendar> calendar() const { return calendar_; }
  std::shared_ptr<oxc::Date> date() const { return date_; }
  oxc::CalendarFormat calendar_type() const;
  bool OnInit() override;
};

wxDECLARE_APP(MyApp);
