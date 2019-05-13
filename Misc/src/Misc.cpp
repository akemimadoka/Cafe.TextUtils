#include <Cafe/TextUtils/Misc.h>

using namespace Cafe;
using namespace TextUtils;

std::vector<std::byte> Cafe::TextUtils::EncodeRuntime(Encoding::CodePage::CodePageType fromCodePage,
                                                      gsl::span<const std::byte> const& span,
                                                      Encoding::CodePage::CodePageType toCodePage)
{
	std::vector<std::byte> resultVec;
	resultVec.reserve(span.size());
	Encoding::RuntimeEncoding::EncodeAll(fromCodePage, span, toCodePage, [&](auto const& result) {
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
