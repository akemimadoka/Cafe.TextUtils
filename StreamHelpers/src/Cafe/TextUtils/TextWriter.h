#pragma once

#include <Cafe/Encoding/Strings.h>
#include <Cafe/Io/Streams/BufferedStream.h>
#include <Cafe/Misc/Environment.h>
#include <Cafe/TextUtils/Format.h>

namespace Cafe::TextUtils
{
	/// @brief  文本写入类
	/// @remark 与常见的设计不同，本类不取得包装的流的所有权，
	///         因此必须由用户手动管理并保证包装的流的生命期在文本写入类的生命期全程都有效
	///         且析构时不会自动关闭包装的类
	template <Encoding::CodePage::CodePageType CodePageValue>
	class TextWriter
	{
	public:
		explicit TextWriter(Io::OutputStream* stream,
		                    std::size_t bufferSize = Io::BufferedOutputStream::DefaultBufferSize)
		    : m_Stream{ stream, bufferSize }
		{
		}

		void Flush()
		{
			m_Stream.Flush();
		}

		template <typename... Args>
		std::size_t Write(Encoding::StringView<CodePageValue> const& format, Args const&... args)
		{
			if constexpr (!sizeof...(args))
			{
				return m_Stream.WriteBytes(gsl::as_bytes(format.GetTrimmedSpan()));
			}
			else
			{
				const auto tmpStr = FormatString(format, args...);
				return m_Stream.WriteBytes(gsl::as_bytes(tmpStr.GetView().GetTrimmedSpan()));
			}
		}

		template <typename... Args>
		std::size_t WriteLine(Encoding::StringView<CodePageValue> const& format, Args const&... args)
		{
			using UsingTrait = Encoding::CodePage::CodePageTrait<CodePageValue>;
			auto writtenBytes = Write(format, args...);
			Encoding::Encoder<Encoding::CodePage::Utf8, CodePageValue>::EncodeAll(
			    Environment::GetNewLine().GetTrimmedSpan(), [&](auto const& result) {
				    if constexpr (Encoding::GetEncodingResultCode<decltype(result)> ==
				                  Encoding::EncodingResultCode::Accept)
				    {
					    if constexpr (UsingTrait::IsVariableWidth)
					    {
						    writtenBytes += m_Stream.WriteBytes(gsl::as_bytes(result.Result));
					    }
					    else
					    {
						    writtenBytes +=
						        m_Stream.WriteBytes(gsl::as_bytes(gsl::span(&result.Result, 1)));
					    }
				    }
				    else
				    {
					    assert(!"Should never happen.");
				    }
			    });

			return writtenBytes;
		}

		Io::BufferedOutputStream* GetStream() noexcept
		{
			return &m_Stream;
		}

	private:
		Io::BufferedOutputStream m_Stream;
	};
} // namespace Cafe::TextUtils
