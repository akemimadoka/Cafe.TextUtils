#include <Cafe/TextUtils/CodePointIterator.h>
#include <catch2/catch_all.hpp>
#include <cstring>

using namespace Cafe;
using namespace TextUtils;

TEST_CASE("Cafe.TextUtils.Misc", "[TextUtils][Misc]")
{
	SECTION("CodePointIterator")
	{
		constexpr auto testString = CAFE_UTF8_SV("测试");
		CodePointIterator<Encoding::CodePage::Utf8> read{ testString.GetSpan() };
		const CodePointIterator<Encoding::CodePage::Utf8> end{};
		CHECK((*read).first == 0x6D4B);
		++read;
		CHECK((*read).first == 0x8BD5);
		++read;
		CHECK((*read).first == 0);
		++read;
		CHECK(read == end);
	}

	SECTION("String convertion")
	{
		const auto codePointString = EncodeTo<Encoding::CodePage::CodePoint>(CAFE_UTF8_SV("测试"));
		REQUIRE(codePointString.GetSize() == 3);
		CHECK(codePointString[0] == 0x6D4B);
		CHECK(codePointString[1] == 0x8BD5);
		CHECK(codePointString[2] == 0);
	}
}
