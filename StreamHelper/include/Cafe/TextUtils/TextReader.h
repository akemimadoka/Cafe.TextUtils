#pragma once

#include <Cafe/Encoding/Strings.h>
#include <Cafe/Io/Streams/BufferedStream.h>

namespace Cafe::TextUtils
{
	/// @brief  文本读取类
	/// @remark 与常见的设计不同，本类不取得包装的流的所有权，
	///         因此必须由用户手动管理并保证包装的流的生命期在文本读取类的生命期全程都有效
	///         且析构时不会自动关闭包装的类
	template <Encoding::CodePage::CodePageType CodePageValue>
	class TextReader
	{
		template <bool Read>
		std::optional<std::pair<
		    Encoding::StaticString<CodePageValue, Encoding::CodePage::GetMaxWidth<CodePageValue>()>,
		    Encoding::CodePointType>>
		Fetch()
		{
			using Trait = Encoding::CodePage::CodePageTrait<CodePageValue>;
			using CharType = typename Trait::CharType;
			CharType buffer[Encoding::CodePage::GetMaxWidth<CodePageValue>()];
			std::size_t totalReadSize{};
			while (totalReadSize < Encoding::CodePage::GetMaxWidth<CodePageValue>())
			{
				const auto readSize = [&] {
					if constexpr (Read)
					{
						return m_Stream.ReadBytes(
						    gsl::as_writeable_bytes(gsl::make_span(&buffer[totalReadSize], 1)));
					}
					else
					{
						return m_Stream.PeekBytes(
						    gsl::as_writeable_bytes(gsl::make_span(&buffer[totalReadSize], 1)));
					}
				}();
				if (!readSize)
				{
					// 若还未读取任何编码单元，仅直接返回空值表示流已到结尾，否则读取的不完整，应当抛出异常
					if (totalReadSize)
					{
						CAFE_THROW(EncodingFailedException, CAFE_UTF8_SV("End of stream."));
					}

					return {};
				}
				if (readSize % sizeof(CharType))
				{
					// 不完整读取，可能流已到结尾
					CAFE_THROW(EncodingFailedException, CAFE_UTF8_SV("End of stream."));
				}

				++totalReadSize;

				Encoding::CodePointType mayBeCodePoint;
				Encoding::EncodingResultCode resultCode;
				Trait::ToCodePoint(gsl::make_span(buffer, totalReadSize), [&](auto const& result) {
					if constexpr (Encoding::GetEncodingResultCode<decltype(result)> ==
					              Encoding::EncodingResultCode::Accept)
					{
						mayBeCodePoint = result.Result;
					}
					resultCode = Encoding::GetEncodingResultCode<decltype(result)>;
				});

				switch (resultCode)
				{
				case Encoding::EncodingResultCode::Accept:
					return { std::in_place, gsl::make_span(buffer, totalReadSize), mayBeCodePoint };
				case Encoding::EncodingResultCode::Incomplete:
					break;
				case Encoding::EncodingResultCode::Reject:
					CAFE_THROW(EncodingFailedException, CAFE_UTF8_SV("Encoding failed."));
				}
			}

			CAFE_THROW(EncodingFailedException, CAFE_UTF8_SV("Encoding failed."));
		}

	public:
		explicit TextReader(Io::InputStream* stream,
		                    std::size_t bufferSize = Io::BufferedInputStream::DefaultBufferSize)
		    : m_Stream{ stream, bufferSize }
		{
		}

		std::optional<std::pair<
		    Encoding::StaticString<CodePageValue, Encoding::CodePage::GetMaxWidth<CodePageValue>()>,
		    Encoding::CodePointType>>
		Read()
		{
			return Fetch<true>();
		}

		std::optional<std::pair<
		    Encoding::StaticString<CodePageValue, Encoding::CodePage::GetMaxWidth<CodePageValue>()>,
		    Encoding::CodePointType>>
		Peek()
		{
			return Fetch<false>();
		}

		Encoding::String<CodePageValue> ReadLine()
		{
			Encoding::String<CodePageValue> result;

			while (const auto readCodePoint = Read())
			{
				const auto [codeUnits, codePoint] = *readCodePoint;
				if (codePoint == '\n')
				{
					return result;
				}
				else if (codePoint == '\r')
				{
					const auto peekMayBeNewLineCodePoint = Peek();
					if (peekMayBeNewLineCodePoint)
					{
						const auto [mayBeNewLineCodeUnits, mayBeNewLineCodePoint] = *peekMayBeNewLineCodePoint;
						if (mayBeNewLineCodePoint == '\n')
						{
							m_Stream.Skip(mayBeNewLineCodeUnits.GetSize());
							return result;
						}

						result.Append(codeUnits.GetSpan());
						result.Append(mayBeNewLineCodeUnits.GetSpan());
					}
					else
					{
						result.Append(codeUnits.GetSpan());
						break;
					}
				}
				else
				{
					result.Append(codeUnits.GetSpan());
				}
			}

			return result;
		}

		Encoding::String<CodePageValue> ReadUntil(Encoding::CodePointType endingCodePoint)
		{
			Encoding::String<CodePageValue> result;

			while (const auto readCodePoint = Read())
			{
				const auto [codeUnits, codePoint] = *readCodePoint;
				if (codePoint == endingCodePoint)
				{
					return result;
				}

				result.Append(codeUnits.GetSpan());
			}

			return result;
		}

	private:
		Io::BufferedInputStream m_Stream;
	};
} // namespace Cafe::TextUtils