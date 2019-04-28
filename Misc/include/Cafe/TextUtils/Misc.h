#pragma once

#include <Cafe/Encoding/Strings.h>
#include <Cafe/ErrorHandling/ErrorHandling.h>

#if __has_include(<Cafe/Encoding/RuntimeEncoding.h>)
#	include <Cafe/Encoding/RuntimeEncoding.h>
#endif

namespace Cafe::TextUtils
{
	CAFE_DEFINE_GENERAL_EXCEPTION(EncodingFailedException);

	template <Encoding::CodePage::CodePageType ToCodePage,
	          Encoding::CodePage::CodePageType FromCodePage, std::ptrdiff_t Extent>
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
			    str.GetSpan(), [&](auto const& result) {
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
	          Encoding::CodePage::CodePageType FromCodePage, std::ptrdiff_t Extent>
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

		return gsl::make_span(str, end - str);
	}

#if __has_include(<Cafe/Encoding/RuntimeEncoding.h>)

	template <Encoding::CodePage::CodePageType ToCodePage>
	Encoding::String<ToCodePage> EncodingFromRuntime(Encoding::CodePage::CodePageType fromCodePage,
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

#	ifdef _WIN32
	template <Encoding::CodePage::CodePageType ToCodePage>
	Encoding::String<ToCodePage> EncodeFromNarrow(std::string_view const& str)
	{
		return EncodingFromRuntime<ToCodePage>(Encoding::RuntimeEncoding::GetAnsiEncoding(),
		                                       gsl::as_bytes(gsl::make_span(str.data(), str.size())));
	}
#	endif
#endif

#ifdef _WIN32
	template <Encoding::CodePage::CodePageType ToCodePage>
	Encoding::String<ToCodePage> EncodeFromWide(std::wstring_view const& str)
	{
		static_assert(sizeof(wchar_t) == sizeof(char16_t));
		return EncodeTo<ToCodePage, Encoding::CodePage::Utf16LittleEndian>(
		    gsl::make_span(reinterpret_cast<const char16_t*>(str.data()), str.size()));
	}
#endif

#ifdef __linux__
	template <Encoding::CodePage::CodePageType ToCodePage>
	Encoding::String<ToCodePage> EncodeFromNarrow(std::string_view const& str)
	{
		using Utf8CharType =
		    typename Encoding::CodePage::CodePageTrait<Encoding::CodePage::Utf8>::CharType;
		static_assert(sizeof(Utf8CharType) == sizeof(char));
		return EncodeTo<ToCodePagem, Encoding::CodePage::Utf8>(
		    gsl::make_span(reinterpret_cast<const Utf8CharType*>(str.data()), str.size()));
	}
#endif
} // namespace Cafe::TextUtils
