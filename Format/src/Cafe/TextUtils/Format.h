#pragma once

#include <Cafe/Encoding/Strings.h>
#include <Cafe/Misc/Math.h>
#include <Cafe/Misc/NumericInterval.h>
#include <Cafe/TextUtils/CodePointIterator.h>
#include <cassert>
#include <sstream>

namespace Cafe::TextUtils
{
	CAFE_DEFINE_GENERAL_EXCEPTION(FormatException, ErrorHandling::CafeException);

	/// @return 结果及消费的编码单元数量
	template <Encoding::CodePage::CodePageType CodePageValue, std::size_t Extent>
	constexpr std::pair<std::uintmax_t, std::size_t>
	AsciiToNumber(Encoding::StringView<CodePageValue, Extent> const& str, std::size_t base = 10)
	{
		assert(2 <= base && base <= 36);

		const Core::Misc::NumericInterval<Encoding::CodePointType> DecimalCodePointValue{
			{ '0', true },
			{ static_cast<Encoding::CodePointType>('0' + std::min(base, std::size_t{ 10 })), false }
		};
		// 若 base 不大于 10 时将会是空集，不需要特殊处理，下同
		const Core::Misc::NumericInterval<Encoding::CodePointType> OverDecimalCodePointValue{
			{ 'A', true }, { static_cast<Encoding::CodePointType>('A' + base - 10), false }
		};
		const Core::Misc::NumericInterval<Encoding::CodePointType> OverDecimalCodePointValue2{
			{ 'a', true }, { static_cast<Encoding::CodePointType>('a' + base - 10), false }
		};

		std::uintmax_t result{};
		std::size_t resultAdvanceCount{};
		CodePointIterator<CodePageValue> read{ str.GetSpan() };
		const CodePointIterator<CodePageValue> end{};
		for (; read != end; ++read)
		{
			const auto [codePoint, advanceCount] = [&] {
				if constexpr (Encoding::CodePage::CodePageTrait<CodePageValue>::IsVariableWidth)
				{
					return *read;
				}
				else
				{
					return std::pair{ *read, 1 };
				}
			}();
			if (DecimalCodePointValue.InBounds(codePoint))
			{
				result = result * base + (codePoint - '0');
			}
			else if (OverDecimalCodePointValue.InBounds(codePoint))
			{
				result = result * base + (codePoint - 'A' + 10);
			}
			else if (OverDecimalCodePointValue2.InBounds(codePoint))
			{
				result = result * base + (codePoint - 'a' + 10);
			}
			else
			{
				break;
			}
			resultAdvanceCount += advanceCount;
		}

		return { result, resultAdvanceCount };
	}

	struct DefaultStringConverter
	{
		template <typename T, Encoding::CodePage::CodePageType CodePageValue,
		          typename OutputReceiver>
		static constexpr void ToString(T const& value,
		                               Encoding::StringView<CodePageValue> const& formatOption,
		                               OutputReceiver&& receiver)
		{
			if constexpr (std::is_integral_v<T>)
			{
				IntegerToString(value, formatOption, std::forward<OutputReceiver>(receiver));
			}
			else if constexpr (std::is_floating_point_v<T>)
			{
				FloatingToString(value, formatOption, std::forward<OutputReceiver>(receiver));
			}
			else if constexpr (Encoding::IsStringView<T> || Encoding::IsStaticString<T>)
			{
				std::forward<OutputReceiver>(receiver)(value.GetSpan());
			}
			else if constexpr (Encoding::IsString<T>)
			{
				std::forward<OutputReceiver>(receiver)(value.GetView().GetSpan());
			}
			else
			{
				CAFE_THROW(FormatException, CAFE_UTF8_SV("Unformattable data."));
			}
		}

	private:
		template <typename T, Encoding::CodePage::CodePageType CodePageValue,
		          typename OutputReceiver>
		static constexpr void
		IntegerToString(T value, Encoding::StringView<CodePageValue> const& formatOption,
		                OutputReceiver&& receiver)
		{
			// 若 value 是有符号数且是最小值，最后要补加 1
			[[maybe_unused]] auto isSignedMin = false;
			std::size_t base = 10;
			auto baseSpecified = false;
			auto useUppercase = false;

			// 解析格式化选项
			CodePointIterator<CodePageValue> read{ formatOption.GetSpan() };
			const CodePointIterator<CodePageValue> end{};
			for (; read != end; ++read)
			{
				const auto item = *read;
				switch (item.first)
				{
				case 'b':
					if (baseSpecified)
					{
						CAFE_THROW(FormatException, CAFE_UTF8_SV("Base has been specified."));
					}
					base = 2;
					baseSpecified = true;
					break;
				case 'o':
					if (baseSpecified)
					{
						CAFE_THROW(FormatException, CAFE_UTF8_SV("Base has been specified."));
					}
					base = 8;
					baseSpecified = true;
					break;
				case 'd':
				case 'i':
					if (baseSpecified)
					{
						CAFE_THROW(FormatException, CAFE_UTF8_SV("Base has been specified."));
					}
					baseSpecified = true;
					break;
				case 'x':
					if (baseSpecified)
					{
						CAFE_THROW(FormatException, CAFE_UTF8_SV("Base has been specified."));
					}
					base = 16;
					baseSpecified = true;
					break;
				case 'X':
					if (baseSpecified)
					{
						CAFE_THROW(FormatException, CAFE_UTF8_SV("Base has been specified."));
					}
					base = 16;
					useUppercase = true;
					baseSpecified = true;
					break;
				default:
					CAFE_THROW(FormatException, CAFE_UTF8_SV("Invalid option."));
				}
			}

			assert(2 <= base && base <= 36);

			if constexpr (std::is_signed_v<T>)
			{
				if (value < 0)
				{
					Encoding::CodePage::CodePageTrait<CodePageValue>::FromCodePoint(
					    '-', [&](auto const& result) {
						    if constexpr (Encoding::GetEncodingResultCode<decltype(result)> ==
						                  Encoding::EncodingResultCode::Accept)
						    {
							    std::forward<OutputReceiver>(receiver)(result.Result);
						    }
					    });

					if (value == std::numeric_limits<T>::min())
					{
						isSignedMin = true;
						value = std::numeric_limits<T>::max();
					}
					else
					{
						value = -value;
					}
				}
			}

			// value 此时为非负数
			std::size_t digitCount{ 1 };
			auto testValue = value;
			while (testValue /= base)
			{
				++digitCount;
			}

			constexpr Encoding::CodePointType DecimalBase{ '0' };
			constexpr Encoding::CodePointType LowerHexBase{ 'a' };
			constexpr Encoding::CodePointType UpperHexBase{ 'A' };

			for (std::size_t i = digitCount; i > 0; --i)
			{
				auto number = value / Core::Misc::Math::Pow(base, i - 1) % base;
				if (i == 1 && isSignedMin)
				{
					++number;
				}
				const Encoding::CodePointType codePoint =
				    number < 10 ? DecimalBase + number
				                : (useUppercase ? UpperHexBase : LowerHexBase) + (number - 10);
				Encoding::CodePage::CodePageTrait<CodePageValue>::FromCodePoint(
				    codePoint, [&](auto const& result) {
					    if constexpr (Encoding::GetEncodingResultCode<decltype(result)> ==
					                  Encoding::EncodingResultCode::Accept)
					    {
						    std::forward<OutputReceiver>(receiver)(result.Result);
					    }
				    });
			}
		}

		template <typename T, Encoding::CodePage::CodePageType CodePageValue,
		          typename OutputReceiver>
		static constexpr void
		FloatingToString(T value, Encoding::StringView<CodePageValue> const& formatOption,
		                 OutputReceiver&& receiver)
		{
			// 有效小数部分
			std::size_t significantDecimal{ 5 };

			// TODO: 解析格式化选项
			CodePointIterator<CodePageValue> read{ formatOption.GetSpan() };
			const CodePointIterator<CodePageValue> end{};
			for (; read != end; ++read)
			{
				const auto item = *read;
				switch (item.first)
				{
				default:
					CAFE_THROW(FormatException, CAFE_UTF8_SV("Invalid option."));
				}
			}

			if (value != value)
			{
				// 是 NaN
				constexpr Encoding::CodePointType NanStr[] = { 'N', 'a', 'N' };
				Encoding::Encoder<Encoding::CodePage::CodePoint, CodePageValue>::EncodeAll(
				    std::span(NanStr), [&](auto const& result) {
					    if constexpr (Encoding::GetEncodingResultCode<decltype(result)> ==
					                  Encoding::EncodingResultCode::Accept)
					    {
						    std::forward<OutputReceiver>(receiver)(result.Result);
					    }
				    });
				return;
			}

			if (value > std::numeric_limits<std::intmax_t>::max() ||
			    value < std::numeric_limits<std::intmax_t>::min())
			{
				CAFE_THROW(FormatException, CAFE_UTF8_SV("value is too big, not implemented now."));
			}

			// TODO: 偷懒做法，bug 很多

			// 输出整数部分
			IntegerToString(static_cast<std::intmax_t>(value),
			                Encoding::StringView<CodePageValue>{},
			                std::forward<OutputReceiver>(receiver));
			if (value < T{})
			{
				value = -value;
			}

			value -= static_cast<std::intmax_t>(value);

			if (value > std::numeric_limits<T>::epsilon())
			{
				// 输出小数部分
				Encoding::CodePage::CodePageTrait<CodePageValue>::FromCodePoint(
				    '.', [&](auto const& result) {
					    if constexpr (Encoding::GetEncodingResultCode<decltype(result)> ==
					                  Encoding::EncodingResultCode::Accept)
					    {
						    std::forward<OutputReceiver>(receiver)(result.Result);
					    }
				    });
				value *= Core::Misc::Math::Pow(10, significantDecimal);
				IntegerToString(static_cast<std::intmax_t>(value),
				                Encoding::StringView<CodePageValue>{},
				                std::forward<OutputReceiver>(receiver));
			}
		}
	};

	struct StringStreamStringConverter
	{
	private:
		std::stringstream m_InternalBuffer;

	public:
		template <typename T, Encoding::CodePage::CodePageType CodePageValue,
		          typename OutputReceiver>
		void ToString(T const& value,
		              [[maybe_unused]] Encoding::StringView<CodePageValue> const& formatOption,
		              OutputReceiver&& receiver)
		{
			m_InternalBuffer << value;
			const auto tmpStr = EncodeFromNarrow<CodePageValue>(m_InternalBuffer.str());
			std::forward<OutputReceiver>(receiver)(tmpStr.GetView().GetSpan());
			m_InternalBuffer.str({});
			m_InternalBuffer.clear();
		}
	};

	struct StdToStringStringConverter
	{
		template <typename T, Encoding::CodePage::CodePageType CodePageValue,
		          typename OutputReceiver>
		void ToString(T const& value,
		              [[maybe_unused]] Encoding::StringView<CodePageValue> const& formatOption,
		              OutputReceiver&& receiver)
		{
			const auto result = std::to_string(value);
			const auto tmpStr = EncodeFromNarrow<CodePageValue>(result);
			std::forward<OutputReceiver>(receiver)(tmpStr.GetView().GetSpan());
		}
	};

	template <Encoding::CodePage::CodePageType CodePageValue>
	struct FormatInfo
	{
		std::size_t Index;
		Encoding::StringView<CodePageValue> FormatOptionText;
	};

	struct DefaultFormatter
	{
		static constexpr Encoding::CodePointType FormatPrefix{ '$' };
		static constexpr Encoding::CodePointType FormatLeftQuote{ '{' };
		static constexpr Encoding::CodePointType FormatOptionToken{ ':' };
		static constexpr Encoding::CodePointType FormatRightQuote{ '}' };

		constexpr DefaultFormatter() noexcept : m_CurrentMode{ Mode::Unknown }, m_CurrentIndex{}
		{
		}

	private:
		enum class Mode
		{
			Unknown,
			IndexMode,
			AutoMode,
		};

		Mode m_CurrentMode;

		// 不使用索引时自动增加索引并选择参数，不可与使用索引混用
		std::size_t m_CurrentIndex;

		template <Encoding::CodePage::CodePageType CodePageValue, std::size_t Extent>
		static constexpr std::pair<bool, std::size_t>
		BeginWith(Encoding::StringView<CodePageValue, Extent> const& format,
		          Encoding::CodePointType codePoint) noexcept
		{
			if (format.IsEmpty())
			{
				return { false, 0 };
			}

			using Trait = Encoding::CodePage::CodePageTrait<CodePageValue>;
			std::pair<bool, std::size_t> result;
			Trait::ToCodePoint(format.GetSpan(), [&](auto const& encodingResult) {
				if constexpr (Encoding::GetEncodingResultCode<decltype(encodingResult)> ==
				              Encoding::EncodingResultCode::Accept)
				{
					result = { encodingResult.Result == codePoint, encodingResult.AdvanceCount };
				}
				else
				{
					result = { false, 1 };
				}
			});

			return result;
		}

		// TODO: 是否可针对非编译期的情况优化？例如使用 SIMD
		template <Encoding::CodePage::CodePageType CodePageValue, std::size_t Extent>
		static constexpr std::size_t SkipUntil(Encoding::StringView<CodePageValue, Extent> format,
		                                       Encoding::CodePointType codePoint) noexcept
		{
			if (format.IsEmpty())
			{
				return 0;
			}

			using Trait = Encoding::CodePage::CodePageTrait<CodePageValue>;
			std::size_t result{};
			auto shouldStop = false;
			do
			{
				Trait::ToCodePoint(format.GetSpan(), [&](auto const& encodingResult) {
					if constexpr (Encoding::GetEncodingResultCode<decltype(encodingResult)> ==
					              Encoding::EncodingResultCode::Accept)
					{
						shouldStop = encodingResult.Result == codePoint;
						// 到匹配之时即停止，不消费匹配的值
						if (!shouldStop)
						{
							result += encodingResult.AdvanceCount;
							format = format.SubStr(encodingResult.AdvanceCount);
						}
					}
					else
					{
						// 编码失败，此时不与要寻找的值匹配因此增加并继续
						++result;
						format = format.SubStr(1);
					}
				});
			} while (!shouldStop && !format.IsEmpty());
			return result;
		}

		template <Encoding::CodePage::CodePageType CodePageValue, std::size_t Extent>
		constexpr std::pair<FormatInfo<CodePageValue>, std::size_t>
		ParseFormatInfo(Encoding::StringView<CodePageValue, Extent> const& format)
		{
			Encoding::StringView<CodePageValue> formatStr = format;
			const auto begin = formatStr.begin();
			FormatInfo<CodePageValue> result{};

			const auto beginWithFormatLeftQuote = BeginWith(formatStr, FormatLeftQuote);
			if (!beginWithFormatLeftQuote.first)
			{
				CAFE_THROW(FormatException, CAFE_UTF8_SV("Invalid format string."));
			}

			formatStr = formatStr.SubStr(beginWithFormatLeftQuote.second);

			auto indexBegin = formatStr.begin();
			while (true)
			{
				if (formatStr.IsEmpty())
				{
					CAFE_THROW(FormatException, CAFE_UTF8_SV("Invalid format string."));
				}

				const auto prevPos = formatStr.begin();
				const auto beginWithFormatOptionToken = BeginWith(formatStr, FormatOptionToken);
				auto parseFormatOption = true;
				if (beginWithFormatOptionToken.first)
				{
					formatStr = formatStr.SubStr(beginWithFormatOptionToken.second);
				}
				else
				{
					const auto beginWithFormatRightQuote = BeginWith(formatStr, FormatRightQuote);
					formatStr = formatStr.SubStr(beginWithFormatRightQuote.second);
					if (beginWithFormatRightQuote.first)
					{
						parseFormatOption = false;
					}
					else
					{
						continue;
					}
				}

				if (indexBegin == prevPos)
				{
					if (m_CurrentMode == Mode::IndexMode)
					{
						CAFE_THROW(FormatException, CAFE_UTF8_SV("Invalid format string."));
					}

					m_CurrentMode = Mode::AutoMode;
				}
				else
				{
					if (m_CurrentMode == Mode::AutoMode)
					{
						CAFE_THROW(FormatException, CAFE_UTF8_SV("Invalid format string."));
					}

					m_CurrentMode = Mode::IndexMode;
				}

				if (m_CurrentMode == Mode::IndexMode)
				{
					const auto parsedIndex =
					    AsciiToNumber(Encoding::StringView<CodePageValue, Extent>{
					        std::span(indexBegin, prevPos) });

					if (parsedIndex.second != std::distance(indexBegin, prevPos))
					{
						CAFE_THROW(FormatException, CAFE_UTF8_SV("Invalid format string."));
					}

					result.Index = parsedIndex.first;
				}
				else
				{
					assert(m_CurrentMode == Mode::AutoMode);
					result.Index = m_CurrentIndex++;
				}

				if (parseFormatOption)
				{
					const auto formatOptionBegin = formatStr.begin();
					while (true)
					{
						if (formatStr.IsEmpty())
						{
							CAFE_THROW(FormatException, CAFE_UTF8_SV("Invalid format string."));
						}

						const auto beginWithFormatRightQuote =
						    BeginWith(formatStr, FormatRightQuote);
						const auto curPos = formatStr.begin();
						formatStr = formatStr.SubStr(beginWithFormatRightQuote.second);
						if (beginWithFormatRightQuote.first)
						{
							result.FormatOptionText = Encoding::StringView<CodePageValue, Extent>{
								std::span(formatOptionBegin, curPos)
							};
							break;
						}
					}
				}

				break;
			}

			return { result, std::distance(begin, formatStr.begin()) };
		}

	public:
		/// @brief  尝试分析格式化信息
		/// @return 格式化信息、消耗的编码单元个数、跳过的编码单元个数
		///         跳过的编码单元不计入消耗之中，将会直接跳过，为了处理 escape
		template <Encoding::CodePage::CodePageType CodePageValue, std::size_t Extent>
		constexpr std::tuple<std::optional<FormatInfo<CodePageValue>>, std::size_t, std::size_t>
		TryParseFormatInfo(Encoding::StringView<CodePageValue, Extent> const& format)
		{
			const auto beginWithFormatPrefix = BeginWith(format, FormatPrefix);
			if (!beginWithFormatPrefix.first)
			{
				const auto skippedLength =
				    SkipUntil(format.SubStr(beginWithFormatPrefix.second), FormatPrefix);
				return { {}, beginWithFormatPrefix.second + skippedLength, 0 };
			}

			const auto restFormat = format.SubStr(beginWithFormatPrefix.second);
			const auto [secondIsFormatPrefix, consumedCount] = BeginWith(restFormat, FormatPrefix);
			// 下一个仍然是 FormatPrefix，作为 escape 被跳过
			if (secondIsFormatPrefix)
			{
				return { {}, consumedCount, beginWithFormatPrefix.second };
			}

			auto result = ParseFormatInfo(restFormat);
			return { std::move(result.first), beginWithFormatPrefix.second + result.second, 0 };
		}
	};

	template <typename OutputReceiver, typename Formatter, typename StringConverter,
	          Encoding::CodePage::CodePageType CodePageValue, std::size_t Extent, typename... Args>
	constexpr void FormatStringWithCustomFormatter(
	    OutputReceiver&& receiver, Formatter&& formatter, StringConverter&& stringConverter,
	    Encoding::StringView<CodePageValue, Extent> const& format, Args const&... args)
	{
		Encoding::StringView<CodePageValue> formatStr = format;
		const auto argsTuple = std::forward_as_tuple(args...);
		while (true)
		{
			const auto [formatInfo, advanceCount, skippedCount] =
			    std::forward<Formatter>(formatter).TryParseFormatInfo(formatStr);
			formatStr = formatStr.SubStr(skippedCount);
			const auto prevPos = formatStr.begin();
			formatStr = formatStr.SubStr(advanceCount);
			if (formatInfo.has_value())
			{
				const auto info = formatInfo.value();
				if (!Core::Misc::RuntimeGet(info.Index, argsTuple, [&](auto const& item) {
					    std::forward<StringConverter>(stringConverter)
					        .ToString(item, info.FormatOptionText, [&](auto&& str) {
						        std::forward<OutputReceiver>(receiver)(
						            static_cast<decltype(str)&&>(str));
					        });
				    }))
				{
					CAFE_THROW(FormatException, CAFE_UTF8_SV("Index out of range."));
				}
			}
			else if (prevPos != formatStr.end())
			{
				std::forward<OutputReceiver>(receiver)(
				    Encoding::StringView<CodePageValue>{ std::span(prevPos, formatStr.begin()) });
			}
			else
			{
				break;
			}
		}
	}

	template <typename OutputReceiver, Encoding::CodePage::CodePageType CodePageValue,
	          std::size_t Extent, typename... Args>
	constexpr void
	FormatStringWithReceiver(OutputReceiver&& receiver,
	                         Encoding::StringView<CodePageValue, Extent> const& format,
	                         Args const&... args)
	{
		FormatStringWithCustomFormatter(std::forward<OutputReceiver>(receiver), DefaultFormatter{},
		                                DefaultStringConverter{}, format, args...);
	}

	template <Encoding::CodePage::CodePageType CodePageValue, std::size_t Extent, typename... Args>
	constexpr std::size_t
	FormatStringSize(Encoding::StringView<CodePageValue, Extent> const& format, Args const&... args)
	{
		std::size_t size{};
		FormatStringWithReceiver(
		    [&](auto const& result) {
			    using ResultType = Core::Misc::RemoveCvRef<decltype(result)>;
			    if constexpr (std::is_same_v<ResultType, typename Encoding::CodePage::CodePageTrait<
			                                                 CodePageValue>::CharType>)
			    {
				    ++size;
			    }
			    else
			    {
				    size += result.GetSize();
			    }
		    },
		    format, args...);
		return size;
	}

	template <typename Allocator, std::size_t SsoThresholdSize, typename GrowPolicy,
	          Encoding::CodePage::CodePageType CodePageValue, std::size_t Extent, typename... Args>
	Encoding::String<CodePageValue, Allocator, SsoThresholdSize, GrowPolicy>
	FormatCustomString(Encoding::StringView<CodePageValue, Extent> const& format,
	                   Args const&... args)
	{
		Encoding::String<CodePageValue, Allocator, SsoThresholdSize, GrowPolicy> resultStr;
		FormatStringWithReceiver([&](auto const& result) { resultStr.Append(result); }, format,
		                         args...);
		return resultStr;
	}

	template <Encoding::CodePage::CodePageType CodePageValue, std::size_t Extent, typename... Args>
	Encoding::String<CodePageValue>
	FormatString(Encoding::StringView<CodePageValue, Extent> const& format, Args const&... args)
	{
		Encoding::String<CodePageValue> resultStr;
		FormatStringWithReceiver([&](auto const& result) { resultStr.Append(result); }, format,
		                         args...);
		return resultStr;
	}
} // namespace Cafe::TextUtils
