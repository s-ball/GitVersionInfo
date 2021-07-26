#include "gtest/gtest.h"
#include "../BuildVersionInfo/VersionBuilder.h"

bool find_tag(FILE* fd, wstring& tag, int& delta);

class TestParser : public ::testing::Test {
protected:
	FILE* file;

	void SetUp() override{
		tmpfile_s(&file);
	}
	void TearDown() override {
		fclose(file);
	}
};

TEST_F(TestParser, simpleTag) {
	fputws(L"tag: 1.2.3\n", file);
	fseek(file, 0, SEEK_SET);
	wstring tag;
	int delta;
	EXPECT_TRUE(find_tag(file, tag, delta));
	EXPECT_EQ(L"1.2.3", tag);
	EXPECT_EQ(0, delta);
}
TEST_F(TestParser, secondLine) {
	fputws(L"master,/origin/master\n", file);
	fputws(L"tag: 1.2.3\n", file);
	fseek(file, 0, SEEK_SET);
	wstring tag;
	int delta;
	EXPECT_TRUE(find_tag(file, tag, delta));
	EXPECT_EQ(L"1.2.3", tag);
	EXPECT_EQ(1, delta);
}
TEST_F(TestParser, longLine) {
	fputws(L"master,/origin/master\n", file);
	fputws(L"0123456789012345678901234567890123456789", file);
	fputws(L"0123456789012345678901234567890123456789", file);
	fputws(L"0123456789012345678901234567890123456789", file);
	fputws(L"0123456789012345678901234567890123456789\n", file);
	fputws(L"tag: 1.2.3\n", file);
	fseek(file, 0, SEEK_SET);
	wstring tag;
	int delta;
	EXPECT_TRUE(find_tag(file, tag, delta));
	EXPECT_EQ(L"1.2.3", tag);
	EXPECT_EQ(2, delta);
}
TEST_F(TestParser, multipleRefs) {
	fputws(L"master, tag: 1.2.3,/origin/master\n", file);
	fseek(file, 0, SEEK_SET);
	wstring tag;
	int delta;
	EXPECT_TRUE(find_tag(file, tag, delta));
	EXPECT_EQ(L"1.2.3", tag);
	EXPECT_EQ(0, delta);
}
