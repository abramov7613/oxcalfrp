#include <filesystem>
#include <iostream>
#include <string>
using std::cout;
using std::endl;
namespace fs = std::filesystem;
int main(int argc, char* argv[])
{
	if(argc==1) {
		cout << "Usage:\n\t" << argv[0] << " <input_file>" << endl;
		return -1;
	}
	fs::path p = argv[1];
	if(!fs::exists(p)) {
		cout << "File not found: " << argv[1] << endl;
		return -1;
	}
	auto result = "resource_" + std::to_string(fs::hash_value(p)) + ".hpp";
	cout << result << endl;
	return 0;
}
