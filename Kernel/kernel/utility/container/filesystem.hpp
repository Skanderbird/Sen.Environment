#pragma once 

#include "kernel/utility/library.hpp"
#include "kernel/utility/assert.hpp"
#include "kernel/utility/platform/windows.hpp"
#include "kernel/utility/container/string.hpp"
#include "kernel/utility/container/path.hpp"
#include "kernel/utility/trait/trait.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

namespace Sen::Kernel::FileSystem
{
	
	inline static auto constexpr close_file =  [](FILE* file){ 
		if (file != nullptr) {
			std::fclose(file);
			file = nullptr;
		}
		return;
	};

	// give file path to open
	// return: the file data as string

	inline static auto read_file(
		std::string_view source
	) -> std::string 
	{
		#if WINDOWS
		auto file = std::ifstream(String::utf8view_to_utf16(fmt::format("\\\\?\\{}",
			String::to_windows_style(source.data()))).data());
		#else
		auto file = std::ifstream(source.data());
		#endif
        if (!file.is_open()) {
			#if WINDOWS
			throw Exception(fmt::format("{}: {}", Language::get("cannot_read_file"), String::to_posix_style(source.data())), std::source_location::current(), "read_file");
			#else
			throw Exception(fmt::format("{}: {}", Language::get("cannot_read_file"), std::string{source.data(), source.size()}), std::source_location::current(), "read_file");
			#endif
        }
        auto buffer = std::stringstream{};
        buffer << file.rdbuf();
		return buffer.str();
	}

	inline static auto read_quick_file (
		std::string_view source
	) -> std::string 
	{
		auto file_deleter = [](FILE* file) {
			if (file != nullptr) {
				std::fclose(file);
			}
		};
		#if WINDOWS
		auto wide_path = String::utf8view_to_utf16(fmt::format("\\\\?\\{}", String::to_windows_style(source.data())));
		auto file = WindowsFileReader{ wide_path };
		#else
		auto file = std::unique_ptr<FILE, decltype(file_deleter)>(std::fopen(std::string(source.data()).data(), "rb"), file_deleter);
		#endif
		#if WINDOWS
		#else
		assert_conditional(file != nullptr, fmt::format("{}: {}", Language::get("cannot_read_file"), std::string{ source.data(), source.size() }), "read_quick_file");
		#endif
		#if WINDOWS
		auto file_path = std::filesystem::path{wide_path};
		#else
		auto file_path = std::filesystem::path{std::string(source)};
		#endif
    	auto file_size = std::filesystem::file_size(file_path);
		auto content = std::string(file_size, '\0');
		#if WINDOWS
		auto bytes_read = DWORD{ 0 };
		auto state = ReadFile(file.handle, content.data(), static_cast<DWORD>(file_size), &bytes_read, nullptr);
		assert_conditional(SUCCEEDED(state), fmt::format("{}: {}", Language::get("cannot_read_file"), String::to_posix_style(std::string{ source.data(), source.size() })), "read_quick_file");
		assert_conditional(bytes_read == file_size, fmt::format("{}: {}", Language::get("cannot_read_file"), String::to_posix_style(std::string{ source.data(), source.size() })), "read_quick_file");
		#else
		auto count = std::fread(content.data(), 1, file_size, file.get());
		assert_conditional(count == file_size, fmt::format("{}: {}", Language::get("cannot_read_file"), String::to_posix_style(source.data())), "read_quick_file");
		#endif
		return content;
	}

	inline static auto read_json(
		std::string_view source
	) -> nlohmann::ordered_json
	{
		#if WINDOWS
		auto file = std::ifstream(String::utf8view_to_utf16(fmt::format("\\\\?\\{}",
			String::to_windows_style(source.data()))).data(), std::ios::binary);
		#else
		auto file = std::ifstream(source.data(), std::ios::binary);
		#endif
		#if WINDOWS
		assert_conditional(file.is_open(), fmt::format("{}: {}", Language::get("cannot_read_file"), String::to_posix_style(source.data())), "read_json");
		#else
		assert_conditional(file.is_open(), fmt::format("{}: {}", Language::get("cannot_read_file"), std::string{source.data(), source.size()}), "read_json");
		#endif
		auto buffer = std::stringstream{};
        buffer << file.rdbuf();
		return nlohmann::ordered_json::parse(buffer.str());
	}

	inline static auto write_json(
		std::string_view filepath,
		const nlohmann::ordered_json & content
	) -> void
	{
		#if WINDOWS
				auto file = std::unique_ptr<FILE, decltype(close_file)>(_wfopen(String::utf8view_to_utf16(fmt::format("\\\\?\\{}", String::to_windows_style(filepath.data()))).data(), L"w"), close_file);
		#else
				auto file = std::unique_ptr<FILE, decltype(close_file)>(std::fopen(filepath.data(), "w"), close_file);
		#endif
		if (file == nullptr) {
			#if WINDOWS
			throw Exception(fmt::format("{}: {}", Language::get("write_file_error"), String::to_posix_style(filepath.data())), std::source_location::current(), "write_json");
			#else
			throw Exception(fmt::format("{}: {}", Language::get("write_file_error"), std::string{filepath.data(), filepath.size()}), std::source_location::current(), "write_json");
			#endif
		}
		auto dumped_content = content.dump(1, '\t');
		std::fwrite(dumped_content.data(), 1, dumped_content.size(), file.get());
		return;
	}

	inline static auto write_json(
		std::string_view filepath,
		const nlohmann::ordered_json & content,
		int indent,
		char indent_char
	) -> void
	{
		#if WINDOWS
				#if !defined MSVC_COMPILER
						static_assert(false, "msvc compiler is required on windows");
				#endif
				auto file = std::unique_ptr<FILE, decltype(close_file)>(_wfopen(String::utf8view_to_utf16(fmt::format("\\\\?\\{}",
					String::to_windows_style(filepath.data()))).data(), L"w"), close_file);
		#else
				auto file = std::unique_ptr<FILE, decltype(close_file)>(std::fopen(filepath.data(), "w"), close_file);
		#endif
		#if WINDOWS
		assert_conditional(file != nullptr, fmt::format("{}: {}", Language::get("write_file_error"), String::to_posix_style(std::string{filepath.data(), filepath.size()})), "write_json");
		#else
		assert_conditional(file != nullptr, fmt::format("{}: {}", Language::get("write_file_error"), filepath), "write_json");
		#endif
		auto dumped_content = content.dump(indent, indent_char);
		std::fwrite(dumped_content.data(), 1, dumped_content.size(), file.get());
		return;
	}

	inline static auto write_json(
		std::string_view filepath,
		const nlohmann::ordered_json & content,
		char indent_char
	) -> void
	{
		#if WINDOWS
				#if !defined MSVC_COMPILER
						static_assert(false, "msvc compiler is required on windows");
				#endif
				auto file = std::unique_ptr<FILE, decltype(close_file)>(_wfopen(String::utf8view_to_utf16(fmt::format("\\\\?\\{}",
					String::to_windows_style(filepath.data()))).data(), L"w"), close_file);
		#else
				auto file = std::unique_ptr<FILE, decltype(close_file)>(std::fopen(filepath.data(), "w"), close_file);
		#endif
		#if WINDOWS
		assert_conditional(file != nullptr, fmt::format("{}: {}", Language::get("write_file_error"), String::to_posix_style(std::string{filepath.data(), filepath.size()})), "write_json");
		#else
		assert_conditional(file != nullptr, fmt::format("{}: {}", Language::get("write_file_error"), filepath), "write_json");
		#endif
		auto dumped_content = content.dump(1, indent_char);
		std::fwrite(dumped_content.data(), 1, dumped_content.size(), file.get());
		return;
	}

	inline static auto write_json(
		std::string_view filepath,
		const nlohmann::ordered_json &content,
		int indent,
		char indent_char,
		bool ensureAscii
	) -> void
	{
		#if WINDOWS
				#if !defined MSVC_COMPILER
						static_assert(false, "msvc compiler is required on windows");
				#endif
				auto file = std::unique_ptr<FILE, decltype(close_file)>(_wfopen(String::utf8view_to_utf16(fmt::format("\\\\?\\{}",
					String::to_windows_style(filepath.data()))).data(), L"w"), close_file);
		#else
				auto file = std::unique_ptr<FILE, decltype(close_file)>(std::fopen(filepath.data(), "w"), close_file);
		#endif
		#if WINDOWS
		assert_conditional(file != nullptr, fmt::format("{}: {}", Language::get("write_file_error"), String::to_posix_style(std::string{filepath.data(), filepath.size()})), "write_json");
		#else
		assert_conditional(file != nullptr, fmt::format("{}: {}", Language::get("write_file_error"), filepath), "write_json");
		#endif
		auto dumped_content = content.dump(indent, indent_char, ensureAscii);
		std::fwrite(dumped_content.data(), 1, dumped_content.size(), file.get());
		return;
	}

	// file path: the file uses utf16le encoding
	// return: the utf16le string

	inline static auto read_file_by_utf16(
		std::string_view source
	) -> std::wstring
	{
	#if WINDOWS
		auto wif = std::wifstream(String::utf8view_to_utf16(fmt::format("\\\\?\\{}",
			String::to_windows_style(source.data()))).data());
		if (!wif.is_open()) {
			throw Exception(fmt::format("{}: {}", Language::get("cannot_read_file"), String::view(String::to_posix_style(source.data()))), std::source_location::current(), "read_file_by_utf16");
		}
		wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));
		auto content = std::wstring((std::istreambuf_iterator<wchar_t>(wif)), std::istreambuf_iterator<wchar_t>());
		return content;
	#else
		auto wif = std::wifstream(source.data());
		if (!wif.is_open()) {
			throw Exception(fmt::format("{}: {}", Language::get("cannot_read_file"), source), std::source_location::current(), "read_file_by_utf16");
		}
		wif.imbue(std::locale(std::locale(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));
		auto content = std::wstring((std::istreambuf_iterator<wchar_t>(wif)), std::istreambuf_iterator<wchar_t>());
		return content;
	#endif
	}

	inline static auto write_file_by_utf16le(
		std::string_view source,
		const std::wstring & data
	) -> void
	{
		auto utf16le_locale = std::locale(std::locale::classic(), new std::codecvt_utf16<wchar_t, 0x10ffff,
			(std::codecvt_mode)(std::little_endian | std::generate_header)>);
		#if WINDOWS
		auto file = std::wofstream(String::utf8view_to_utf16(fmt::format("\\\\?\\{}",
			String::to_windows_style(source.data()))).data(), std::ios::binary);
		#else
		auto file = std::wofstream(source.data(), std::ios::binary);
		#endif
		if (!file.is_open()) {
			throw Exception(fmt::format("{}: {}", Language::get("cannot_read_file"), String::view(String::to_posix_style(source.data()))), std::source_location::current(), "write_file_by_utf16le");
		}
		file.imbue(utf16le_locale);
		file << data;
		return;
	}

	// file system

	namespace fs = std::filesystem;

	// directoryPath: folder path
	// return: create directory

	inline static auto create_directory(
		std::string_view path
	) -> void
	{
		#if WINDOWS
		if(fs::is_directory(String::utf8_to_utf16(path.data()))){
			return;
		}
		#else
		if(fs::is_directory(String::to_posix_style(path.data()))){
			return;
		}
		#endif
		#if WINDOWS
			fs::create_directories(String::utf8_to_utf16(fmt::format("\\\\?\\{}", String::to_windows_style(path.data()))));
		#else
			fs::create_directories(String::to_posix_style(path.data()));
		#endif
		return;
	}



	// path: file path to open
	// content: content to write
	// return: the file has been written

	inline static auto write_file(
		std::string_view filepath,
		std::string_view content
	) -> void
	{
		auto temporary = Path::to_posix_style(filepath.data());
		auto data = String::split(temporary, "/"_sv);
		data.erase(data.end() - 1, data.end());
		auto c = String::join(data, "/"_sv);
		create_directory(c);
		#if WINDOWS
		auto file = std::unique_ptr<FILE, decltype(close_file)>(_wfopen(String::utf8view_to_utf16(fmt::format("\\\\?\\{}", String::to_windows_style(filepath.data()))).data(), L"w"), close_file);
		#else
		auto file = std::unique_ptr<FILE, decltype(close_file)>(std::fopen(filepath.data(), "w"), close_file);
		#endif
		#if WINDOWS
		assert_conditional(file != nullptr, fmt::format("{}: {}", Language::get("write_file_error"), String::to_posix_style(std::string{filepath.data(), filepath.size()})), "write_file");
		#else
		assert_conditional(file != nullptr, fmt::format("{}: {}", Language::get("write_file_error"), filepath), "write_file");
		#endif
		std::fwrite(content.data(), 1, content.size(), file.get());
		return;
	}

	// filePath: the file path to write file
	// content: content to write
	// return: the file has been written

	inline static auto out_file(
		std::string_view filePath,
		std::string_view content
	) -> void
	{
		auto temporary = Path::normalize(filePath.data());
		auto data = String::split(temporary, "/"_sv);
		auto& last = data.at(data.size() - 1);
		data.pop_back();
		create_directory(String::join(data, "/"_sv));
		write_file(filePath, content);
		return;
	}

	// filePath: the file path to write file
	// content: json object
	// return: the file has been written to json

	inline static auto out_json(
		std::string_view filePath,
		const nlohmann::ordered_json &content
	) -> void
	{
		out_file(filePath, content.dump(1, '\t'));
		return;
	}

	// -------------------------------------------------
	
	template <typename T> requires CharacterBufferView<T> 
	inline static auto read_binary(
		std::string_view filepath
	) -> List<T>
	{
		#if WINDOWS
		auto file = WindowsFileReader{String::utf8_to_utf16(fmt::format("\\\\?\\{}",
				String::to_windows_style(filepath.data()))).data()};
		#else
		auto file = std::unique_ptr<FILE, decltype(&std::fclose)>(std::fopen(filepath.data(), "rb"), &std::fclose);
    	assert_conditional(file != nullptr, fmt::format("{}: {}", Language::get("cannot_read_file"), String::to_posix_style(filepath.data())), "read_binary");
		#endif
		#if WINDOWS
		auto size = std::filesystem::file_size(std::filesystem::path{String::utf8_to_utf16(filepath.data())});
		#else
		auto size = std::filesystem::file_size(std::filesystem::path{filepath});
		#endif
		auto data = List<T>{};
		data.reserve(size);
		#if WINDOWS
		data.resize(size);
		auto bytes_read = DWORD{};
		auto state = ReadFile(file.handle, data.data(), static_cast<DWORD>(size), &bytes_read, nullptr);
		assert_conditional(static_cast<bool>(SUCCEEDED(state)), fmt::format("{}: {}", Language::get("cannot_read_file"), String::to_posix_style(std::string{filepath.data(), filepath.size()})), "read_binary");
		assert_conditional(bytes_read == size, fmt::format("{}: {}", Language::get("cannot_read_file"), String::to_posix_style(std::string{filepath.data(), filepath.size()})), "read_binary");
		#else
		auto bytes_read = std::fread(data.data(), sizeof(T), size, file.get());
    	assert_conditional(bytes_read == size, fmt::format("{}: {}", Language::get("cannot_read_file"), String::to_posix_style(filepath.data())), "read_binary");
		#endif
		return data;	
	}

	inline static auto read_directory(
		std::string_view directory_path
	) -> List<std::string>
	{
		auto count = std::size_t{0};
		#if WINDOWS
			for (auto& c : fs::directory_iterator(String::utf8_to_utf16(directory_path.data()))) {
				++count;
			}
		#else
			for (auto& c : fs::directory_iterator(directory_path)) {
				++count;
			}
		#endif
		auto result = List<std::string>{};
		result.reserve(count); 
		#if WINDOWS
			for (auto& c : fs::directory_iterator(String::utf8_to_utf16(directory_path.data())))
				result.emplace_back(Path::normalize(String::utf16_to_utf8(c.path().wstring())));
		#else
			for (auto& c : fs::directory_iterator(directory_path))
				result.emplace_back(Path::normalize(c.path().string()));
		#endif

		return result;
	}

	inline static auto read_directory_only_file(
		std::string_view directory_path
	) -> List<std::string>
	{
		auto count = std::size_t{0};
		#if WINDOWS
			for (auto& c : fs::directory_iterator(String::utf8_to_utf16(directory_path.data())))
				if (c.is_regular_file()) {
					++count;
				}
		#else
			for (auto& c : fs::directory_iterator(directory_path))
				if (c.is_regular_file()) {
					++count;
				}
		#endif
		auto result = List<std::string>{};
		result.reserve(count);  
		#if WINDOWS
			for (auto& c : fs::directory_iterator(String::utf8_to_utf16(directory_path.data())))
				if (c.is_regular_file()) {
					result.emplace_back(Path::normalize(String::utf16_to_utf8(c.path().wstring())));
				}
		#else
			for (auto& c : fs::directory_iterator(directory_path))
				if (c.is_regular_file()) {
					result.emplace_back(Path::normalize(c.path().string()));
				}
		#endif

		return result;
	}

	inline static auto read_directory_only_directory(
		std::string_view directory_path
	) -> List<std::string> const
	{
		auto count = size_t{0};
		#if WINDOWS
			for (auto& c : fs::directory_iterator(String::utf8_to_utf16(directory_path.data())))
				if (c.is_directory()) ++count;
		#else
			for (auto& c : fs::directory_iterator(directory_path))
				if (c.is_directory()) ++count;
		#endif
		auto result = List<std::string>{};
		result.reserve(count); 
		#if WINDOWS
			for (auto& c : fs::directory_iterator(String::utf8_to_utf16(directory_path.data())))
				if (c.is_directory()) {
					result.emplace_back(Path::normalize(String::utf16_to_utf8(c.path().wstring())));
				}
		#else
			for (auto& c : fs::directory_iterator(directory_path))
				if (c.is_directory()) {
					result.emplace_back(Path::normalize(c.path().string()));
				}
		#endif
		return result;
	}

	inline static auto read_whole_directory(
		std::string_view directory_path
	) -> List<std::string>
	{
		auto result = List<std::string>{};
		auto directory = std::stack<std::string>{};
		directory.push(std::string(directory_path));
		while (!directory.empty()) {
			auto current_dir = directory.top();
			directory.pop();
			#if WINDOWS
				for (auto& c : fs::directory_iterator(String::utf8_to_utf16(current_dir)))
			#else
				for (auto& c : fs::directory_iterator(current_dir))
			#endif
			{
				if (c.is_directory()) {
					directory.push(c.path().string());
				}
				#if WINDOWS
					result.emplace_back(Path::normalize(String::utf16_to_utf8(c.path().wstring())));
				#else
					result.emplace_back(Path::normalize(c.path().string()));
				#endif
			}
		}

		return result;
	}

	template <typename T> requires CharacterBufferView<T>
	inline static auto write_binary(
		std::string_view path,
		const List<T> & data
	) -> void
	{
		#if defined(_WIN32) || defined(_WIN64)
		auto file = WindowsFileWriter{String::utf8_to_utf16(fmt::format("\\\\?\\{}", String::to_windows_style({path.data(), path.size()})))};
		auto bytesWritten = DWORD{0};
		auto result = WriteFile(
			file.handle, 
			data.data(), 
			static_cast<DWORD>(data.size()), 
			&bytesWritten, 
			nullptr
		);
		assert_conditional(SUCCEEDED(result), fmt::format("{}: {}", Language::get("write_file_error"), String::to_posix_style(std::string{path.data(), path.size()})), "write_binary");
        assert_conditional(bytesWritten == data.size(), fmt::format("{}: {}", Language::get("write_file_error"), String::to_posix_style(std::string{path.data(), path.size()})), "write_binary");
		#else
		auto file = std::unique_ptr<FILE, decltype(close_file)>(std::fopen(String::to_posix_style(std::string{path.data(), path.size()}).data(), "wb"), close_file);
		assert_conditional(file != nullptr, fmt::format("{}: {}", Language::get("write_file_error"), String::to_posix_style(path.data())), "write_binary");
		std::fwrite(reinterpret_cast<const char *>(data.data()), sizeof(T), data.size(), file.get());
		#endif
		return;
	}

	inline static auto make_xml_exception(
		tinyxml2::XMLError &status,
		int line_number
	) -> std::string
	{
		auto result = std::string{};
		switch (status) {
			case tinyxml2::XMLError::XML_CAN_NOT_CONVERT_TEXT: {
				result = "XML Error: Cannot convert text to the requested type.";
				break;
			}
			case tinyxml2::XMLError::XML_ELEMENT_DEPTH_EXCEEDED: {
				result = "XML Error: The nesting depth of elements exceeded the allowed limit.";
				break;
			}
			case tinyxml2::XMLError::XML_ERROR_COUNT: {
				result = "XML Error: Generic error count exceeded or unknown error occurred.";
				break;
			}
			case tinyxml2::XMLError::XML_ERROR_EMPTY_DOCUMENT: {
				result = "XML Error: The document is empty.";
				break;
			}
			case tinyxml2::XMLError::XML_ERROR_FILE_COULD_NOT_BE_OPENED: {
				result = "XML Error: The file could not be opened.";
				break;
			}
			case tinyxml2::XMLError::XML_ERROR_FILE_NOT_FOUND: {
				result = "XML Error: The specified file was not found.";
				break;
			}
			case tinyxml2::XMLError::XML_ERROR_FILE_READ_ERROR: {
				result = "XML Error: An error occurred while reading the file.";
				break;
			}
			case tinyxml2::XMLError::XML_ERROR_MISMATCHED_ELEMENT: {
				result = "XML Error: Mismatched element tags detected.";
				break;
			}
			case tinyxml2::XMLError::XML_NO_ATTRIBUTE: {
				result = "XML Error: The requested attribute was not found.";
				break;
			}
			case tinyxml2::XMLError::XML_WRONG_ATTRIBUTE_TYPE: {
				result = "XML Error: Attribute value type does not match the expected type.";
				break;
			}
			case tinyxml2::XMLError::XML_ERROR_PARSING_ELEMENT: {
				result = "XML Error: An error occurred while parsing an element.";
				break;
			}
			case tinyxml2::XMLError::XML_ERROR_PARSING_ATTRIBUTE: {
				result = "XML Error: An error occurred while parsing an attribute.";
				break;
			}
			case tinyxml2::XMLError::XML_ERROR_PARSING_TEXT: {
				result = "XML Error: An error occurred while parsing text.";
				break;
			}
			case tinyxml2::XMLError::XML_ERROR_PARSING_CDATA: {
				result = "XML Error: An error occurred while parsing a CDATA section.";
				break;
			}
			case tinyxml2::XMLError::XML_ERROR_PARSING_COMMENT: {
				result = "XML Error: An error occurred while parsing a comment.";
				break;
			}
			case tinyxml2::XMLError::XML_ERROR_PARSING_DECLARATION: {
				result = "XML Error: An error occurred while parsing an XML declaration.";
				break;
			}
			case tinyxml2::XMLError::XML_ERROR_PARSING_UNKNOWN: {
				result = "XML Error: An unknown error occurred during parsing.";
				break;
			}
			default: {
				result = "XML Error: Unknown or unhandled error occurred.";
				break;
			}
		}
		result += fmt::format(" Line number: {}", line_number);
		return result;
	}



	/**
	 * file path: the file path to read
	 * return: xml document
	*/

	inline static auto read_xml(
		std::string_view source,
		tinyxml2::XMLDocument* xml
	) -> void
	{
		#if WINDOWS
		auto file = std::ifstream(String::utf8view_to_utf16(fmt::format("\\\\?\\{}",
			String::to_windows_style(source.data()))).data());
		#else
		auto file = std::ifstream(source.data());
		#endif
        if (!file.is_open()) {
			throw Exception(fmt::format("{}: {}", Language::get("cannot_read_file"), String::to_posix_style(source.data())), std::source_location::current(), "read_xml");
        }
        auto buffer = std::stringstream{};
        buffer << file.rdbuf();
		auto status_code = xml->Parse(buffer.str().data(), buffer.str().size());
		assert_conditional(status_code == tinyxml2::XML_SUCCESS, fmt::format("{}: \"{}\". {}", Kernel::Language::get("xml.read_error"), String::view(String::to_posix_style(source.data())), make_xml_exception(status_code, xml->ErrorLineNum())), "read_xml");
		return;
	}

	/**
	 * file path: the file path to write
	 * xml document: the xml document
	 * notice: the function will delete the xml document after write
	 * return: xml dumped data
	*/

	inline static auto write_xml(
		std::string_view file_path,
		tinyxml2::XMLDocument* data
	) -> void
	{
		auto printer = tinyxml2::XMLPrinter{};
		data->Print(&printer);
		FileSystem::write_file(file_path, String::make_string_view(printer.CStr(), static_cast<std::size_t>(printer.CStrSize() - 1)));
		return;
	}

	inline static auto is_file(
		std::string_view source
	) -> bool
	{
		#if WINDOWS
		auto result = std::filesystem::is_regular_file(String::utf8_to_utf16(source.data()));
		#else
		auto result = std::filesystem::is_regular_file(source.data());
		#endif
		return result;
	}

	inline static auto is_directory(
		std::string_view source
	) -> bool
	{
		#if WINDOWS
		auto result = std::filesystem::is_directory(String::utf8_to_utf16(source.data()));
		#else
		auto result = std::filesystem::is_directory(source.data());
		#endif
		return result;
	}

	inline static auto rename(
		std::string_view source,
		std::string_view destination
	) -> void
	{
		#if WINDOWS
		std::filesystem::rename(std::filesystem::path{ String::utf8_to_utf16(source.data()) }, std::filesystem::path{ String::utf8_to_utf16(destination.data()) });
		#else
		std::filesystem::rename(std::filesystem::path{ source.data() }, std::filesystem::path{ destination.data() });
		#endif
		return;
	}

	inline static auto copy_directory(
		std::string_view source,
		std::string_view destination
	) -> void
	{
		#if WINDOWS
		std::filesystem::copy(std::filesystem::path{ String::utf8_to_utf16(source.data()) }, std::filesystem::path{ String::utf8_to_utf16(destination.data()) },
			std::filesystem::copy_options::recursive |
			std::filesystem::copy_options::overwrite_existing);
		#else
		std::filesystem::copy(std::filesystem::path{ source.data() }, std::filesystem::path{ destination.data() }, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
		#endif
		return;
	}

	inline static auto copy(
		std::string_view source,
		std::string_view destination
	) -> void
	{
		#if WINDOWS
		std::filesystem::copy(std::filesystem::path{ String::utf8_to_utf16(source.data()) }, std::filesystem::path{ String::utf8_to_utf16(destination.data()) });
		#else
		std::filesystem::copy(std::filesystem::path{ source.data() }, std::filesystem::path{ destination.data() });
		#endif
		return;
	}

	inline static auto remove(
		std::string_view source
	) -> void
	{
		#if WINDOWS
		std::filesystem::remove(std::filesystem::path{ String::utf8_to_utf16(source.data()) });
		#else
		std::filesystem::remove(std::filesystem::path{ source.data() });
		#endif
		return;
	}

	inline static auto remove_all(
		std::string_view source
	) -> void
	{
		#if WINDOWS
		std::filesystem::remove_all(std::filesystem::path{ String::utf8_to_utf16(source.data()) });
		#else
		std::filesystem::remove_all(std::filesystem::path{ source.data() });
		#endif
		return;
	}
}
