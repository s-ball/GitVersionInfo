#include "gtest/gtest.h"
#include "../BuildVersionInfo/VersionBuilder.h"

TEST(TestLocalized, simple_init) {
	VersionBuilder builder{ L"", L"" };
	VersionBuilder::localized loc{ {0, 1252}, L"foo"};
	EXPECT_EQ(L"foo", (loc.*(builder.as_array[0])));
}
TEST(TestLocalized, init_move) {
	VersionBuilder builder{ L"", L"" };
	VersionBuilder::localized loc;
	loc = { {0, 1252}, L"foo" };
	EXPECT_EQ(L"foo", (loc.*(builder.as_array[0])));
}
