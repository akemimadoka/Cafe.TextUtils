#include <Cafe/TextUtils/Format.h>
#include <catch2/catch.hpp>

using namespace Cafe;
using namespace TextUtils;

TEST_CASE("Cafe.TextUtils.Format", "[TextUtils][Format]")
{
	SECTION("Formatting")
	{
		const auto formattedString = FormatString(CAFE_UTF8_SV("${0}, ${1}, ${3:x}, ${2}"), 1, 2.5f, -3, 18);
		REQUIRE(formattedString.GetView() == CAFE_UTF8_SV("1, 2.50000, 12, -3"));
	}
}
