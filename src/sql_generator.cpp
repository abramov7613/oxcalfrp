#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>
#include <tuple>
#include <algorithm>

int main(int argc, char** argv)
{
  auto print_usage = [](const char* a){
    std::cout << "Usage: " << a << " out_file in_file1 [in_file2 in_file3 ...]" << std::endl;
  };
  if(argc<3) {
    print_usage(argv[0]);
    return -1;
  }
  std::vector<std::string> names_uniq;
  std::vector<std::tuple<int,int,std::string>> db; // month | day | name
  std::regex re_pattern("^(\\d{1,2}) (\\d{1,2}) (.+)$");
  for(int i = 2; i<argc; ++i) {
    std::ifstream input(argv[i]);
    if(!input.is_open()){
      std::cout << "can't open file " << argv[i] << std::endl;
      return -1;
    }
    for (std::string line; std::getline(input, line);) {
      std::smatch match;
      if (std::regex_match(line, match, re_pattern)) {
        int month = std::stoi(match[1].str());
        int day = std::stoi(match[2].str());
        db.push_back(std::make_tuple(month, day, match[3].str()));
      }
    }
    input.close();
  }
  std::sort(db.begin(), db.end(), [](const auto& a ,const auto& b){
    return std::get<std::string>(a) < std::get<std::string>(b);
  });
  names_uniq.reserve(db.size());
  std::transform(db.begin(), db.end(), std::back_inserter(names_uniq), [](const auto& e){
    return std::get<std::string>(e);
  });
  names_uniq.erase(std::unique(names_uniq.begin(), names_uniq.end()), names_uniq.end());
  std::ofstream out(argv[1], std::ios::out|std::ios::ate);
  if(!out.is_open()){
    std::cout << "can't open file " << argv[1] << std::endl;
    return -1;
  }
  out << "CREATE TABLE names (name_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, name TEXT NOT NULL);\n" ;
  out << "CREATE TABLE dates (month INT, day INT, name_id INT NOT NULL, "
         "FOREIGN KEY(name_id) REFERENCES names(name_id));\n" ;
  for(const auto& s: names_uniq) if(!s.empty())
    out << "INSERT INTO names(name) VALUES(\"" << s << "\");\n" ;
  std::for_each(db.begin(), db.end(), [&out, begin=names_uniq.begin(), end=names_uniq.end()](const auto& e){
    const int month = std::get<0>(e);
    const int day = std::get<1>(e);
    const auto& name = std::get<2>(e);
    if(auto it = std::lower_bound(begin, end, name); it != end) {
      int idx = std::distance(begin, it) + 1;
      out << "INSERT INTO dates VALUES(" << month << ", " << day << ", " << idx << ");\n" ;
    }
  });
  out.close();
  return 0;
}
