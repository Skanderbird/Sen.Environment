#pragma once

#include "kernel/definition/utility.hpp"
#include "kernel/support/popcap/resource_stream_group/definition.hpp"

namespace Sen::Kernel::Support::Miscellaneous::Shared
{
    using namespace Sen::Kernel::Support::PopCap::ResourceStreamGroup;

    inline static constexpr auto k_block_size = 4_size;

    inline static constexpr auto k_string_null = ""_sv;

    inline static constexpr auto k_padding_unit_size = 0x1000_size;

    inline static constexpr auto k_none_size = 0_size;

    inline static constexpr auto k_begin_index = 0_size;

    inline auto sub_bytes(
        std::vector<uint8_t> const &data,
        size_t const &begin,
        size_t const &size) -> std::vector<uint8_t>
    {
        assert(begin + size <= data.size());
        auto bytes = std::vector<std::uint8_t>{};
        bytes.assign(data.begin() + begin, data.begin() + begin + size);
        return bytes;
    }

    inline auto write_image(
        std::string const &destination,
        Definition::Image<int> const &data) -> void
    {
        FileSystem::create_directory(Path::getParents(destination));
        ImageIO::write_png(destination, data);
        return;
    }

    inline auto write_bytes(
        std::string const &destination,
        std::vector<uint8_t> const &data) -> void
    {
        FileSystem::create_directory(Path::getParents(destination));
        FileSystem::write_binary(destination, data);
        return;
    }

    inline static auto write_json(
        std::string const &destination,
        nlohmann::ordered_json const &content) -> void
    {
        FileSystem::create_directory(Path::getParents(destination));
        FileSystem::write_json(destination, content);
    }

    inline static auto write_xml(
        std::string const &destination,
        tinyxml2::XMLDocument &content) -> void
    {
        FileSystem::create_directory(Path::getParents(destination));
        FileSystem::write_xml(destination, &content);
    }

    inline auto write_text(
        std::string const &destination,
        std::string const &content) -> void
    {
        FileSystem::create_directory(Path::getParents(destination));
        FileSystem::write_file(destination, content);
        return;
    }

    inline auto toupper(
        std::string &str) -> void
    {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return;
    }

    inline auto toupper_back(
        std::string const &str) -> std::string
    {
        auto upper_str = std::string{str.data(), str.size()};
        std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(), ::toupper);
        return upper_str;
    }

    inline auto compare_string(
        std::string_view a,
        std::string_view b) -> bool
    {
        if (a.size() != b.size())
        {
            return false;
        }
        for (const auto &i : Range(a.size()))
        {
            if (tolower(a[i]) != tolower(b[i]))
            {
                return false;
            }
        }
        return true;
    }

    inline auto compute_utf8_character_extra_size(
        char const &character) -> size_t
    {
        auto extra_size = size_t{};
        if (character < 0b1'0000000)
        {
            extra_size = 0;
        }
        else if (character < 0b11'000000)
        {
            assert("first utf-8 character is valid");
        }
        else if (character < 0b111'00000)
        {
            extra_size = 1;
        }
        else if (character < 0b1111'0000)
        {
            extra_size = 2;
        }
        else if (character < 0b11111'000)
        {
            extra_size = 3;
        }
        else
        {
            assert("first utf-8 character is valid");
        }
        return extra_size;
    }

    template <typename value, auto rate>
        requires std::is_arithmetic_v<value>
    inline static auto exchange_floater_with_rate(
        DataStreamView &stream,
        double &data) -> void
    {
        static_assert(sizeof(rate) == sizeof(float));
        data = static_cast<double>(static_cast<float>(stream.read_of<value>()) / rate);
        return;
    }

    template <typename value, auto rate>
        requires std::is_integral_v<value> || std::is_floating_point_v<value>
    inline static auto exchange_floater_with_rate(
        double const &data,
        DataStreamView &stream) -> void
    {
        static_assert(sizeof(rate) == sizeof(float));
        stream.write_of<value>(static_cast<value>(std::round(static_cast<float>(data) * rate)));
        return;
    }

    template <typename Type, typename Exchanger, typename... Size>
        requires true && (std::is_same<Size, std::size_t>::value && ...) && (!std::is_void_v<Type>) && (!std::is_void_v<Exchanger>) &&
                     (!std::is_reference_v<Exchanger>) && (std::is_same_v<Exchanger, std::remove_cvref_t<Exchanger>>)
    inline static auto exchange_list(
        DataStreamView &stream,
        std::vector<Type> &value,
        Exchanger const &exchanger,
        Size... size) -> void
    {
        static_assert(sizeof...(Size) == 1 || sizeof...(Size) == 0, "Expected 0 or 1 argument only");
        if constexpr (sizeof...(Size) == 1)
        {
            value.resize(std::get<0>(std::make_tuple(size...)));
        }
        for (auto &element : value)
        {
            exchanger(stream, element);
        }
        return;
    }

    template <auto WriteSize, typename SizeType, typename Type, typename Exchanger>
        requires true && (!std::is_void_v<Type>) && (!std::is_void_v<Exchanger>) && (std::is_arithmetic_v<SizeType>) &&
                     (!std::is_reference_v<Exchanger>) && (std::is_same_v<Exchanger, std::remove_cvref_t<Exchanger>>)
    inline static auto exchange_list(
        DataStreamView &stream,
        std::vector<Type> const &value,
        Exchanger const &exchanger) -> void
    {
        static_assert(WriteSize == true || WriteSize == false, "WriteSize must be true or false");
        if constexpr (WriteSize)
        {
            stream.write_of<SizeType>(static_cast<SizeType>(value.size()));
        }
        for (const auto &element : value)
        {
            exchanger(stream, element);
        }
        return;
    }

    namespace CompiledMapData
    {
        inline auto get_common_size(
            std::string const &range_1,
            std::string const &range_2) -> size_t
        {
            auto maximum_common_size = range_1.size() >= range_2.size() ? range_1.size() : range_2.size();
            for (auto index : Range(maximum_common_size))
            {
                if (range_1[index] != range_2[index])
                {
                    return index;
                }
            }
            return maximum_common_size;
        }

        inline auto compute_ripe_size(
            std::map<std::string, Common::ResourceInformation> const &value) -> size_t
        {
            struct WorkOption
            {
                size_t inherit_length;
                bool has;
            };
            auto string_list = std::vector<std::string>{};
            for (auto &element : value)
            {
                string_list.emplace_back(element.first);
            }
            auto work_option = std::vector<WorkOption>(string_list.size());
            if (!string_list.empty())
            {
                work_option[0].inherit_length = k_none_size;
                work_option[0].has = true;
            }
            auto block_count = k_none_size;
            for (auto index : Range(string_list.size()))
            {
                auto &element_key = string_list[index];
                auto a_string_has_child = std::vector<bool>(element_key.size() + 1);
                for (auto index_1 : Range(static_cast<size_t>(index + 1), string_list.size(), size_t{1}))
                {
                    if (!work_option[index_1].has)
                    {
                        auto common_size = get_common_size(element_key, string_list[index_1]);
                        if (!a_string_has_child.at(common_size) && common_size >= work_option[index].inherit_length)
                        {
                            a_string_has_child[common_size] = true;
                            work_option[index_1] = WorkOption{
                                .inherit_length = common_size,
                                .has = true};
                        }
                    }
                }
                auto character_index = k_none_size;
                if (work_option[index].has)
                {
                    character_index += work_option[index].inherit_length;
                    work_option[index].inherit_length = k_none_size;
                    work_option[index].has = false;
                }
                block_count += element_key.size() - character_index + size_t{1};
                auto additional_size = value.at(element_key).read_texture_additional ? Common::information_resource_texture_additional_section_size : Common::information_resource_general_additional_section_size;
                block_count += (additional_size / k_block_size);
            }
            return block_count * k_block_size;
        }

        template <typename ResourceType, typename Exchanger>
            requires true && (!std::is_void_v<ResourceType>) && (!std::is_void_v<Exchanger>) &&
                         (!std::is_reference_v<Exchanger>) && (std::is_same_v<Exchanger, std::remove_cvref_t<Exchanger>>)
        inline auto encode(
            DataStreamView &stream,
            std::map<std::string, ResourceType> const &value,
            Exchanger const &exchanger) -> void
        {
            struct WorkOption
            {
                size_t inherit_length;
                size_t parent_offset;
                bool has;
            };
            auto resource_information_section_offset = stream.write_pos;
            auto string_list = std::vector<std::string>{};
            for (auto &element : value)
            {
                string_list.emplace_back(element.first);
            }
            auto work_option = std::vector<WorkOption>(string_list.size());
            if (!string_list.empty())
            {
                work_option[0] = WorkOption{
                    .inherit_length = k_none_size,
                    .parent_offset = k_begin_index,
                    .has = true};
                for (auto index : Range(string_list.size()))
                {
                    auto &element_key = string_list[index];
                    auto current_string_has_child = std::vector<bool>(element_key.size() + 1);
                    for (auto index_1 : Range(static_cast<size_t>(index + 1), string_list.size(), size_t{1}))
                    {
                        if (!work_option[index_1].has)
                        {
                            auto common_size = get_common_size(element_key, string_list[index_1]);
                            if (!current_string_has_child.at(common_size) && common_size >= work_option[index].inherit_length)
                            {
                                current_string_has_child[common_size] = true;
                                work_option[index_1] = WorkOption{
                                    .inherit_length = common_size,
                                    .parent_offset = (stream.write_pos / k_block_size + (common_size - work_option[index].inherit_length)),
                                    .has = true};
                            }
                        }
                    }
                    auto character_index = k_none_size;
                    if (work_option[index].has)
                    {
                        character_index += work_option[index].inherit_length;
                        auto current_postion = stream.write_pos;
                        auto parent_position = work_option[index].parent_offset * k_block_size;
                        auto composite_value = stream.readUint32(parent_position) | ((current_postion - resource_information_section_offset) / k_block_size) << 8;
                        stream.writeUint32(composite_value, parent_position);
                        stream.write_pos = current_postion;
                        work_option[index] = WorkOption{};
                    }
                    while (character_index < element_key.size())
                    {
                        stream.writeUint8(static_cast<uint8_t>(element_key[character_index]));
                        stream.write_pos += 3;
                        ++character_index;
                    }
                    stream.writeNull(k_block_size);
                    exchanger(stream, value.at(element_key));
                }
            }
            // stream.write_pos = before_postion;
            return;
        }

        template <typename ResourceType, typename Exchanger>
            requires true && (!std::is_void_v<ResourceType>) && (!std::is_void_v<Exchanger>) &&
                         (!std::is_reference_v<Exchanger>) && (std::is_same_v<Exchanger, std::remove_cvref_t<Exchanger>>)
        inline auto decode(
            DataStreamView &stream,
            size_t const &resource_information_section_offset,
            size_t const &resource_information_section_size,
            std::map<std::string, ResourceType> &value,
            Exchanger const &exchanger) -> void
        {
            auto before_pos = stream.read_pos;
            stream.read_pos = resource_information_section_offset;
            auto offset_limit = resource_information_section_offset + resource_information_section_size;
            auto parent_string = std::map<size_t, std::string>{};
            while (stream.read_pos < offset_limit)
            {
                auto key = std::string{};
                auto position = static_cast<size_t>((stream.read_pos - resource_information_section_offset) / k_block_size);
                if (parent_string.find(position) != parent_string.end())
                {
                    key += parent_string[position];
                    parent_string.erase(position);
                }
                while (true)
                {
                    auto current_character = stream.readUint8();
                    auto child_string_offset = stream.readUint24();
                    if (child_string_offset != uint32_t{0})
                    {
                        parent_string[static_cast<size_t>(child_string_offset)] = key;
                    }
                    if (current_character == 0)
                    {
                        break;
                    }
                    key += static_cast<char>(current_character);
                }
                if (key.empty())
                {
                    break;
                }
                //
                auto resource_information = ResourceType{};
                exchanger(stream, resource_information);
                value.emplace(key, resource_information);
            }
            stream.read_pos = before_pos;
            return;
        }
    }
};