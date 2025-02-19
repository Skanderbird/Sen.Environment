
#pragma once

#include "kernel/utility/container/optional/optional.hpp"
#include "kernel/utility/stream/read_memory_stream.hpp"
#include "kernel/utility/stream/write_memory_stream.hpp"
#include "kernel/utility/stream/read_file_stream.hpp"
#include "kernel/utility/stream/write_file_stream.hpp"


#define VAR_TO_STRING(x) #x

namespace Sen::Kernel {
    template<typename T>
    concept is_not_void = (!std::is_void_v<T>);

    template<typename T>
    concept is_class = std::is_class_v<T> && (!std::is_void_v<T>);

    template<typename T>
    concept is_exchanger = std::is_same_v<T, std::remove_cvref_t<T>> && (!std::is_void_v<T>) && (!std::is_reference_v<T>);

    template<typename T>
    concept is_integer = std::is_arithmetic_v<T> && (!std::is_floating_point_v<T>);

    template<typename T>
    concept is_list = std::is_same_v<T, List<typename T::value_type>>;

    template<typename T>
    concept read_stream = std::is_same_v<T, ReadMemoryStream> || std::is_same_v<T, ReadFileStream>;

    template<typename T>
    concept write_stream = std::is_same_v<T, WriteMemoryStream> || std::is_same_v<T, WriteFileStream>;

    static constexpr auto k_block_size = 4_size;

    static constexpr auto k_string_null = 4_size;

    static constexpr auto k_block_width = 32_size;

    static constexpr auto k_chunk_width = 32_size;

    static constexpr auto k_padding_unit_size = 0x1000_size;

    static constexpr auto k_none_size = 0_size;

    static constexpr auto k_begin_index = 0_size;

    static constexpr auto k_string_empty = ""_sv;

    static auto get_iv_from_key(String const &key) -> String
    {
        assert_conditional(key.size() == 32_size, "Key must have 32 characters", "get_iv_from_key");
        return key.sub(4_size, 28_size);
    }

    template <typename Type> requires std::is_arithmetic_v<Type>
    static constexpr auto value_max() -> Type
    {
        return std::numeric_limits<Type>::max();
    }

    template <typename Type> requires std::is_arithmetic_v<Type>
    static constexpr auto value_min() -> Type
    {
        return std::numeric_limits<Type>::min();
    }

    template <typename Type> requires std::is_arithmetic_v<Type>
    static constexpr auto type_bit_count() -> size_t
    {
        return sizeof(Type) * 8_size;
    }

    template <typename Type>
    static auto compare_value_to_reset(Optional<Type>& data, Type const &value)
    {
        if (*data == value)
        {
            data.reset();
        }
    }

    static auto compare_value_to_reset(Optional<String>& data, std::string_view const value)
    {
        if (*data == value)
        {
            data.reset();
        }
    }


    template <is_integer Type>
    static constexpr auto log2n(Type const &value) -> Type {
       return static_cast<Type>(1 << static_cast<Type>((std::log2(value) + 0.5)));
    }

    template <typename Size, is_not_void Type, is_exchanger Exchanger, read_stream ReadStream> requires std::is_void_v<Size> || is_integer<Size>
    static auto exchange_list(Exchanger const &exchanger, ReadStream &stream, List<Type> &value) -> void {
          if constexpr (!std::is_void_v<Size>) {
              value.resize(static_cast<size_t>(stream.template read_impl<Size>()));
          }
          for (auto &element : value) {
              exchanger(stream, element);
          }
    }

    template <typename Size, is_not_void Type, is_exchanger Exchanger, write_stream WriteStream> requires std::is_void_v<Size> || is_integer<Size>
    static auto exchange_list(Exchanger const &exchanger, WriteStream &stream, List<Type> const &value) -> void {
        if constexpr (!std::is_void_v<Size>) {
            stream.template write<Size>(static_cast<Size>(value.size()));
        }
        for (auto &element : value) {
            exchanger(stream, element);
        }
    }

    template <typename Raw, typename Type, read_stream ReadStream> requires std::is_arithmetic_v<Raw> && std::is_enum_v<Type>
    static auto exchange_enumeration(ReadStream &stream, Type &value) -> void {
        using underlying_type = std::underlying_type_t<Type>;
        auto index = stream.template read_impl<Raw>();
        const auto is_valid_enum = index >= static_cast<Raw>(value_min<underlying_type>()) && index <= static_cast<Raw>(static_cast<Type>(value_max<underlying_type>()));
        assert_conditional(is_valid_enum, fmt::format("Failed to convert enum {} by value {}", VAR_TO_STRING(Type), index), "exchange_enumeration");
        value = static_cast<Type>(index);
    }

    template <typename Raw, typename Type, write_stream WriteStream> requires std::is_arithmetic_v<Raw> && std::is_enum_v<Type>
    static auto exchange_enumeration(WriteStream &stream, Type const &value) -> void {
        using underlying_type = std::underlying_type_t<Type>;
        const auto index = static_cast<Raw>(value);
        const auto is_valid_enum = index >= static_cast<Raw>(value_min<underlying_type>()) && index <= static_cast<Raw>(static_cast<Type>(value_max<underlying_type>()));
        assert_conditional(is_valid_enum, fmt::format("Failed to convert enum {} by value {}", VAR_TO_STRING(Type), index), "exchange_enumeration");
        stream.template write<Raw>(index);
    }

    template <typename Raw, typename Type, read_stream ReadStream> requires std::is_arithmetic_v<Raw> && std::is_arithmetic_v<Type>
    static auto exchange_number_fixed(ReadStream &stream, Type &value) -> void {
        value = static_cast<Type>(stream.template read_impl<Raw>());
    }

    template <typename Raw, typename Type, write_stream WriteStream> requires std::is_arithmetic_v<Raw> && std::is_arithmetic_v<Type>
    static auto exchange_number_fixed(WriteStream &stream, Type const &value) -> void {
        stream.template write<Raw>(static_cast<Raw>(value));
    }

    template <typename Raw, typename Type, read_stream ReadStream> requires std::is_arithmetic_v<Raw> && std::is_arithmetic_v<Type>
    static auto exchange_number_fixed(ReadStream &stream, Optional<Type> &value) -> void {
        value = Type{};
        exchange_number_fixed<Raw, Type>(stream, *value);
    }

    template <typename Raw, typename Type, write_stream WriteStream> requires std::is_arithmetic_v<Raw> && std::is_arithmetic_v<Type>
    static auto exchange_number_fixed(WriteStream &stream, Optional<Type> const &value) -> void {
        auto c_value = value.has_value() ? *value : Type{};
        exchange_number_fixed<Raw, Type>(stream, c_value);
    }

    template <is_integer Size, is_list Type, read_stream ReadStream>
    static auto exchange_list_size(ReadStream &stream, Type &value) -> void {
        value.resize(static_cast<size_t>(stream.template read_impl<Size>()));
    }

    template <is_integer Size, is_list Type, write_stream WriteStream>
    static auto exchange_list_size(WriteStream &stream, Type const &value) -> void {
        stream.template write<Size>(static_cast<Size>(value.size()));
    }

    template <typename Type, read_stream ReadStream> requires std::is_arithmetic_v<Type>
    static auto exchange_number(ReadStream &stream, Type &value) -> void {
        value = stream.template read_impl<Type>();
    }

    template <typename Type, write_stream WriteStream> requires std::is_arithmetic_v<Type>
    static auto exchange_number(WriteStream &stream, Type const &value) -> void {
        stream.template write<Type>(static_cast<Type>(value));
    }

    template <is_integer Type, read_stream ReadStream>
    static auto exchange_unicode_fixed(ReadStream &stream, String &value) -> void {
        value = stream.string(sizeof(Type));
    }

    template <is_integer Type, write_stream WriteStream>
    static auto exchange_unicode_fixed(WriteStream &stream, String const &value) -> void {
        assert_conditional(value.size() == sizeof(Type), "Unicode must be 2 size", "exchange_unicode_fixed");
        stream.string(value);
    }

    template <is_integer Type, read_stream ReadStream>
    static auto exchange_string_block(ReadStream &stream, String &value) -> void {
        value = stream.template string_of<Type>();
    }

    template <is_integer Type, write_stream WriteStream>
    static auto exchange_string_block(WriteStream &stream, String const &value) -> void {
        stream.template string_of<Type>(value);
    }

    template <is_integer Type, read_stream ReadStream>
    static auto exchange_string_block(ReadStream &stream, Optional<String> &value) -> void {
        value = String{};
        exchange_string_block<Type>(stream, *value);
    }

    template <is_integer Type, write_stream WriteStream>
    static auto exchange_string_block(WriteStream &stream, Optional<String> const &value) -> void {
        auto c_value = value.has_value() ? *value : String{};
        exchange_string_block<Type>(stream, c_value);
    }

    template <read_stream ReadStream>
    static auto exchange_boolean(ReadStream &stream, bool &value) -> void {
        value = stream.boolean();
    }

    template <write_stream WriteStream>
    static auto exchange_boolean(WriteStream &stream, bool const &value) -> void {
        stream.boolean(value);
    }

    template <read_stream ReadStream>
    static auto exchange_boolean(ReadStream &stream, Optional<bool> &value) -> void {
        value = false;
        exchange_boolean(stream, *value);
    }

    template <write_stream WriteStream>
    static auto exchange_boolean(WriteStream &stream, Optional<bool> const &value) -> void {
        auto c_value = value.has_value() ? *value : false;
        exchange_boolean(stream, c_value);
    }

    template <typename Type> requires FileSystem::Detail::is_buffer_container_v<Type>
    static auto exchange_identifier_block(Type &data, const std::string_view value) -> bool {
        for (const auto i : Range{value.size()})
        {
            if (static_cast<char>(data[i]) != value[i])
            {
                return false;
            }
        }
        return true;
    }

    template <typename Type> requires FileSystem::Detail::is_buffer_container_v<Type>
    static auto exchange_identifier_block(Type &data, const std::string_view value) -> void {
        for (const auto i : Range{value.size()})
        {
            data[i] = value[i];
        }
    }

    /*
    static auto exchange_identifier_block(ReadMemoryStream &stream, const std::string_view value) -> bool {
       return stream.string(value.size()) == value;
    }

    static auto exchange_identifier_block(WriteStream &stream, const std::string_view value) -> void {
        stream.string(String{value});
    }
    */

    template <auto rate, is_integer Type, read_stream ReadStream> requires std::is_arithmetic_v<Type>
    static auto exchange_floater_with_rate(ReadStream &stream, double &value) -> void {
        static_assert(sizeof(rate) == sizeof(float), "Rate size must be size of float");
        value = static_cast<double>(stream.template read_impl<Type>() / rate);
    }

    template <auto rate, is_integer Type, write_stream WriteStream>
    static auto exchange_floater_with_rate(WriteStream &stream, double const &value) -> void {
        static_assert(sizeof(rate) == sizeof(float), "Rate size must be size of float");
        stream.template write<Type>(static_cast<Type>(std::round(value * rate)));
    }

    template <is_integer Type, read_stream ReadStream>
    static auto exchange_raw_constant(ReadStream &stream, Type const&value) -> void
    {
        auto v = Type{};
        exchange_number_fixed<Type>(stream, v);
        assert_conditional(v == value, "exchange_raw_constant failed", "exchange_raw_constant"); //TODO
    }

    template <is_integer Type, write_stream WriteStream>
    static auto exchange_raw_constant(WriteStream &stream, Type const&value) -> void
    {
        exchange_number_fixed<Type>(stream, value);
    }

    template <read_stream ReadStream>
    static auto exchange_null_block(ReadStream &stream, size_t const&size) -> void
    {
        stream += size;
    }

    template <write_stream WriteStream>
    static auto exchange_null_block(WriteStream &stream, size_t const&size) -> void
    {
        stream.null(size);
    }

    template <typename T, typename E> requires std::is_base_of_v<BaseContainer<extract_container_t<E>>, T>
    static auto contain(T &data, E const &value) -> bool {
       return  std::find(data.begin(), data.end(), value) != data.end();
    }

    template <is_integer Type, read_stream ReadStream, typename ...Args>requires (std::is_same_v<Args, String> && ...)
    static auto exchange_mutil_by_string_block(ReadStream &stream, const std::string_view& delimiter, Args & ...args) -> void{
        static_assert(sizeof...(Args) > 0_size, "Args size must higher 0");
        auto string = stream.template string_of<Type>();
        auto list = List<String>{};
        StringHelper::split_string(string, delimiter, list);
        assert_conditional(sizeof...(args) == list.size(), "Wrong number of arguments", "exchange_mutil_by_string_block"); //TODO: add locale
        auto view = list | std::views::transform([](String const &part) { return part; });
        auto index = k_begin_index;
        ((args = view[index++]), ...);
    }

    template <is_integer Type, write_stream WriteStream, typename ...Args>requires (std::is_same_v<Args, String> && ...)
    static auto exchange_mutil_by_string_block(WriteStream &stream, const std::string_view delimiter, Args const & ...args) -> void{
        constexpr auto N = sizeof...(Args);
        static_assert(N > 0_size, "Args size must higher 0");
        auto delimiter_string = String{delimiter};
        auto string = String{};
        auto index = k_begin_index;
        ((string += index++ < N - 1 ? (args + delimiter_string) : args), ...);
        stream.template string_of<Type>(string);
    }
 }