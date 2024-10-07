#pragma once 

#include "kernel/definition/library.hpp"
#include "kernel/definition/assert.hpp"
#include "kernel/definition/basic/string.hpp"
#include "kernel/definition/basic/path.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

namespace Sen::Kernel::FileSystem
{

	template <typename T>
	concept CharacterBufferView = std::is_same_v<T, char> || std::is_same_v<T, unsigned char>;

	/**
	 * Lambda
	*/
	
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
			throw Exception(fmt::format("{}: {}", Language::get("cannot_read_file"), String::to_posix_style(source.data())), std::source_location::current(), "read_file");
        }
        auto buffer = std::stringstream{};
        buffer << file.rdbuf();
		return buffer.str();
	}

	// Provide file path to read json
	// return: if the json is valid, the json data will be parsed as object

	inline static auto read_json(
		std::string_view source
	) -> std::shared_ptr<nlohmann::ordered_json> const 
	{
		auto file = std::ifstream(String::utf8view_to_utf16(fmt::format("\\\\?\\{}",
			String::to_windows_style(source.data()))).data());
        if (!file.is_open()) {
			throw Exception(fmt::format("{}: {}", Language::get("cannot_read_file"), String::to_posix_style(source.data())), std::source_location::current(), "read_json");
        }
        auto buffer = std::stringstream{};
        buffer << file.rdbuf();
		return std::make_shared<nlohmann::ordered_json>(nlohmann::ordered_json::parse(buffer.str()));
	}

	// Provide file path to write
	// Provide json content to serialize & write
	// return: writed json content

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
			throw Exception(fmt::format("{}: {}", Language::get("write_file_error"), String::to_posix_style(filepath.data())), std::source_location::current(), "write_json");
		}
		auto dumped_content = content.dump(1, '\t');
		std::fwrite(dumped_content.data(), 1, dumped_content.size(), file.get());
		return;
	}

	// Provide file path to write
	// Provide json content to serialize & write
	// indent: provide indentation
	// indent_char: indentation in segment line
	// return: writed json content

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
		if (file == nullptr) {
			throw Exception(fmt::format("{}: {}", Language::get("write_file_error"), filepath), std::source_location::current(), "write_json");
		}
		auto dumped_content = content.dump(indent, indent_char);
		std::fwrite(dumped_content.data(), 1, dumped_content.size(), file.get());
		return;
	}

	// Provide file path to write
	// Provide json content to serialize & write
	// indent_char: indentation in segment line
	// return: writed json content

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
		if (file == nullptr) {
			throw Exception(fmt::format("{}: {}", Language::get("write_file_error"), filepath), std::source_location::current(), "write_json");
		}
		auto dumped_content = content.dump(1, indent_char);
		std::fwrite(dumped_content.data(), 1, dumped_content.size(), file.get());
		return;
	}

	// Provide file path to write
	// Provide json content to serialize & write
	// indent: provide indentation
	// indent_char: indentation in segment line
	// ensure ascii: will it ensure ascii?
	// return: writed json content

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
		if (file == nullptr) {
			throw Exception(fmt::format("{}: {}", Language::get("cannot_read_file"), filepath), std::source_location::current(), "write_json");
		}
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
		auto wif = std::wifstream(std::wstring{ source.begin(), source.end() });
		if (!wif.is_open()) {
			throw Exception(fmt::format("{}: {}", Language::get("cannot_read_file"), source), std::source_location::current(), "read_file_by_utf16");
		}
		wif.imbue(std::locale(std::locale(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));
		auto content = std::wstring((std::istreambuf_iterator<wchar_t>(wif)), std::istreambuf_iterator<wchar_t>());
		return content;
	#endif
	}

	// filePath: the file path to write
	// data: utf16-le charset
	// return: the data has been written

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
		if(fs::is_directory(path.data())){
			return;
		}
		#endif
		#if WINDOWS
			fs::create_directories(String::utf8_to_utf16(fmt::format("\\\\?\\{}", String::to_windows_style(path.data()))));
		#else
			fs::create_directories(path);
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
		if (file == nullptr) {
			throw Exception(fmt::format("{}: {}", Language::get("cannot_write_file"), String::to_posix_style(filepath.data())), std::source_location::current(), "write_file");
		}
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
	) -> std::vector<T> const
	{
		#if WINDOWS
		auto file = std::ifstream(String::utf8_to_utf16(fmt::format("\\\\?\\{}",
				String::to_windows_style(filepath.data()))).data(), std::ios::binary);
		#else
		auto file = std::ifstream(filepath.data(), std::ios::binary);
		#endif
		assert_conditional(file.is_open(), fmt::format("{}: {}", Language::get("cannot_read_file"), String::to_posix_style(filepath.data())), "read_binary");
		file.seekg(0, std::ios::end);
		auto size = static_cast<std::streamsize>(file.tellg());
		file.seekg(0, std::ios::beg);
		auto data = std::vector<T>(size);
		assert_conditional(file.read(reinterpret_cast<char*>(data.data()), size), fmt::format("{}: {}", Language::get("cannot_read_file"), String::to_posix_style(filepath.data())), "read_binary");
		return data;	
	}

	// dirPath: directory to read
	// return: everything inside it even directory or file

	inline static auto read_directory(
		std::string_view directory_path
	) -> std::vector<std::string> const
	{
		auto result = std::vector<std::string>{};
		#if WINDOWS
			for (auto& c : fs::directory_iterator(String::utf8_to_utf16(directory_path.data())))
		#else
			for (auto& c : fs::directory_iterator(directory_path))
		#endif
		{
			#if WINDOWS
				result.emplace_back(Path::normalize(String::utf16_to_utf8(c.path().wstring())));
			#else
				result.emplace_back(Path::normalize(c.path().string()));
			#endif
		}
		return result;
	}

	// dirPath: directory to read
	// return: only files inside

	inline static auto read_directory_only_file(
		std::string_view directory_path
	) -> std::vector<string> const
	{
		auto result = std::vector<string>{};
		#if WINDOWS
				for (auto& c : fs::directory_iterator(String::utf8_to_utf16(directory_path.data())))
		#else
				for (auto& c : fs::directory_iterator(directory_path))
		#endif
		{
			if(c.is_regular_file()){
				#if WINDOWS
					result.emplace_back(Path::normalize(String::utf16_to_utf8(c.path().wstring())));
				#else
					result.emplace_back(Path::normalize(c.path().string()));
				#endif
			}
		}
		return result;
	}

	// dirPath: directory to read
	// return: only dirs inside

	inline static auto read_directory_only_directory(
		std::string_view directory_path
	) -> std::vector<std::string> const
	{
		auto result = std::vector<std::string>{};
		#if WINDOWS
				for (auto& c : fs::directory_iterator(String::utf8_to_utf16(directory_path.data())))
		#else
				for (auto& c : fs::directory_iterator(directory_path))
		#endif
		{
			if(c.is_directory()){
				#if WINDOWS
					result.emplace_back(Path::normalize(String::utf16_to_utf8(c.path().wstring())));
				#else
					result.emplace_back(Path::normalize(c.path().string()));
				#endif
			}
		}
		return result;
	}


	// dirPath: directory to read
	// return: only files inside nested directories

	inline static auto read_whole_directory(
		std::string_view directory_path
	) -> std::vector<std::string> const
	{
		auto result = std::vector<string>{};
		#if WINDOWS
				for (auto& c : fs::directory_iterator(String::utf8_to_utf16(directory_path.data())))
		#else
				for (auto& c : fs::directory_iterator(directory_path))
		#endif
		{
			if(c.is_directory()){
				#if WINDOWS
					for (auto& e : read_whole_directory(String::utf16_to_utf8(c.path().wstring())))
				#else
					for (auto& e : read_whole_directory(c.path().string()))
				#endif
				{
					result.emplace_back(Path::normalize(e));
				}
			}
			else{
				#if WINDOWS
					result.emplace_back(Path::normalize(String::utf16_to_utf8(c.path().wstring())));
				#else
					result.emplace_back(Path::normalize(c.path().string()));
				#endif
			}
		}
		return result;
	}

	/**
	 * outFile: the output file to write
	 * data: data to write
	 * return: the written file
	*/


	template <typename T> requires CharacterBufferView<T>
	inline static auto write_binary(
		std::string_view filepath,
		const std::vector<T> & data
	) -> void
	{
		#if WINDOWS
				auto file = std::unique_ptr<FILE, decltype(close_file)>(_wfopen(String::utf8view_to_utf16(fmt::format("\\\\?\\{}",
				String::to_windows_style(filepath.data()))).data(), L"wb"), close_file);
		#else
				auto file = std::unique_ptr<FILE, decltype(close_file)>(std::fopen(filepath.data(), "wb"), close_file);
		#endif
		if(file == nullptr){
			throw Exception(fmt::format("{}: {}", Language::get("write_file_error"), String::to_posix_style(filepath.data())), std::source_location::current(), "write_binary");
		}
		std::fwrite(reinterpret_cast<const char *>(data.data()), sizeof(T), data.size(), file.get());
		return;
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
			throw Exception(fmt::format("{}: {}", Language::get("cannot_read_file"), String::to_posix_style(source.data())), std::source_location::current(), "read_json");
        }
        auto buffer = std::stringstream{};
        buffer << file.rdbuf();
		auto status_code = xml->Parse(buffer.str().data(), buffer.str().size());
		assert_conditional(status_code == tinyxml2::XML_SUCCESS, fmt::format("{}: {}", Kernel::Language::get("xml.read_error"), String::view(String::to_posix_style(source.data()))), "read_xml");
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

	class FileHandler {

		private:

			template <typename T, typename CloseMethod>
			using Pointer = std::unique_ptr<T, CloseMethod>;

		protected:

			inline static auto constexpr close_file = [](FILE* file) {
				if (file != nullptr) {
					std::fclose(file);
					file = nullptr;
				}
				return;
			};


		public:

			Pointer<std::FILE, decltype(close_file)> file{};

			FileHandler(
				std::string_view source,
				std::string_view mode
			) 
			{
				#if WINDOWS
					file.reset(_wfopen(String::utf8_to_utf16(String::to_windows_style(source.data())).data(), String::utf8view_to_utf16(mode).data()));
				#else
					file.reset(std::fopen(source.data(), mode.data()));
				#endif
				assert_conditional(file != nullptr, String::format(fmt::format("{}", Language::get("file_is_nullptr")), String::to_posix_style(source.data())), "FileHandler");
			}

			auto close(

			) -> void
			{
				thiz.file.reset(nullptr);
				return;
			}

			auto read(

			) -> char
			{
				return std::fgetc(thiz.file.get());
			}

			auto position(
			) -> std::size_t
			{
				return fsize(thiz.file.get());
			}

			auto position(
				std::size_t pos
			) -> void
			{
				std::fseek(thiz.file.get(), 0, pos);
				return;
			}

			auto size(

			) -> std::size_t
			{
				std::fseek(thiz.file.get(), 0, SEEK_END);
				auto file_size = fsize(thiz.file.get());
				std::fseek(thiz.file.get(), 0, SEEK_SET);
				return file_size;
			}

			auto read_all (

			) -> std::vector<uint8_t>
			{
				auto file_size = size();
				auto data = std::vector<uint8_t>{};
				data.resize(file_size);
				std::fread(data.data(), 1, file_size, thiz.file.get());
				return data;
			}

			auto write_all (
				const std::vector<uint8_t>& data
			) -> void
			{
				std::fwrite(reinterpret_cast<char const*>(data.data()), 1, data.size(), thiz.file.get());
				return;
			}

			auto write (
				char data
			) -> void
			{
				std::fputc(static_cast<int>(data), thiz.file.get());
				return;
			}

			FileHandler(
			) = delete;

			FileHandler(
				FileHandler const& that
			) = delete;

			FileHandler(
				FileHandler&& that
			) = delete;

			~FileHandler(
			) = default;


	};

	class FileSystemWatcher {
		public:
			using Callback = std::function<void(const std::string& event, const std::string& filename)>;

			FileSystemWatcher(const std::string& directory)
				: directory(directory) {}

			void start(Callback callback) {
				this->callback = callback;

		#ifdef _WIN32
				watch_windows();
		#else
				assert_conditional(false, fmt::format("{}", Language::get("os.system.process_invalid")), "start");
		#endif
			}

		private:
			std::string directory;
			
			Callback callback;

		#ifdef _WIN32
			void watch_windows() {
				auto hDir = CreateFile(
					directory.c_str(),
					FILE_LIST_DIRECTORY,
					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
					nullptr,
					OPEN_EXISTING,
					FILE_FLAG_BACKUP_SEMANTICS,
					nullptr
				);
				assert_conditional(!(hDir == INVALID_HANDLE_VALUE), fmt::format("{}", Language::get("windows.process.failed_to_open_directory_to_handle")), "watch_windows");
				char buffer[1024];
				auto bytesReturned = DWORD{};
				while (true) {
					if (ReadDirectoryChangesW(
							hDir,
							buffer,
							sizeof(buffer),
							TRUE,
							FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE,
							&bytesReturned,
							nullptr,
							nullptr
						)) {

						FILE_NOTIFY_INFORMATION *pNotify;
						auto offset = int{0};
						do {
							pNotify = (FILE_NOTIFY_INFORMATION *) &buffer[offset];
							auto wfilename = std::wstring(pNotify->FileName, pNotify->FileName + pNotify->FileNameLength / sizeof(WCHAR));
							auto size_needed = WideCharToMultiByte(CP_UTF8, 0, wfilename.c_str(), static_cast<int>(wfilename.size()), nullptr, 0, nullptr, nullptr);
							auto filename = std::string(size_needed, 0);
							WideCharToMultiByte(CP_UTF8, 0, wfilename.c_str(), static_cast<int>(wfilename.size()), &filename[0], size_needed, nullptr, nullptr);
							auto event = std::string{};
							switch (pNotify->Action) {
								case FILE_ACTION_ADDED:
									event = "add";
									break;
								case FILE_ACTION_REMOVED:
									event = "delete";
									break;
								case FILE_ACTION_MODIFIED:
									event = "update";
									break;
								case FILE_ACTION_RENAMED_OLD_NAME:
								case FILE_ACTION_RENAMED_NEW_NAME:
									event = "rename";
									break;
								default:
									event = "unknown";
									break;
								}
							callback(event, filename);
							offset += pNotify->NextEntryOffset;
						} while (pNotify->NextEntryOffset != 0);
					}
				}
				CloseHandle(hDir);
			}
		#endif
		};
}