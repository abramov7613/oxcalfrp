#pragma once

#include "MKlimenko_resource.hpp"
#include <span>
#include <algorithm>
#include <charconv>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;

class Saver {
private:
	fs::path root{};
	std::vector<std::string> filenames;
	std::ofstream resource_holder_hpp;
	std::ofstream resource_hpp;
	std::mutex m;
	bool verbose = true;

	const std::string subfolder_name = "embedded_resources";

	constexpr static std::array resource_holder_begin_text{
		"class ResourceHolder {",
		"private:",
	};
	constexpr static std::array resource_holder_text{
		"",
		"public:",
		"\tauto Gather(const std::string& file) const {",
		"\t\tauto it = std::find_if(resources.begin(), resources.end(), [&file](const auto& lhs) {",
		"\t\t\treturn lhs.GetPath() == file;",
		"\t\t});",
		"\t\tif (it == resources.end())",
		"\t\t\tthrow std::runtime_error(\"Unable to detect resource with name \" + file);",
		"\t\t",
		"\t\treturn it->GetArray();",
		"\t}",
		"",
		"\tauto Gather(const char* file) const {",
		"\t\treturn Gather(std::string(file));",
		"\t}",
		"",
		"\tauto ListFiles() const {",
		"\t\tstd::vector<std::string> dst{};",
		"\t\tdst.reserve(resources.size());",
		"\t\tfor (auto&el : resources)",
		"\t\t\tdst.push_back(el.GetPath());",
		"",
		"\t\treturn dst;",
		"\t}",
		"",
		"\tauto FindByFilename(const std::string& file) const {",
		"\t\tstd::vector<Resource> dst{};",
		"\t\tdst.reserve(resources.size());",
		"\t\tstd::copy_if(resources.begin(), resources.end(), std::back_inserter(dst), [&file](const auto& item) {",
		"\t\t\tauto path = item.GetPath();",
		"\t\t\tauto last_forward = path.find_last_of('\\\\');",
		"\t\t\tauto last_inverse = path.find_last_of('/');",
		"\t\t\t",
		"\t\t\tif (last_forward != std::string::npos)",
		"\t\t\t\tpath = path.substr(last_forward + 1);",
		"\t\t\telse if (last_inverse != std::string::npos)",
		"\t\t\t\tpath = path.substr(last_inverse + 1);",
		"\t\t\treturn path == file;",
		"\t\t});",
		"\t\t",
		"\t\treturn dst;",
		"\t}",
		"",
		"\tauto FindByFilename(const char* file) const {",
		"\t\treturn FindByFilename(std::string(file));",
		"\t}",
		"",
		"\tauto operator()(const std::string& file) const {",
		"\t\treturn Gather(file);",
		"\t}",
		"",
		"\tauto operator()(const char* file) const {",
		"\t\treturn Gather(std::string(file));",
		"\t}",
		"};",
	};
	constexpr static std::array resource_text{
		"#pragma once",
		"",
		"#include <algorithm>",
		"#include <array>",
		"#include <cstdint>",
		"#include <functional>",
		"#include <iterator>",
		"#include <span>",
		"#include <tuple>",
		"#include <vector>",
		"",
		"class Resource {",
		"public:",
		"\ttemplate <typename T>",
		"\tusing span = std::span<T>;",
		"\tusing EmbeddedData = span<const std::uint8_t>;",
		"",
		"private:",
		"\tconst EmbeddedData arr_view;",
		"\tconst std::string path;",
		"",
		"public:",
		"\tResource() = default;",
		"\ttemplate <typename Container>",
		"\tResource(const Container& arr_, std::string path_) : arr_view(arr_), path(std::move(path_)) {}",
		"",
		"\tauto GetArray() const {",
		"\t\treturn arr_view;",
		"\t}",
		"",
		"\tauto& GetPath() const {",
		"\t\treturn path;",
		"\t}",
		"};",
	};

	auto Format(int val) const {
		std::array<char, 5> str;
		auto [p, ec] = std::to_chars(str.data(), str.data() + str.size(), val);
		*p++ = ',';
		return std::string(str.data(), p - str.data());
	}

	auto FromFilename(const std::string& filename) {
		return fs::path(root).append(filename).string();
	}

	bool IsSame(const std::stringstream& data, fs::path resource_path) const {
		std::ifstream test_file(resource_path);
		if (!test_file)
			return false;

		return std::string(std::istreambuf_iterator<char>(test_file), std::istreambuf_iterator<char>()) == data.str();
	}

	struct AsyncPrinter final {
		std::stringstream ss;

		~AsyncPrinter() {
			std::cout << ss.str();
		}

		template <typename T>
		friend AsyncPrinter& operator<<(AsyncPrinter& printer, const T& value) {
			printer.ss << value;
			return printer;
		}

		friend AsyncPrinter& operator<<(AsyncPrinter& printer, std::ostream& (*f)(std::ostream&)) {
			f(printer.ss);
			return printer;
		}
	};

public:
	Saver(fs::path root_path) :
		root(root_path),
		resource_holder_hpp(FromFilename("resource_holder.hpp")),
		resource_hpp(FromFilename("resource.hpp")) {

		if (!resource_holder_hpp.is_open())
			throw std::runtime_error("Unable to create helper header");
		if (!resource_hpp.is_open())
			throw std::runtime_error("Unable to create resource header");

		auto subfolder = root.append(subfolder_name);
		if (!fs::exists(subfolder))
			fs::create_directory(subfolder);
	}

	~Saver() noexcept {
		try {
			resource_holder_hpp << "#pragma once" << std::endl << std::endl;
			resource_holder_hpp << "#include \"resource.hpp\"" << std::endl;

			for (auto& el : filenames)
				resource_holder_hpp << "#include \"" << subfolder_name << "/" << el << ".hpp\"" << std::endl;

			resource_holder_hpp << std::endl;

			for (auto& el : resource_holder_begin_text)
				resource_holder_hpp << el << std::endl;

			resource_holder_hpp << "\tstd::array<Resource, " << filenames.size() << "> resources {" << std::endl;
			for (auto& el : filenames)
				resource_holder_hpp << "\t\tResource(" << el << ",\t" << el << "_path)," << std::endl;
			resource_holder_hpp << "\t};" << std::endl;

			for (auto& el : resource_holder_text)
				resource_holder_hpp << el << std::endl;

			for (auto& el : resource_text)
				resource_hpp << el << std::endl;

		}
		catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
		}
		catch (...) {
			std::cerr << "Unknown exception has occured" << std::endl;
		}
	}

	void Save(const Resource& res) {
		Save(res.GetArray(), res.GetPath());
	}

	void Save(Resource::EmbeddedData data, fs::path resource_path) {
		try {
			AsyncPrinter console_output;
			[[maybe_unused]]
			auto corrected_path = resource_path.make_preferred();
			if (verbose)
				console_output << "embed.exe: saving " << resource_path.string();

			auto array_filename = "resource_" + std::to_string(fs::hash_value(resource_path));
			{
				if (std::find(filenames.begin(), filenames.end(), array_filename) != filenames.end()) {
					if (verbose)
						console_output << " ... Skipped as a duplicate" << std::endl;
					return;
				}
				auto lock = std::scoped_lock(m);
				filenames.push_back(array_filename);
			}
			auto header_filename = array_filename + ".hpp";
			auto header_path = fs::path(root).append(header_filename);

			std::stringstream out;
			out << "#pragma once" << std::endl << std::endl;
			out << R"(#include "../resource_holder.hpp")" << std::endl << std::endl;
			out << "namespace { " << std::endl;
			out << "\tconst std::array<std::uint8_t, " << data.size() << "> " << array_filename << " {" << std::endl;
			out << "\t\t";
			for (auto& el : data)
				out << Format(el);

			out << std::endl << "\t};" << std::endl;
			out << "\tconst auto " << array_filename << "_path = R\"(" << resource_path.string() << ")\";" << std::endl << "}" << std::endl;

			if (IsSame(out, header_path)) {
				if (verbose)
					console_output << " ... Skipped" << std::endl;
				return;
			}
			std::ofstream out_file(header_path.c_str());
			if (!out_file.is_open())
				throw std::runtime_error("Unable to open file " + header_path.string());

			out_file << out.rdbuf();
			if (verbose)
				console_output << std::endl;
		}
		catch (...) {
			resource_holder_hpp << "static_assert(false, R\"(Error while embedding " << resource_path.string() << " file)\");" << std::endl;
			throw;
		}
	}
};
