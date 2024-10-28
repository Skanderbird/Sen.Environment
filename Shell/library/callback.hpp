#pragma once

#include "library/macro.hpp"
#include "library/dialog.hpp"
#include "version.hpp"

namespace Sen::Shell {
	
	namespace Console {

		inline auto static input(

		) -> std::string
		{
			auto str = std::string{};
			#if WINDOWS
			auto state_b = BOOL{};
			auto handle = GetStdHandle(STD_INPUT_HANDLE);
			auto handle_mode = DWORD{};
			if (GetConsoleMode(handle, &handle_mode)) {
				auto text_16 = std::array<char16_t, 0x1000>{};
				auto length = DWORD{};
				state_b = ReadConsoleW(handle, text_16.data(), static_cast<DWORD>(text_16.size()), &length, nullptr);
				auto text_8 = utf16_to_utf8(std::u16string_view{text_16.data(), length - 2});
				str = std::move(reinterpret_cast<std::string &>(text_8));
			}
			else {
				std::getline(std::cin, str);
			}
			#else 
				std::getline(std::cin, str);
			#endif
			return str;
		}

		inline auto static print(
			const std::string & title,
			const std::string & message = std::string{""},
			const Sen::Shell::Interactive::Color color = Sen::Shell::Interactive::Color::DEFAULT
		) -> void
		{
			#if WINDOWS
				auto state_b = BOOL{};
				auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
				auto handle_mode = DWORD{};
				auto consoleInfo = CONSOLE_SCREEN_BUFFER_INFO{};
				GetConsoleScreenBufferInfo(handle, &consoleInfo);
				auto originalColor = consoleInfo.wAttributes;
				SetConsoleTextAttribute(handle, color);
				auto state = GetConsoleMode(handle, &handle_mode);
				if (state) {
					auto text = title + '\n';
					auto text_16 = utf8_to_utf16(text);
					state_b = WriteConsoleW(handle, text_16.data(), static_cast<DWORD>(text_16.size()), nullptr, nullptr);
				} 
				else {
					std::cout << title << '\n' << std::flush;
				}
			#else
				switch (color) {
					case Sen::Shell::Interactive::Color::RED: {
						std::cout << "\033[31m" << title << "\033[0m" << '\n' << std::flush;
						break;
					}
					case Sen::Shell::Interactive::Color::GREEN: {
						std::cout << "\033[32m" << title << "\033[0m" << '\n' << std::flush;
						break;
					}
					case Sen::Shell::Interactive::Color::CYAN: {
						std::cout << "\033[36m" << title << "\033[0m" << '\n' << std::flush;
						break;
					}
					case Sen::Shell::Interactive::Color::YELLOW: {
						std::cout << "\033[33m" << title << "\033[0m" << '\n' << std::flush;
						break;
					case Sen::Shell::Interactive::Color::DEFAULT: {
						std::cout << title << '\n' << std::flush;
						break;
					}
					default: {
						throw std::runtime_error("Unknown color");
					}
				}
			#endif
			#if WINDOWS
				SetConsoleTextAttribute(handle, originalColor);
			#endif
				if (message != "" && state) {
					# if WINDOWS
					if (state) {
						auto text = message + '\n';
						auto text_16 = utf8_to_utf16(text);
						state_b = WriteConsoleW(handle, text_16.data(), static_cast<DWORD>(text_16.size()), nullptr, nullptr);
					} else {
						std::cout << message << '\n' << std::flush;
					}
					#else
						std::cout << message << '\n' << std::flush;
					#endif
				}
				return;
			}


		inline static auto convert_color(
			const std::string& that
		) -> Sen::Shell::Interactive::Color
		{
			if (that == "red") {
				return Sen::Shell::Interactive::Color::RED;
			}
			if (that == "green") {
				return Sen::Shell::Interactive::Color::GREEN;
			}
			if (that == "cyan") {
				return Sen::Shell::Interactive::Color::CYAN;
			}
			if (that == "yellow") {
				return Sen::Shell::Interactive::Color::YELLOW;
			}
			return Sen::Shell::Interactive::Color::DEFAULT;
		}
	}

	inline static auto callback(
		CStringList* list,
		CStringView* destination
	) -> void
	{
		auto result = StringList::to_vector(*list);
		assert_conditional(result.size() >= 1, "argument must be greater than 1");
		if (result[0] == "display") {
			delete[] copy;
			copy = nullptr;
			assert_conditional(result.size() >= 2, "argument must be greater than 2");
			switch (result.size()) {
				case 2: {
					Console::print(result[1]);
					break;
				}
				case 3: {
					Console::print(result[1], result[2]);
					break;
				}
				case 4: {
					Console::print(result[1], result[2], Console::convert_color(result[3]));
					break;
				}
			}
			return;
		}
		if (result[0] == "input") {
			delete[] copy;
			copy = nullptr;
			auto c = Console::input();
			copy = new char[c.size() + 1];
			std::memcpy(copy, c.data(), c.size());
			copy[c.size()] = '\0';
			destination->size = c.size();
			destination->value = copy;
			return;
		}
		if (result[0] == "is_gui") {
			delete[] copy;
			copy = nullptr;
			destination->value = "0";
			destination->size = 1;
			return;
		}
		if (result[0] == "wait") {
			#if WINDOWS
					auto hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
					SetConsoleTextAttribute(hConsole, Sen::Shell::Interactive::Color::CYAN);
					std::cout << "● " << std::flush;
			#else
					std::cout << "\033[36m● \033[0m";
			#endif
			return;
		}
		if (result[0] == "clear") {
			#if WINDOWS
					auto hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
					SetConsoleTextAttribute(hConsole, Sen::Shell::Interactive::Color::DEFAULT);
			#endif
			return;
		}
		if (result[0] == "version") {
			delete[] copy;
			copy = nullptr;
			auto version = std::to_string(Sen::Shell::version);
			copy = new char[version.size() + 1];
			std::memcpy(copy, version.data(), version.size());
			copy[version.size()] = '\0';
			destination->size = version.size();
			destination->value = copy;
			return;
		}
		if (result[0] == "host") {
			delete[] copy;
			copy = nullptr;
			assert_conditional(result.size() >= 5, "argument must be greater than 5");
			auto svr = Server{};
			svr.Get(std::format("/{}", result[1]), [&](const Request&, Response& res) {
				res.set_content(result[2], "text/html");
			});
			svr.listen(result[3], std::stoi(result[4]));
			return;
		}
		if (result[0] == "pick_file") {
			delete[] copy;
			copy = nullptr;
			#if WINDOWS
				auto raw_selection = Dialog::pick_path(false, false);
				if (!raw_selection.empty()) {
					copy = new char[raw_selection[0].size() + 1];
					std::memcpy(copy, raw_selection[0].data(), raw_selection[0].size());
					copy[raw_selection[0].size()] = '\0';
					destination->size = raw_selection[0].size();
					destination->value = copy;
				}
			#elif LINUX || MACINTOSH
				auto raw_selection = tinyfd_openFileDialog("", nullptr, 0, nullptr, nullptr, false);
				if (raw_selection != nullptr) {
					copy = new char[std::strlen(raw_selection) + 1];
					std::memcpy(copy, raw_selection, strlen(raw_selection));
					copy[std::strlen(raw_selection)] = '\0';
					destination->size = std::strlen(raw_selection);
					destination->value = copy;
				}
			#else
				throw std::runtime_error{ "invalid method" };
			#endif
			return;
		}
		if (result[0] == "pick_directory") {
			delete[] copy;
			copy = nullptr;
			#if WINDOWS
			auto raw_selection = Dialog::pick_path(true, false);
			if (!raw_selection.empty()) {
				copy = new char[raw_selection[0].size() + 1];
				std::memcpy(copy, raw_selection[0].data(), raw_selection[0].size());
				copy[raw_selection[0].size()] = '\0';
				destination->size = raw_selection[0].size();
				destination->value = copy;
			}
			#elif LINUX || MACINTOSH
			auto raw_selection = tinyfd_selectFolderDialog("", nullptr);
			if (raw_selection != nullptr) {
				copy = new char[strlen(raw_selection) + 1];
				std::memcpy(copy, raw_selection, strlen(raw_selection));
				copy[std::strlen(raw_selection)] = '\0';
				destination->size = std::strlen(raw_selection);
				destination->value = copy;
			}
			#endif
			return;
		}
		if (result[0] == "push_notification") {
			delete[] copy;
			copy = nullptr;
			assert_conditional(result.size() >= 3, "argument must be greater than 3");
			tinyfd_notifyPopup(result[1].data(), result[2].data(), "info");
			return;
		}
		if (result[0] == "finish") {
			return;
		}
		return;
	}

}