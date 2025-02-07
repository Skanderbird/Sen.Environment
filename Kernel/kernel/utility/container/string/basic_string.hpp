#pragma once

#include "common.hpp"
#include "kernel/utility/macro.hpp"
#include "kernel/utility/algorithm/pointer.hpp"
#include "kernel/utility/container/array/byte_array.hpp"
#include "kernel/utility/container/list/list.hpp"

namespace Sen::Kernel {

	class BasicString;

	using String = BasicString;

    class BasicString : public BaseContainer<char> {

	public:

		using Size = std::size_t;

		template <typename U>
		using Pointer = U*;

		using Character = char;

		using Iterator = Pointer<Character>;

    	template <typename T>
    	using List = CList<T>;

		inline static auto constexpr none = static_cast<size_t>(-1);

    	usize _capacity;

	public:

		constexpr explicit BasicString(

		) = default;

		constexpr explicit BasicString(
			const Size& size
		) : BaseContainer<char>{new Character[size + 1], size}, _capacity{size + 1}
		{
			thiz.value[size] = '\0';
		}

		explicit BasicString(
			const char* data
		) : BaseContainer<char>{nullptr, std::strlen(data)}, _capacity{}
		{
			thiz._capacity = thiz._size + 1;
			thiz.value = new Character[thiz._capacity];
			thiz.value[thiz._size] = '\0';
			std::memcpy(thiz.value, data, thiz._size);
		}

    	explicit BasicString(
			char* data,
			const Size& size
		) : BaseContainer<char>{ new Character[size + 1], size }, _capacity{size + 1}
		{
			thiz.value[thiz._size] = '\0';
			std::memcpy(thiz.value, data, thiz._size);
		}

		explicit BasicString(
			const char* data,
			const Size& size
		) : BaseContainer<char>{ new Character[size + 1], size }, _capacity{size + 1}
		{
			thiz.value[thiz._size] = '\0';
			std::memcpy(thiz.value, data, thiz._size);
		}

		auto allocate(
			Size const& size
		) -> void override
		{
			if (thiz.value != nullptr) {
				delete[] thiz.value;
			}
			thiz._capacity = size + 1;
			thiz.value = new Character[thiz._capacity];
			std::memset(thiz.value, 0, thiz._capacity);
		}

    	auto reallocate(
			Size const& size
		) -> void
		{
			thiz._capacity = size + 1;
			auto new_value = new Character[thiz._capacity];
			std::memcpy(new_value, thiz.value, thiz._size);
			delete[] thiz.value;
			thiz.value = new_value;
			std::memset(thiz.value + thiz._size, 0, thiz._capacity - thiz._size);
		}

		~BasicString(

		) override
		{
			if (thiz.value != nullptr) {
				delete[] value;
				thiz.value = nullptr;
			}
		}

    	BasicString(
			const BasicString& other
		) {
			thiz.value = new Character[other._capacity];
			thiz._size = other._size;
			thiz._capacity = other._capacity;
			std::memcpy(thiz.value, other.value, other._size);
			thiz.value[thiz._size] = '\0';
		}

		auto operator =(
			const BasicString& other
		) -> BasicString & {
			if (thiz.value != nullptr) {
				delete[] thiz.value;
			}
			thiz._capacity = other._capacity;
			thiz.value = new Character[thiz._capacity];
			thiz._size = other._size;
			std::memcpy(thiz.value, other.value, other._size);
			thiz.value[thiz._size] = '\0';
			return thiz;
		}

    	BasicString(
			BasicString&& other
		) noexcept : BaseContainer{}, _capacity{} {
			thiz = as_move(other);
		}

		constexpr auto operator=(
			BasicString&& other
		) noexcept -> BasicString& {
			if (this != &other) {
				delete[] thiz.value;
				thiz._size = other._size;
				thiz.value = other.value;
				thiz._capacity = other._capacity;
				other.value = nullptr;
				other._size = 0;
				other._capacity = 0;
			}
			return thiz;
		}

    	constexpr auto operator [](
			Size const& index
		) const -> Character
    	{
    		return thiz.value[index];
    	}

    	constexpr auto operator [](
			Size const& index
		) -> Character& override
    	{
    		return thiz.value[index];
    	}

		constexpr auto operator ==(
			const BasicString &other
		) const -> bool {
			if (thiz._size != other._size) {
				return false;
			}
			return std::memcmp(thiz.value, other.value, thiz._size) == 0;
		}

		constexpr auto operator != (
			const BasicString& other
		) const -> bool
		{
			return !(thiz.operator==(other));
		}

		auto operator < (
			const BasicString& other
		) -> bool const = delete;

		auto operator > (
			const BasicString& other
		) = delete;

		auto operator <= (
			const BasicString& other
		) -> bool const = delete;

		auto operator >= (
			const BasicString& other
		) -> bool const = delete;

		constexpr auto size(

		) const -> Size override {
			return thiz._size;
		}

		friend auto operator <<(std::ostream &os, const BasicString& other) -> std::ostream& {
			if (other.value != nullptr) {
				os.write(other.value, other._size);
			}
			return os;
		}

		constexpr auto string(

		) const -> std::string
		{
			return std::string{ thiz.value, thiz._size };
		}

		auto wstring(

		) const -> std::wstring
		{
			#if _WIN32
			auto size = static_cast<size_t>(MultiByteToWideChar(CP_UTF8, 0, thiz.value, static_cast<int>(thiz._size), nullptr, 0));
			auto destination = std::wstring{};
			destination.resize(size);
			MultiByteToWideChar(CP_UTF8, 0, thiz.value, static_cast<int>(thiz._size), &destination[0], size);
			return destination;
			#else
			auto convert = std::wstring_convert<std::codecvt_utf8<wchar_t>>{};
			return convert.from_bytes(thiz.value, thiz.value + thiz._size);
			#endif
		}

    	auto warray (
    	) const -> CArray<wchar_t> {
			#if _WIN32
			auto size = static_cast<size_t>(MultiByteToWideChar(CP_UTF8, 0, thiz.value, static_cast<int>(thiz._size), nullptr, 0));
			auto destination = CArray<wchar_t>{size};
			MultiByteToWideChar(CP_UTF8, 0, thiz.value, static_cast<int>(thiz._size), &destination[0], size);
			return destination;
			#else
			auto convert = std::wstring_convert<std::codecvt_utf8<wchar_t>>{};
			auto wstr = convert.from_bytes(thiz.value, thiz.value + thiz._size);
			auto destination = CArray<wchar_t>{wstr.size()};
			std::memcpy(destination.cbegin(), wstr.cbegin(), wstr.size() * sizeof(wchar_t));
			return destination;
			auto convert = std::wstring_convert<std::codecvt_utf8<wchar_t>>{};
			auto wstr = convert.from_bytes(thiz.value, thiz.value + thiz._size);
			auto destination = CArray<wchar_t>{wstr.size()};
			std::memcpy(destination.cbegin(), wstr.data(), wstr.size() * sizeof(wchar_t));
			return destination;
			#endif
		}

		constexpr operator std::string_view(
		) const {
			return std::string_view{ thiz.value, thiz._size };
		}

		constexpr auto length(

		) const -> Size
		{
			return thiz._size;
		}

    	constexpr auto take_ownership (
			BasicString& other
		) -> void {
			thiz.value = other.value;
			thiz._size = other._size;
			thiz._capacity = other._capacity;
			other.value = nullptr;
			other._size = 0;
			other._capacity = 0;
		}

		virtual auto insert(
			const Size& index,
			Pointer<const char> str,
			const Size& len
		) -> void {
			assert_conditional(index < thiz._size, fmt::format("Inserted index is larger than the size of the string"), "insert");
			auto new_size = thiz._size + len;
			if (new_size <= thiz._capacity) {
				std::memmove(thiz.value + index + len, thiz.value + index, thiz._size - index);
				std::memcpy(thiz.value + index, str, len);
			} else {
				auto new_capacity = std::max(new_size, thiz._capacity + 4096_size);
				auto raw = new Character[new_capacity + 1];
				std::memcpy(raw, thiz.value, index);
				std::memcpy(raw + index, str, len);
				std::memcpy(raw + index + len, thiz.value + index, thiz._size - index);
				raw[new_size] = '\0';
				delete[] thiz.value;
				thiz.value = raw;
				thiz._capacity = new_capacity;
			}
			thiz._size = new_size;
			thiz.value[new_size] = '\0';
		}

		virtual auto insert(
			const Size& index,
			const Character& ch
		) -> void {
			auto temporary = std::array<char, 1>{ ch };
			return insert(index, temporary.data(), temporary.size());
		}

		virtual auto insert(
			const Size& index,
			const BasicString& other
		) -> void {
			return insert(index, other.value, other._size);
		}

		virtual auto erase(
			const Size& index,
			const Size& len
		) -> BasicString& {
			assert_conditional(index < thiz._size, fmt::format("Erased index is larger than the size of the string"), "erase");
			return erase(thiz.value + index, thiz.value + std::min(index + len, thiz._size));
		}

		virtual auto erase(
			Iterator first,
			Iterator last
		) -> BasicString& {
			assert_conditional(first >= thiz.value && first <= thiz.end(), fmt::format("Erase begin iterator is outside bounds of the String iterator"), "erase");
			assert_conditional(last >= thiz.value && last <= thiz.end(), fmt::format("Erase end iterator is outside bounds of the String iterator"), "erase");
			assert_conditional(first <= last, fmt::format("Cannot erase if the first iterator is larger than last iterator"), "erase");
			auto erase_len = static_cast<Size>(last - first);
			auto new_size = thiz._size - erase_len;
			std::memcpy(first, last, thiz.end() - last);
			thiz._size = new_size;
			thiz.value[thiz._size] = '\0';
			return thiz;
		}

    	virtual auto push(const Character& c) -> void {
			auto new_size = thiz._size + 1;
			if (new_size <= thiz._capacity) {
				thiz.value[thiz._size] = c;
			} else {
				auto new_capacity = std::max(new_size, thiz._capacity + 4096_size);
				auto new_value = new Character[new_capacity + 1];
				std::memcpy(new_value, thiz.value, thiz._size);
				new_value[thiz._size] = c;
				delete[] thiz.value;
				thiz.value = new_value;
				thiz._capacity = new_capacity;
			}
			thiz._size = new_size;
			thiz.value[new_size] = '\0';
		}


		virtual constexpr auto pop(
		) -> void {
			if (thiz._size > 0) {
				--thiz._size;
				thiz.value[thiz._size] = '\0';
			}
		}

    	virtual auto append(
    		const Size& count,
    		Character ch
    	) -> BasicString& {
			auto new_size = thiz._size + count;
			if (new_size <= thiz._capacity) {
				std::memset(thiz.value + thiz._size, ch, count);
			} else {
				auto new_capacity = std::max(new_size, thiz._capacity + 4096_size);
				auto new_value = new Character[new_capacity + 1];
				std::memcpy(new_value, thiz.value, thiz._size);
				std::memset(new_value + thiz._size, ch, count);
				delete[] thiz.value;
				thiz.value = new_value;
				thiz._capacity = new_capacity;
			}
			thiz._size = new_size;
			thiz.value[new_size] = '\0';
			return thiz;
		}


    	virtual auto append(
    		Pointer<const Character> s,
    		const Size& count
    	) -> BasicString& {
			auto new_size = thiz._size + count;
			if (new_size <= thiz._capacity) {
				std::memcpy(thiz.value + thiz._size, s, count);
			} else {
				auto new_capacity = std::max(new_size, thiz._capacity + 4096_size);
				auto new_value = new Character[new_capacity + 1];
				std::memcpy(new_value, thiz.value, thiz._size);
				std::memcpy(new_value + thiz._size, s, count);

				delete[] thiz.value;
				thiz.value = new_value;
				thiz._capacity = new_capacity;
			}
			thiz._size = new_size;
			thiz.value[new_size] = '\0';
			return thiz;
		}


		virtual auto append(
			Pointer<const Character> s
		) -> BasicString& {
			return append(s, std::strlen(s));
		}

		auto append(
			const BasicString& str
		) -> BasicString& {
			return append(str.value, str._size);
		}

		auto append(
			const BasicString& str,
			const Size& pos,
			const Size& count = none
		) -> BasicString& {
			assert_conditional(pos <= str._size, fmt::format("Cannot append if the position is larger than the size of the string. The position: {}, String size: {}", pos, str._size), "append");
			return append(str.value + pos, std::min(count, str._size - pos));
		}

		template <typename Iterate>
		auto append(
			const Iterate& first,
			const Iterate& last
		) -> BasicString& {
			auto count = last - first;
			return append(&(*first), count);
		}

		virtual auto append(
			std::initializer_list<Character> ilist
		) -> BasicString& {
			return append(ilist.begin(), ilist.size());
		}

		auto replace(
			const Size& pos,
			const Size& count,
			const BasicString& str
		) -> BasicString& {
			return replace(pos, count, str.value, str._size);
		}

		auto replace(
			Pointer<Character> first,
			Pointer<Character> last,
			const BasicString& str
		) -> BasicString& {
			return replace(first - value, last - first, str.value, str._size);
		}

		auto replace(
			const Size& pos,
			const Size& count,
			const BasicString& str,
			const Size& pos2,
			const Size& count2 = none
		) -> BasicString& {
			assert_conditional(pos2 <= str._size, fmt::format("Cannot replace if the position is larger than the size of the string. The position: {}, String size: {}", pos2, str._size), "replace");
			return replace(pos, count, str.value + pos2, std::min(count2, str._size - pos2));
		}

    	virtual auto replace(
			const Size& pos,
			const Size& count,
			Pointer<const Character> cstr,
			const Size& count2
		) -> BasicString& {
			auto new_size = thiz._size - count + count2;
			if (new_size <= thiz._capacity) {
				if (count2 != count) {
					std::memmove(thiz.value + pos + count2, thiz.value + pos + count, thiz._size - pos - count);
				}
				std::memcpy(thiz.value + pos, cstr, count2);
			} else {
				auto new_capacity = std::max(new_size, thiz._capacity + 4096_size);
				auto new_value = new Character[new_capacity + 1];
				std::memcpy(new_value, thiz.value, pos);
				std::memcpy(new_value + pos, cstr, count2);
				std::memcpy(new_value + pos + count2, thiz.value + pos + count, thiz._size - pos - count);
				delete[] thiz.value;
				thiz.value = new_value;
				thiz._capacity = new_capacity;
			}
			thiz._size = new_size;
			thiz.value[new_size] = '\0';
			return thiz;
		}


		virtual auto replace(
			Pointer<Character> first,
			Pointer<Character> last,
			Pointer<const Character> cstr,
			const Size& count2
		) -> BasicString& {
			return replace(first - value, last - first, cstr, count2);
		}

		virtual auto replace(
			const Size& pos,
			const Size& count,
			Pointer<const Character> cstr
		) -> BasicString& {
			return replace(pos, count, cstr, std::strlen(cstr));
		}

		auto replace(
			Pointer<Character> first,
			Pointer<Character> last,
			Pointer<const Character> cstr
		) -> BasicString& {
			return replace(first - thiz.value, last - first, cstr, std::strlen(cstr));
		}

		virtual auto replace(
			const Size& pos,
			const Size& count,
			const Size& count2,
			Character ch
		) -> BasicString& {
			auto new_str = BasicString{ count2 };
			std::memset(new_str.value, ch, count2);
			return replace(pos, count, new_str);
		}

		virtual auto replace(
			Pointer<Character> first,
			Pointer<Character> last,
			const Size& count2,
			Character ch
		) -> BasicString& {
			return replace(first - value, last - first, count2, ch);
		}

		template <typename It>
		auto replace(
			Pointer<Character> first,
			Pointer<Character> last,
			It first2,
			It last2
		) -> BasicString& {
			return replace(first - value, last - first, &(*first2), last2 - first2);
		}

		virtual auto replace(
			Pointer<Character> first,
			Pointer<Character> last,
			std::initializer_list<Character> ilist
		) -> BasicString& {
			return replace(first - value, last - first, ilist.begin(), ilist.size());
		}

		auto copy(
			Pointer<Character> source,
			const Size& size
		) const -> void
		{
			std::memcpy(source, thiz.value, size);
		}

		auto operator+=(
			const BasicString& other
		) -> BasicString& {
		    auto new_size = thiz._size + other._size;
		    if (new_size <= thiz._capacity) {
		        std::memcpy(thiz.value + thiz._size, other.value, other._size);
		    } else {
		        auto new_capacity = std::max(new_size, thiz._capacity + 4096_size);
		        auto new_value = new Character[new_capacity + 1];
		        std::memcpy(new_value, thiz.value, thiz._size);
		        std::memcpy(new_value + thiz._size, other.value, other._size);
		        delete[] thiz.value;
		        thiz.value = new_value;
		        thiz._capacity = new_capacity;
		    }
		    thiz._size = new_size;
		    thiz.value[new_size] = '\0';
		    return thiz;
		}

		virtual auto operator+=(
			const char* other
		) -> BasicString& {
		    auto other_size = std::strlen(other);
		    auto new_size = thiz._size + other_size;
		    if (new_size <= thiz._capacity) {
		        std::memcpy(thiz.value + thiz._size, other, other_size);
		    } else {
		        auto new_capacity = std::max(new_size, thiz._capacity + 4096_size);
		        auto new_value = new Character[new_capacity + 1];
		        std::memcpy(new_value, thiz.value, thiz._size);
		        std::memcpy(new_value + thiz._size, other, other_size);
		        delete[] thiz.value;
		        thiz.value = new_value;
		        thiz._capacity = new_capacity;
		    }
		    thiz._size = new_size;
		    thiz.value[new_size] = '\0';
		    return thiz;
		}

		virtual auto operator+=(
			Character other
		) -> BasicString& {
		    auto new_size = thiz._size + 1;
		    if (new_size <= thiz._capacity) {
		        thiz.value[thiz._size] = other;
		    } else {
		        auto new_capacity = std::max(new_size, thiz._capacity + 4096_size);
		        auto new_value = new Character[new_capacity + 1];
		        std::memcpy(new_value, thiz.value, thiz._size);
		        new_value[thiz._size] = other;
		        delete[] thiz.value;
		        thiz.value = new_value;
		        thiz._capacity = new_capacity;
		    }
		    thiz._size = new_size;
		    thiz.value[new_size] = '\0';
		    return thiz;
		}

		virtual auto operator+=(
			std::initializer_list<Character> ilist
		) -> BasicString& {
		    auto new_size = thiz._size + ilist.size();
		    if (new_size <= thiz._capacity) {
		        std::memcpy(thiz.value + thiz._size, ilist.begin(), ilist.size());
		    } else {
		        auto new_capacity = std::max(new_size, thiz._capacity + 4096_size);
		        auto new_value = new Character[new_capacity + 1];
		        std::memcpy(new_value, thiz.value, thiz._size);
		        std::memcpy(new_value + thiz._size, ilist.begin(), ilist.size());
		        delete[] thiz.value;
		        thiz.value = new_value;
		        thiz._capacity = new_capacity;
		    }
		    thiz._size = new_size;
		    thiz.value[new_size] = '\0';
		    return thiz;
		}

		template <typename StringViewLike>
		requires requires(StringViewLike str) {
		    { str.data() } -> std::same_as<Pointer<const char>>;
		    { str.size() } -> std::same_as<Size>;
		}
		auto operator+=(
			const StringViewLike& str
		) -> BasicString& {
		    auto new_size = thiz._size + str.size();
		    if (new_size <= thiz._capacity) {
		        std::memcpy(thiz.value + thiz._size, str.data(), str.size());
		    } else {
		        auto new_capacity = std::max(new_size, thiz._capacity + 4096_size);
		        auto new_value = new Character[new_capacity + 1];
		        std::memcpy(new_value, thiz.value, thiz._size);
		        std::memcpy(new_value + thiz._size, str.data(), str.size());
		        delete[] thiz.value;
		        thiz.value = new_value;
		        thiz._capacity = new_capacity;
		    }
		    thiz._size = new_size;
		    thiz.value[new_size] = '\0';
		    return thiz;
		}

    	auto view (
    	) const -> std::string_view {
			return std::string_view{ thiz.value, thiz._size };
		}

    	auto sub (
    		const Size& from,
			const Size& to
		) const -> String {
			assert_conditional(to <= thiz._size, "To index must be smaller than string size", "substring");
			assert_conditional(from <= to, "From index must be smaller than To index", "substring");
			auto result = String{to - from};
			std::memcpy(result.value, thiz.value, to - from);
			return result;
		}

    	auto sub (
			const Size& from
		) const -> String {
			return sub(from, thiz._size - from);
		}

		virtual auto substring (
			const Size& from,
			const Size& to
		) -> void {
			assert_conditional(to <= thiz._size, "To index must be smaller than string size", "substring");
			assert_conditional(from <= to, "From index must be smaller than To index", "substring");
			auto result = new Character[to - from];
			std::memcpy(result, thiz.value, to - from);
			delete[] thiz.value;
			thiz.value = result;
			thiz._size = to - from;
		}

		virtual auto substring (
			const Size& from
		) -> void {
			return thiz.substring(from, thiz._size - from);
		}

    protected:

		static auto build_longest_prefab_suffix(
			const char* pattern,
			const usize& m,
			SizeArray& lps
		) -> void {
    		auto len = 0_size;
    		auto i = 1_size;
    		while (i < m) {
    			if (pattern[i] == pattern[len]) {
    				len++;
    				lps[i] = len;
    				++i;
    			}
    			else {
    				if (len != 0) {
    					len = lps[len - 1];
    				}
    				else {
    					lps[i] = 0;
    					++i;
    				}
    			}
    		}
    	}

    public:

    	auto find (
    		char pattern,
			const Size& pos = 0
    	) const -> Size {
    		for (auto i = pos; i < thiz._size; ++i) {
				if (thiz.value[i] == pattern) {
					return i;
				}
    		}
    		return none;
    	}

    	auto find(
			const char* pattern,
			const size_t& size,
			const Size& pos = 0
		) const -> Size {
    		auto m = size;
    		auto n = thiz._size;
    		if (m == 0_size) return pos;
    		if (n < m || pos >= n) return none;
    		auto lps = SizeArray{m};
    		build_longest_prefab_suffix(pattern, m, lps);
    		auto i = pos;
    		auto j = 0_size;
    		while (i < n) {
    			if (pattern[j] == thiz.value[i]) {
    				++i;
    				++j;
    			}
    			if (j == m) {
    				return i - j;
    			}
    			if (i < n && pattern[j] != thiz.value[i]) {
    				if (j != 0) {
    					j = lps[j - 1];
    				}
    				else {
    					++i;
    				}
    			}
    		}
    		return none;
    	}

    	auto find(
			const String& pattern,
			const Size& pos = 0
		) const -> Size {
    		return find(pattern.value, pattern._size, pos);
    	}

    	auto find(
			const std::string_view& pattern,
			const Size& pos = 0
		) const -> Size {
    		return find(pattern.data(), pattern.size(), pos);
    	}

    	auto find(
			const char* pattern,
			const Size& pos = 0
		) const -> Size {
    		return find(pattern, std::strlen(pattern), pos);
    	}

    	auto rfind(
			const char* pattern,
			const size_t& size,
			const Size& pos = none
		) const -> Size {
    		auto m = size;
    		auto n = thiz._size;
    		if (m == 0_size) return pos;
    		if (n < m) return none;
    		auto bad_character = std::array<size_t, 256>{};
    		for (auto i = 0_size; i < m; ++i) {
    			bad_character[pattern[i]] = i;
    		}
    		auto start = (pos == none) ? n - m : std::min(pos, n - m);
    		for (auto i = start; i >= m - 1; --i) {
    			auto j = m - 1;
    			while (pattern[j] == value[i + j]) {
    				if (j == 0_size) {
    					return i;
    				}
    				--j;
    			}
    			auto shift = m - 1 - bad_character[value[i]];
    			if (shift < 1_size) shift = 1;
    			i -= shift;
    		}
    		return none;
    	}

    	auto rfind(
			const char* pattern,
			const Size& pos = none
		) const -> Size {
			return rfind(pattern, std::strlen(pattern), pos);
    	}

		auto rfind(
			const String& pattern,
			const Size& pos = none
		) const -> Size {
			return rfind(pattern.value, pattern._size, pos);
		}

		auto rfind(
			const std::string_view& pattern,
			const Size& pos = none
		) const -> Size {
			return rfind(pattern.data(), pattern.size(), pos);
		}

    	auto starts_with (
			const char* pattern,
			const Size& m
		) const -> bool {
    		if (m > thiz._size) {
    			return false;
    		}
    		return std::memcmp(thiz.value, pattern, m) == 0;
    	}

    	auto starts_with (
			const std::string_view& value
		) const -> bool {
    		return thiz.starts_with(value.data(), value.size());
    	}

    	auto starts_with (
			const char* value
		) const -> bool {
    		return thiz.starts_with(value, std::strlen(value));
    	}

    	auto starts_with (
			const String& value
		) const -> bool {
    		return thiz.starts_with(value.cbegin(), value.size());
    	}

    	auto ends_with (
			const char* pattern,
			const Size& m
		) const -> bool {
    		if (m > thiz._size) {
    			return false;
    		}
    		return std::memcmp(thiz.value + thiz._size - m, pattern, m) == 0;
    	}

    	auto ends_with (
			const std::string_view& value
		) const -> bool {
    		return thiz.ends_with(value.data(), value.size());
    	}

    	auto ends_with (
			const String& value
		) const -> bool {
    		return thiz.ends_with(value.cbegin(), value.size());
    	}

    	auto contains (
    		const String& pattern
    	) const -> bool {
    		return thiz.find(pattern) != none;
    	}

    	auto rcontains (
			const String& pattern
		) const -> bool {
    		return thiz.rfind(pattern) != none;
    	}

		virtual auto trim_left(
    	) -> void {
    		auto index = 0_size;
    		while (index < thiz._size && is_whitespace_character(value[index])) {
    			++index;
    		}
    		if (index > 0) {
    			auto new_size = thiz._size - index;
    			auto new_value = new Character[new_size + 1];
    			std::memcpy(new_value, value + index, new_size + 1);
    			delete[] thiz.value;
    			thiz.value = new_value;
    			thiz._size = new_size;
    		}
    	}

		virtual auto trim_right(

    	) -> void
    	{
    		auto index = thiz._size;
    		while (index > 0 && is_whitespace_character(value[index - 1])) {
    			--index;
    		}
    		if (index < thiz._size) {
    			value[index] = '\0';
    			thiz._size = index;
    		}
    	}

		virtual auto trim (

    	) -> void {
			thiz.trim_right();
			thiz.trim_left();
    	}

    	auto find_all (
    		const char* target,
    		const size_t& length
    	) const -> List<Size>  {
    		auto positions = List<Size>{};
    		if (length == 0 || thiz._size < length) {
    			return positions;
    		}
    		auto pos = 0_size;
    		while ((pos = find(target, length, pos)) != none) {
    			positions.append(pos);
    			pos += length;
    		}
    		return positions;
		}

    	auto find_all (
			const char target
		) const -> List<Size>  {
    		auto positions = List<Size>{};
    		if (thiz._size < 1) {
    			return positions;
    		}
    		auto pos = 0_size;
    		while ((pos = find(target,  pos)) != none) {
    			positions.append(pos);
    			pos += 1;
    		}
    		return positions;
    	}

		auto find_all (
			const std::string_view& target
		) const -> List<Size> {
			return thiz.find_all(target.data(), target.size());
		}

		auto find_all (
			const String& target
		) const -> List<Size> {
			return thiz.find_all(target.cbegin(), target.size());
		}

		auto find_all (
			const char* target
		) const -> List<Size> {
			return thiz.find_all(target, std::strlen(target));
		}

		virtual auto replace_all(
    		const char* target,
			const size_t& target_length,
    		const char* replacement,
			const size_t& replacement_length
		) -> void
    	{
    		if (target_length == 0) {
    			return;
    		}
    		auto positions = thiz.find_all(target);
    		if (positions.empty()) {
    			return;
    		}
    		auto count = positions.size();
    		auto new_length = thiz._size + count * (replacement_length - target_length);
    		if (new_length == thiz._size) {
    			for (auto& pos : positions) {
    				std::memcpy(thiz.value + pos, replacement, replacement_length);
    			}
    		}
    		else {
    			if (new_length > thiz._capacity) {
    				auto new_capacity = std::max(new_length, thiz._capacity * 3);
    				auto new_data = new Character[new_capacity + 1];
    				auto source_index = 0_size;
    				auto destination_index = 0_size;
    				auto position_index = 0_size;
    				while (source_index < thiz._size) {
    					if (position_index < count && source_index == positions[position_index]) {
    						std::memcpy(new_data + destination_index, replacement, replacement_length);
    						source_index += target_length;
    						destination_index += replacement_length;
    						position_index++;
    					} else {
    						new_data[destination_index++] = thiz.value[source_index++];
    					}
    				}
    				new_data[new_length] = '\0';
    				delete[] thiz.value;
    				thiz.value = new_data;
    				thiz._capacity = new_capacity;
    			} else {
    				auto source_index = thiz._size;
    				auto destination_index = new_length;
    				auto position_index = count;
    				while (source_index > 0) {
    					--source_index;
    					--destination_index;
    					if (position_index > 0 && source_index == positions[position_index - 1]) {
    						std::memcpy(thiz.value + destination_index - replacement_length + 1, replacement, replacement_length);
    						destination_index -= replacement_length - 1;
    						source_index -= target_length - 1;
    						--position_index;
    					} else {
    						thiz.value[destination_index] = thiz.value[source_index];
    					}
    				}
    				thiz.value[new_length] = '\0';
    			}
    			thiz._size = new_length;
    		}
    	}

		virtual auto replace_all(
			const std::string_view& target,
			const std::string_view& replacement
		) -> void {
			return thiz.replace_all(target.data(), target.size(), replacement.data(), replacement.size());
    	}

		auto replace_all(
			const String& target,
			const String& replacement
		) -> void {
			return thiz.replace_all(target.cbegin(), target.size(), replacement.cbegin(), replacement.size());
		}

		virtual auto replace_all(
			const char* target,
			const char* replacement
		) -> void {
			return thiz.replace_all(target, std::strlen(target), replacement, std::strlen(replacement));
		}

		virtual auto replace_all(
    		char target,
    		char replacement
		) const -> void
    	{
    		auto positions = thiz.find_all(target);
    		if (positions.empty()) {
    			return;
    		}
    		for (auto& pos : positions) {
    			std::memcpy(thiz.value + pos, &replacement, 1);
    		}
    	}

    	auto assign (
    		String& other
    	) -> void {
			delete[] thiz.value;
			thiz.value = other.value;
			other.value = nullptr;
			thiz._size = other._size;
			other._size = 0;
    	}

	};

	inline auto operator "" _s (
		const char* value,
		size_t size
	) -> String {
		return String{value, size};
	}

}

namespace std {
	template <>
	struct hash<Sen::Kernel::String> {
		auto operator()(const Sen::Kernel::String& str) const noexcept -> size_t {
			return Sen::Kernel::hash_string(str);
		}
	};
}