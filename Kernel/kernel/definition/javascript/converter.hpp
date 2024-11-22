#pragma once

#include "kernel/definition/javascript/value.hpp"
#include "kernel/definition/javascript/runtime.hpp"

namespace Sen::Kernel::Definition::JavaScript::Converter {

    namespace Detail {

        inline auto handle_conversion_error(JSContext* context, const JSValue& value, const std::string& type) -> void {
            throw Exception(fmt::format("Failed to convert JS value to {}", type), std::source_location::current(), "handle_conversion_error");
        }

    }

	inline static auto get_string(JSContext* context, const JSValue& value) -> std::string {
        auto size = std::size_t{};
        auto c_str = JS_ToCStringLen(context, &size, value);
        if (c_str == nullptr) {
			Detail::handle_conversion_error(context, value, "get_string");
		}
        auto str = std::string { c_str, size };
        JS_FreeCString(context, c_str);
        return str;
    }

    inline static auto get_int32(JSContext* context, const JSValue& value) -> int32_t {
        auto result = int32_t{};
        if (JS_ToInt32(context, &result, value) < 0) {
			Detail::handle_conversion_error(context, value, "get_int32");
		}
        return result;
    }

    inline static auto get_float64(JSContext* context, const JSValue& value) -> double {
        auto result = double{};
        if (JS_ToFloat64(context, &result, value) < 0) {
			Detail::handle_conversion_error(context, value, "get_float64");
		}
        return result;
    }

	inline static auto get_float32(JSContext* context, const JSValue& value) -> float {
        auto result = double{};
        if (JS_ToFloat64(context, &result, value) < 0) {
			Detail::handle_conversion_error(context, value, "get_float32");
		}
        return static_cast<float>(result);
    }

    inline static auto get_int64(JSContext* context, const JSValue& value) -> int64_t {
        auto result = int64_t{};
        if (JS_ToInt64(context, &result, value) < 0) {
			Detail::handle_conversion_error(context, value, "get_int64");
		}
        return result;
    }

    inline static auto get_bigint64(JSContext* context, const JSValue& value) -> int64_t {
        auto result = int64_t{};
        if (JS_ToBigInt64(context, &result, value) < 0) {
			Detail::handle_conversion_error(context, value, "get_bigint64");
		}
        return result;
    }

    inline static auto get_uint32(JSContext* context, const JSValue& value) -> uint32_t {
        auto result = uint32_t{};
        if (JS_ToUint32(context, &result, value) < 0) {
			Detail::handle_conversion_error(context, value, "get_uint32");
		}
        return result;
    }

	inline static auto get_uint64(JSContext* context, const JSValue& value) -> uint64_t {
        auto result = uint64_t{};
        if (JS_ToIndex(context, &result, value) < 0) {
			Detail::handle_conversion_error(context, value, "get_uint64");
		}
        return result;
    }

    inline static auto get_bool(JSContext* context, const JSValue& value) -> bool {
        return static_cast<bool>(JS_ToBool(context, value));
    }

    inline static auto to_string(JSContext* context, std::string_view content) -> JSValue {
        return JS_NewStringLen(context, content.data(), content.size());
    }

    inline static auto to_bool(JSContext* context, bool value) -> JSValue {
        return JS_NewBool(context, value ? 1 : 0);
    }

    inline static auto to_number(JSContext* context, int32_t value) -> JSValue {
        return JS_NewInt32(context, value);
    }

    inline static auto to_number(JSContext* context, double value) -> JSValue {
        return JS_NewFloat64(context, value);
    }

    inline static auto to_number(JSContext* context, uint32_t value) -> JSValue {
        return JS_NewUint32(context, value);
    }

    inline static auto to_number(JSContext* context, int64_t value) -> JSValue {
        return JS_NewInt64(context, value);
    }

    inline static auto to_number(JSContext* context, uint64_t value) -> JSValue {
        return JS_NewBigUint64(context, value);
    }

	template <typename T>
    inline static auto to_bigint(JSContext* context, T value) -> JSValue {
        if constexpr (std::is_same<T, uint64_t>::value) {
            return JS_NewBigUint64(context, value);
        } else if constexpr (std::is_integral<T>::value) {
            return JS_NewBigInt64(context, static_cast<int64_t>(value));
        }
    }

	#if WINDOWS
    inline static auto get_undefined() -> JSValue { return JS_UNDEFINED; }
    inline static auto get_null() -> JSValue { return JS_NULL; }
    #else
    inline static constexpr auto get_undefined() -> JSValue { return JS_UNDEFINED; }
    inline static constexpr auto get_null() -> JSValue { return JS_NULL; }
    #endif

	template <typename T>
	inline static auto get_vector(JSContext* context, const JSValue& that) -> List<T> {
		auto atom = Atom{context, "length"};
		auto length_value = JS_GetProperty(context, that, atom.value);
		auto length = Converter::get_int32(context, length_value);
		JS_FreeValue(context, length_value);
		auto m_list = List<T>{};
		for (auto i : Range<int>(length)) {
			auto value = JS_GetPropertyUint32(context, that, i);
			if constexpr (std::is_same<T, int>::value) {
				m_list.emplace_back(Converter::get_int32(context, value));
			} else if constexpr (std::is_same<T, bool>::value) {
				m_list.emplace_back(Converter::get_bool(context, value));
			} else if constexpr (std::is_same<T, long long>::value) {
				m_list.emplace_back(Converter::get_int64(context, value));
			} else if constexpr (std::is_same<T, uint32_t>::value) {
				m_list.emplace_back(Converter::get_uint32(context, value));
			} else if constexpr (std::is_same<T, uint64_t>::value) {
				m_list.emplace_back(Converter::get_uint64(context, value));
			} else if constexpr (std::is_same<T, float>::value) {
				m_list.emplace_back(Converter::get_float32(context, value));
			} else if constexpr (std::is_same<T, double>::value) {
				m_list.emplace_back(Converter::get_float64(context, value));
			} else if constexpr (std::is_same<T, std::string>::value) {
				m_list.emplace_back(Converter::get_string(context, value));
			} else {
				m_list.emplace_back(value);
			}
			JS_FreeValue(context, value);
		}
		return m_list;
	}

	template <typename T>
	inline static auto get_array(JSContext* context, const JSValue& that) -> List<T> {
		auto atom = Atom{context, "length"};
		auto length_value = JS_GetProperty(context, that, atom.value);
		auto length = Converter::get_int32(context, length_value);
		JS_FreeValue(context, length_value);
		auto m_list = List<T>{};
		for (auto i : Range<int>(length)) {
			auto value = JS_GetPropertyUint32(context, that, i);
			if constexpr (std::is_same<T, int>::value) {
				m_list.emplace_back(static_cast<T>(Converter::get_bigint64(context, value)));
			} else if constexpr (std::is_same<T, bool>::value) {
				m_list.emplace_back(Converter::get_bool(context, value));
			} else if constexpr (std::is_same<T, long long>::value) {
				m_list.emplace_back(static_cast<T>(Converter::get_bigint64(context, value)));
			} else if constexpr (std::is_same<T, uint32_t>::value) {
				m_list.emplace_back(static_cast<T>(Converter::get_bigint64(context, value)));
			} else if constexpr (std::is_same<T, uint64_t>::value) {
				m_list.emplace_back(static_cast<T>(Converter::get_bigint64(context, value)));
			} else if constexpr (std::is_same<T, float>::value) {
				m_list.emplace_back(static_cast<T>(Converter::get_float32(context, value)));
			} else if constexpr (std::is_same<T, double>::value) {
				m_list.emplace_back(static_cast<T>(Converter::get_float64(context, value)));
			} else {
				m_list.emplace_back(value);
			}

			JS_FreeValue(context, value);
		}
		return m_list;
	}

	template <>
	inline static auto get_vector<std::string>(JSContext* context, const JSValue& that) -> List<std::string> {
		auto atom = Atom{context, "length"};
		auto len_val = JS_GetProperty(context, that, atom.value);
		auto length = Converter::get_int32(context, len_val);
		JS_FreeValue(context, len_val);
		auto m_list = List<std::string>{};
		for (auto i : Range<int>(length)) {
			auto val = JS_GetPropertyUint32(context, that, i);
			m_list.emplace_back(Converter::get_string(context, val));
			JS_FreeValue(context, val);
		}
		return m_list;
	}

	template <>
	inline static auto get_vector<std::string_view>(JSContext* context, const JSValue& that) -> List<std::string_view> {
		auto atom = Atom{context, "length"};
		auto len_val = JS_GetProperty(context, that, atom.value);
		auto length = Converter::get_int32(context, len_val);
		JS_FreeValue(context, len_val);
		auto m_list = List<std::string_view>{};
		for (auto i : Range<int>(length)) {
			auto val = JS_GetPropertyUint32(context, that, i);
			m_list.emplace_back(Converter::get_string(context, val));
			JS_FreeValue(context, val);
		}
		return m_list;
	}

	template <typename T>
	inline static auto to_array(JSContext* ctx, const List<T>& vec) -> JSValue {
		auto js_array = JS_NewArray(ctx);
		for (auto i : Range<std::size_t>(vec.size())) {
			if constexpr (std::is_same<T, int>::value) {
				JS_SetPropertyUint32(ctx, js_array, i, JS_NewInt32(ctx, vec[i]));
			} else if constexpr (std::is_same<T, uint32_t>::value) {
				JS_SetPropertyUint32(ctx, js_array, i, JS_NewBigInt64(ctx, static_cast<int64_t>(vec[i])));
			} else if constexpr (std::is_same<T, float>::value) {
				JS_SetPropertyUint32(ctx, js_array, i, JS_NewFloat64(ctx, static_cast<double>(vec[i])));
			} else if constexpr (std::is_same<T, double>::value) {
				JS_SetPropertyUint32(ctx, js_array, i, JS_NewFloat64(ctx, vec[i]));
			} else if constexpr (std::is_same<T, uint64_t>::value) {
				JS_SetPropertyUint32(ctx, js_array, i, JS_NewBigUint64(ctx, vec[i]));
			} else if constexpr (std::is_same<T, bool>::value) {
				JS_SetPropertyUint32(ctx, js_array, i, JS_NewBool(ctx, vec[i]));
			} else if constexpr (std::is_same<T, long long>::value) {
				JS_SetPropertyUint32(ctx, js_array, i, JS_NewBigInt64(ctx, vec[i]));
			} else if constexpr (std::is_same<T, std::string>::value) {
				JS_SetPropertyUint32(ctx, js_array, i, JS_NewString(ctx, vec[i].c_str()));
			}
		}
		return js_array;
	}


	inline static auto constexpr close_buffer = [](auto buffer)
	{ 
		std::free(buffer); 
		return; 
	};

	inline static auto read_file_as_js_arraybuffer(
		JSContext *ctx, 
		std::string_view source
	) -> JSValue
	{
		#if WINDOWS
		auto fp = std::unique_ptr<FILE, decltype(Language::close_file)>(_wfopen(String::utf8_to_utf16(source.data()).data(), L"rb"), Language::close_file);
		#else
		auto fp = std::unique_ptr<FILE, decltype(Language::close_file)>(std::fopen(source.data(), "rb"), Language::close_file);
		#endif
		if (fp == nullptr) {
			#if WINDOWS
			throw Exception(fmt::format("{}: {}", Language::get("cannot_read_file"), String::to_posix_style(std::string{source.data(), source.size()})), std::source_location::current(), "read_file_as_js_arraybuffer");
			#else
			throw Exception(fmt::format("{}: {}", Language::get("cannot_read_file"), source), std::source_location::current(), "read_file_as_js_arraybuffer");
			#endif
		}
		#if WINDOWS
		auto file_size = std::filesystem::file_size(std::filesystem::path{String::utf8_to_utf16(source.data()) });
		#else
		auto file_size = std::filesystem::file_size(std::filesystem::path{ source });
		#endif
		auto buffer = std::unique_ptr<char[], decltype(close_buffer)>((char*) std::malloc(file_size * sizeof(char)), close_buffer);
		if (buffer == nullptr) {
			#if WINDOWS
			throw Exception(fmt::format("C malloc allocating memory failed, source file: {}", String::to_posix_style({source.data(), source.size()})), std::source_location::current(), "read_file_as_js_arraybuffer");
			#else
			throw Exception(fmt::format("C malloc allocating memory failed, source file: {}", source), std::source_location::current(), "read_file_as_js_arraybuffer");
			#endif
		}
		auto result = std::fread(buffer.get(), 1, file_size, fp.get());
		if (result != file_size) {
			throw Exception(fmt::format("{}: {}", Language::get("cannot_read_file"), String::to_posix_style({source.data(), source.size()})), std::source_location::current(), "read_file_as_js_arraybuffer");
		}
		auto array_buffer = JS_NewArrayBufferCopy(ctx, reinterpret_cast<uint8_t*>(buffer.get()), file_size);
		return array_buffer;
	}

	inline static auto write_file_as_arraybuffer(
		JSContext* ctx,
		std::string_view destination,
		const JSValue& that
	) -> void
	{
		auto size = size_t{};
		auto data = JS_GetArrayBuffer(ctx, &size, that);
		if (data == nullptr) {
			throw Exception(fmt::format("{}", Language::get("js.converter.failed_to_get_js_array_buffer")), std::source_location::current(), "write_file_as_arraybuffer");
		}
		#if WINDOWS
			auto ofs = std::unique_ptr<FILE, decltype(Language::close_file)>(_wfopen(String::utf8_to_utf16(fmt::format("\\\\?\\{}", String::to_windows_style(destination.data()))).data(), L"wb"), Language::close_file);
		#else
			auto ofs = std::unique_ptr<FILE, decltype(Language::close_file)>(std::fopen(destination.data(), "wb"), Language::close_file);
		#endif
		if (ofs.get() == nullptr) {
			#if WINDOWS
			throw Exception(fmt::format("{}", Language::get("open_write_failed"), String::to_posix_style({destination.data(), destination.size()})), std::source_location::current(), "write_file_as_arraybuffer");
				#else
			throw Exception(fmt::format("{}", Language::get("open_write_failed"), destination),std::source_location::current(), "write_file_as_arraybuffer");
			#endif
		}
		std::fwrite(reinterpret_cast<char*>(data), 1, size, ofs.get());
		return;
		}

		inline static auto to_binary_list(
			JSContext* context,
			JSValue array_buffer
		) -> List<uint8_t>
		{
			auto size = std::size_t{};
			auto data = JS_GetArrayBuffer(context, &size, array_buffer);
			return make_list(data, size);
		}


		inline static auto toArrayBuffer(
			JSContext* ctx,
			const List<uint8_t> & v
		) -> JSValue
		{
			return JS_NewArrayBufferCopy(ctx, v.data(), v.size());
		}
}