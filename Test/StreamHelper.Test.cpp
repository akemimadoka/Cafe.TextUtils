#include <Cafe/Io/Streams/MemoryStream.h>
#include <Cafe/TextUtils/TextReader.h>
#include <Cafe/TextUtils/TextWriter.h>
#include <catch2/catch.hpp>

using namespace Cafe;
using namespace Encoding;
using namespace TextUtils;
using namespace Io;

TEST_CASE("Cafe.TextUtils.StreamHelper", "[TextUtils][StreamHelper]")
{
	SECTION("StreamHelpers")
	{
		constexpr auto TestString = CAFE_UTF8_SV("测试");

		MemoryStream stream;
		TextWriter<CodePage::Utf8> writer{ &stream };
		writer.WriteLine(TestString);
		writer.Flush();

		const auto internalStorage = stream.GetInternalStorage();
		REQUIRE(std::memcmp(internalStorage.data(), TestString.GetData(), TestString.GetSize() - 1) ==
		        0);
		REQUIRE(internalStorage[internalStorage.size() - 1] == static_cast<std::byte>('\n'));

		stream.SeekFromBegin(0);

		TextReader<CodePage::Utf8> reader{ &stream };
		const auto line = reader.ReadLine();

		REQUIRE(line == TestString);
	}
}
