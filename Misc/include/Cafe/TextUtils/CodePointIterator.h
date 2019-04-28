#include <Cafe/Encoding/Encode.h>
#include <Cafe/ErrorHandling/ErrorHandling.h>
#include <optional>

#include "Misc.h"

namespace Cafe::TextUtils
{
	namespace Detail
	{
		template <bool>
		struct MaybeCurrentInfoBase
		{
		protected:
			mutable std::pair<Encoding::CodePointType, std::size_t> m_CurrentCodePointWidth;

			constexpr MaybeCurrentInfoBase() noexcept
			    : m_CurrentCodePointWidth{ std::numeric_limits<Encoding::CodePointType>::max(),
				                             std::numeric_limits<std::size_t>::max() }
			{
			}

			constexpr bool IsStateValid() const noexcept
			{
				return m_CurrentCodePointWidth.first != std::numeric_limits<Encoding::CodePointType>::max();
			}

			constexpr void ResetState() const noexcept
			{
				m_CurrentCodePointWidth.first = std::numeric_limits<Encoding::CodePointType>::max();
			}
		};

		template <>
		struct MaybeCurrentInfoBase<false>
		{
		protected:
			mutable Encoding::CodePointType m_CurrentCodePoint;

			constexpr MaybeCurrentInfoBase() noexcept
			    : m_CurrentCodePoint{ std::numeric_limits<Encoding::CodePointType>::max() }
			{
			}

			constexpr bool IsStateValid() const noexcept
			{
				return m_CurrentCodePoint != std::numeric_limits<Encoding::CodePointType>::max();
			}

			constexpr void ResetState() const noexcept
			{
				m_CurrentCodePoint = std::numeric_limits<Encoding::CodePointType>::max();
			}
		};
	} // namespace Detail

	struct ThrowOnEncodingFailedPolicy
	{
		[[noreturn]] static Encoding::CodePointType GetReplacementCodePoint()
		{
			CAFE_THROW(EncodingFailedException, CAFE_UTF8_SV("Encoding failed."));
		}
	};

	template <Encoding::CodePointType ReplacementCodePoint = 0xFFFD>
	struct ReturnReplacementPolicy
	{
		static constexpr Encoding::CodePointType GetReplacementCodePoint() noexcept
		{
			return ReplacementCodePoint;
		}
	};

	template <Encoding::CodePage::CodePageType CodePageValue,
	          typename OnEncodingFailedPolicy = ThrowOnEncodingFailedPolicy>
	class CodePointIterator : Detail::MaybeCurrentInfoBase<
	                              Encoding::CodePage::CodePageTrait<CodePageValue>::IsVariableWidth>
	{
		using UsingCodePageTrait = Encoding::CodePage::CodePageTrait<CodePageValue>;
		using CharType = typename UsingCodePageTrait::CharType;

	public:
		static constexpr auto UsingCodePage = CodePageValue;

		using iterator_category = std::forward_iterator_tag;
		using value_type = std::conditional_t<UsingCodePageTrait::IsVariableWidth,
		                                      std::pair<Encoding::CodePointType, std::size_t>,
		                                      Encoding::CodePointType>;
		using difference_type = std::ptrdiff_t;

		using pointer = value_type*;
		using reference = value_type;

		// 作为 end 迭代器
		constexpr CodePointIterator() noexcept
		{
		}

		constexpr CodePointIterator(gsl::span<const CharType> const& span) noexcept
		    : m_UnderlyingSpan{ span }
		{
		}

		constexpr reference operator*() const
		    noexcept(noexcept(OnEncodingFailedPolicy::GetReplacementCodePoint()))
		{
			return GetCurrentInfo();
		}

		constexpr CodePointIterator&
		operator++() noexcept(!UsingCodePageTrait::IsVariableWidth ||
		                      noexcept(OnEncodingFailedPolicy::GetReplacementCodePoint()))
		{
			if constexpr (UsingCodePageTrait::IsVariableWidth)
			{
				m_UnderlyingSpan = m_UnderlyingSpan.subspan(GetCurrentInfo().second);
			}
			else
			{
				m_UnderlyingSpan = m_UnderlyingSpan.subspan(1);
			}

			this->ResetState();

			return *this;
		}

		constexpr bool operator==(CodePointIterator const& other) const noexcept
		{
			return (m_UnderlyingSpan.empty() && other.m_UnderlyingSpan.empty()) ||
			       (m_UnderlyingSpan.data() == other.m_UnderlyingSpan.data() &&
			        m_UnderlyingSpan.size() == other.m_UnderlyingSpan.size());
		}

		constexpr bool operator!=(CodePointIterator const& other) const noexcept
		{
			return !(*this == other);
		}

		constexpr bool IsEnd() const noexcept
		{
			return m_UnderlyingSpan.empty();
		}

		constexpr gsl::span<const CharType> const& GetSpan() const noexcept
		{
			return m_UnderlyingSpan;
		}

	private:
		constexpr auto GetCurrentInfo() const
		    noexcept(noexcept(OnEncodingFailedPolicy::GetReplacementCodePoint()))
		{
			if constexpr (UsingCodePageTrait::IsVariableWidth)
			{
				if (!this->IsStateValid())
				{
					UsingCodePageTrait::ToCodePoint(m_UnderlyingSpan, [this](auto const& result) {
						if constexpr (Encoding::GetEncodingResultCode<decltype(result)> ==
						              Encoding::EncodingResultCode::Accept)
						{
							this->m_CurrentCodePointWidth = { result.Result, result.AdvanceCount };
						}
						else
						{
							this->m_CurrentCodePointWidth = { OnEncodingFailedPolicy::GetReplacementCodePoint(),
								                                1 };
						}
					});
				}

				return this->m_CurrentCodePointWidth;
			}
			else
			{
				if (!this->IsStateValid())
				{
					UsingCodePageTrait::ToCodePoint(m_UnderlyingSpan[0], [this](auto const& result) {
						if constexpr (Encoding::GetEncodingResultCode<decltype(result)> ==
						              Encoding::EncodingResultCode::Accept)
						{
							this->m_CurrentCodePoint = result.Result;
						}
						else
						{
							this->m_CurrentCodePoint = OnEncodingFailedPolicy::GetReplacementCodePoint();
						}
					});
				}

				return this->m_CurrentCodePoint;
			}
		}

		gsl::span<const CharType> m_UnderlyingSpan;
	};
} // namespace Cafe::TextUtils
