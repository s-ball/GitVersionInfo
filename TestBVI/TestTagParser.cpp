#include "gtest/gtest.h"
#include "../BuildVersionInfo/VersionBuilder.h"

class TagParser : public ::testing::Test {
protected:
	static wstring inifile;
	static wstring outfile;

	std::ofstream out;

	static void SetUpTestCase() {
		wchar_t* tmp = _wtempnam(L".", L"ini");
		inifile = tmp;
		free(tmp);
	}
	static void TearOffTestCase() {
		_wremove(inifile.c_str());
	}
	void SetUp() override {
		out.open("git.bat");
		out << "@ECHO OFF\n";
	}
	void TearDown() override {
		remove("git.bat");
	}
};

TEST_F(TagParser, alt_2) {
	out << "IF \"%1\"==\"status\" GOTO end\n";
	out << "echo abc\necho def\n";
	out << "echo tag: foo2 5.12.3\n";
	out << ":end";
	out.close();
	VersionBuilder builder(inifile, outfile);
	wstring version = builder.readVersion();
	EXPECT_TRUE(version.size() > 1);
	EXPECT_EQ(5, (int)builder.finfo.version[0]);
	EXPECT_EQ(12, (int)builder.finfo.version[1]);
	EXPECT_EQ(3, (int)builder.finfo.version[2]);
	EXPECT_EQ(2, (int)builder.finfo.version[3]);
	EXPECT_EQ(wstring(L"foo2 5.12.3-2"), builder.finfo.str_version);
}
TEST_F(TagParser, normal) {
	out << "IF \"%1\"==\"status\" GOTO end\n";
	out << "echo abc\necho def\n";
	out << "echo tag: foo-2.11\n";
	out << ":end";
	out.close();
	VersionBuilder builder(inifile, outfile);
	wstring version = builder.readVersion();
	EXPECT_TRUE(version.size() > 1);
	EXPECT_EQ(2, (int)builder.finfo.version[0]);
	EXPECT_EQ(11, (int)builder.finfo.version[1]);
	EXPECT_EQ(0, (int)builder.finfo.version[2]);
	EXPECT_EQ(2, (int)builder.finfo.version[3]);
	EXPECT_EQ(wstring(L"foo-2.11-2"), builder.finfo.str_version);
}
TEST_F(TagParser, dirty) {
	out << "IF \"%1\"==\"status\" GOTO status\n";
	out << "echo abc\necho def\n";
	out << "echo tag: foo-2.11\n";
	out << "GOTO end\n:status\necho M  foo.cpp\n";
	out << ":end";
	out.close();
	VersionBuilder builder(inifile, outfile);
	wstring version = builder.readVersion();
	EXPECT_TRUE(version.size() > 1);
	EXPECT_EQ(2, (int)builder.finfo.version[0]);
	EXPECT_EQ(11, (int)builder.finfo.version[1]);
	EXPECT_EQ(0, (int)builder.finfo.version[2]);
	EXPECT_EQ(3, (int)builder.finfo.version[3]);
	EXPECT_EQ(wstring(L"foo-2.11-3"), builder.finfo.str_version);
}

wstring TagParser::inifile;
wstring TagParser::outfile;
