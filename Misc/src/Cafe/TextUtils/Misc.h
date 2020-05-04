#pragma once

#include <Cafe/Encoding/Strings.h>
#include <Cafe/ErrorHandling/ErrorHandling.h>

#if __has_include(<Cafe/Encoding/RuntimeEncoding.h>)
#	include <Cafe/Encoding/RuntimeEncoding.h>
#	include <Cafe/Misc/Environment.h>
#endif

namespace Cafe::TextUtils
{
	CAFE_DEFINE_GENERAL_EXCEPTION(EncodingFailedException);

	template <Encoding::CodePage::CodePageType ToCodePage,
	          Encoding::CodePage::CodePageType FromCodePage, std::size_t Extent>
	Encoding::String<ToCodePage> EncodeTo(Encoding::StringView<FromCodePage, Extent> const& str)
	{
		if constexpr (FromCodePage == ToCodePage)
		{
			return str;
		}
		else
		{
			Encoding::String<ToCodePage> resultStr;
			Encoding::Encoder<FromCodePage, ToCodePage>::EncodeAll(
			    str.GetTrimmedSpan(), [&](auto const& result) {
				    if constexpr (Encoding::GetEncodingResultCode<decltype(result)> ==
				                  Encoding::EncodingResultCode::Accept)
				    {
					    resultStr.Append(result.Result);
				    }
				    else
				    {
					    CAFE_THROW(EncodingFailedException, CAFE_UTF8_SV("Encoding failed"));
				    }
			    });
			return resultStr;
		}
	}

	template <Encoding::CodePage::CodePageType ToCodePage,
	          Encoding::CodePage::CodePageType FromCodePage, std::size_t Extent>
	Encoding::String<ToCodePage>
	EncodeToWithReplacement(Encoding::StringView<FromCodePage, Extent> str,
	                        Encoding::CodePointType replacement = 0xFFFD)
	{
		if constexpr (FromCodePage == ToCodePage)
		{
			return str;
		}
		else
		{
			str = str.Trim();
			Encoding::String<ToCodePage> resultStr;
			while (!str.IsEmpty())
			{
				Encoding::Encoder<FromCodePage, ToCodePage>::EncodeAll(str, [&](auto const& result) {
					if constexpr (Encoding::GetEncodingResultCode<decltype(result)> ==
					              Encoding::EncodingResultCode::Accept)
					{
						if constexpr (Encoding::CodePage::CodePageTrait<FromCodePage>::IsVariableWidth)
						{
							str = str.SubStr(result.AdvanceCount);
						}
						else
						{
							str = str.SubStr(1);
						}
						resultStr.Append(result.Result);
					}
					else
					{
						// 此时编码中止
						str = str.SubStr(1);
						Encoding::CodePage::CodePageTrait<ToCodePage>::FromCodePoint(
						    replacement, [&](auto const& result) {
							    if constexpr (Encoding::GetEncodingResultCode<decltype(result)> ==
							                  Encoding::EncodingResultCode::Accept)
							    {
								    resultStr.Append(result.Result);
							    }
						    });
					}
				});
			}

			return resultStr;
		}
	}

	template <Encoding::CodePage::CodePageType CodePageValue>
	constexpr Encoding::StringView<CodePageValue> AsNullTerminatedStringView(
	    const typename Encoding::CodePage::CodePageTrait<CodePageValue>::CharType* str) noexcept
	{
		if (!str)
		{
			return {};
		}

		auto end = str;
		while (*end++)
		{
		}

		return gsl::span(str, end - str);
	}

#if __has_include(<Cafe/Encoding/RuntimeEncoding.h>)

	template <Encoding::CodePage::CodePageType ToCodePage>
	Encoding::String<ToCodePage> EncodeFromRuntime(Encoding::CodePage::CodePageType fromCodePage,
	                                               gsl::span<const std::byte> const& span)
	{
		Encoding::String<ToCodePage> resultStr;
		resultStr.Reserve(span.size());
		Encoding::RuntimeEncoding::RuntimeEncoder<ToCodePage>::EncodeAllFrom(
		    fromCodePage, gsl::as_bytes(span), [&](auto const& result) {
			    if (result.ResultCode == Encoding::EncodingResultCode::Accept)
			    {
				    resultStr.Append(result.Result);
			    }
			    else
			    {
				    CAFE_THROW(EncodingFailedException, CAFE_UTF8_SV("Encoding failed"));
			    }
		    });
		return resultStr;
	}

	template <Encoding::CodePage::CodePageType FromCodePage>
	std::vector<std::byte> EncodeToRuntime(Encoding::StringView<FromCodePage> const& str,
	                                       Encoding::CodePage::CodePageType toCodePage)
	{
		std::vector<std::byte> resultVec;
		resultVec.reserve(str.GetSize());
		Encoding::RuntimeEncoding::RuntimeEncoder<FromCodePage>::EncodeAllTo(
		    str.GetSpan(), toCodePage, [&](auto const& result) {
			    if (result.ResultCode == Encoding::EncodingResultCode::Accept)
			    {
				    resultVec.insert(resultVec.end(), result.Result.cbegin(), result.Result.cend());
			    }
			    else
			    {
				    CAFE_THROW(EncodingFailedException, CAFE_UTF8_SV("Encoding failed"));
			    }
		    });
		return resultVec;
	}

#	ifdef _WIN32
	template <Encoding::CodePage::CodePageType ToCodePage>
	Encoding::String<ToCodePage> EncodeFromNarrow(std::string_view const& str)
	{
		return EncodeFromRuntime<ToCodePage>(Environment::GetNarrowEncoding(),
		                                     gsl::as_bytes(gsl::span(str.data(), str.size())));
	}

	template <Encoding::CodePage::CodePageType FromCodePage>
	std::string EncodeToNarrow(Encoding::StringView<FromCodePage> const& str)
	{
		std::string resultStr;
		resultStr.reserve(str.GetSize());
		Encoding::RuntimeEncoding::RuntimeEncoder<FromCodePage>::EncodeAllTo(
		    Environment::GetNarrowEncoding(), str.GetTrimmedSpan(), [&](auto const& result) {
			    if (result.ResultCode == Encoding::EncodingResultCode::Accept)
			    {
				    resultStr.append(reinterpret_cast<const char*>(result.Result.data()),
				                     result.Result.size());
			    }
			    else
			    {
				    CAFE_THROW(EncodingFailedException, CAFE_UTF8_SV("Encoding failed"));
			    }
		    });
		return resultStr;
	}
#	endif
#endif

#ifdef _WIN32
	static_assert(sizeof(wchar_t) == sizeof(char16_t) && alignof(wchar_t) == alignof(char16_t));

	template <Encoding::CodePage::CodePageType ToCodePage>
	Encoding::String<ToCodePage> EncodeFromWide(std::wstring_view const& str)
	{
		return EncodeTo<ToCodePage>(Encoding::StringView<Encoding::CodePage::Utf16LittleEndian>(
		    gsl::span(reinterpret_cast<const char16_t*>(str.data()), str.size())));
	}

	template <Encoding::CodePage::CodePageType FromCodePage>
	std::wstring EncodeToWide(Encoding::StringView<FromCodePage> const& str)
	{
		const auto tmpStr = EncodeTo<Encoding::CodePage::Utf16LittleEndian>(str);
		return std::wstring(reinterpret_cast<const wchar_t*>(tmpStr.GetData()), tmpStr.GetSize());
	}
#endif

#ifdef __linux__
	template <Encoding::CodePage::CodePageType ToCodePage>
	Encoding::String<ToCodePage> EncodeFromNarrow(std::string_view const& str)
	{
		using Utf8CharType =
		    typename Encoding::CodePage::CodePageTrait<Encoding::CodePage::Utf8>::CharType;
		static_assert(sizeof(Utf8CharType) == sizeof(char) && alignof(Utf8CharType) == alignof(char));
		return EncodeTo<ToCodePage>(Encoding::StringView<Encoding::CodePage::Utf8>(
		    gsl::span(reinterpret_cast<const Utf8CharType*>(str.data()), str.size())));
	}

	template <Encoding::CodePage::CodePageType FromCodePage>
	std::string EncodeToNarrow(Encoding::StringView<FromCodePage> const& str)
	{
		using Utf8CharType =
		    typename Encoding::CodePage::CodePageTrait<Encoding::CodePage::Utf8>::CharType;
		static_assert(sizeof(Utf8CharType) == sizeof(char) && alignof(Utf8CharType) == alignof(char));
		const auto tmpStr = EncodeTo<Encoding::CodePage::Utf8>(str);
		return std::string(reinterpret_cast<const char*>(tmpStr.GetData()), tmpStr.GetSize());
	}

	static_assert(sizeof(wchar_t) == sizeof(Encoding::CodePointType) &&
	              alignof(wchar_t) == alignof(Encoding::CodePointType));

	template <Encoding::CodePage::CodePageType ToCodePage>
	Encoding::String<ToCodePage> EncodeFromWide(std::wstring_view const& str)
	{
		return EncodeTo<ToCodePage>(Encoding::StringView<Encoding::CodePage::CodePoint>(
		    gsl::span(reinterpret_cast<const Encoding::CodePointType*>(str.data()), str.size())));
	}

	template <Encoding::CodePage::CodePageType FromCodePage>
	std::wstring EncodeToWide(Encoding::StringView<FromCodePage> const& str)
	{
		const auto tmpStr = EncodeTo<Encoding::CodePage::CodePoint>(str);
		return std::wstring(reinterpret_cast<const wchar_t*>(tmpStr.GetData()), tmpStr.GetSize());
	}
#endif
} // namespace Cafe::TextUtils
