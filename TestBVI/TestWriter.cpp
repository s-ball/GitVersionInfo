#include "gtest/gtest.h"
#include "../BuildVersionInfo/VersionBuilder.h"

class TestWriter : public ::testing::Test {
protected:
	static wstring inifile;
	static wstring outfile;

	static void SetUpTestCase() {
		wchar_t* tmp = _wtempnam(L".", L"out");
		outfile = tmp;
		free(tmp);
	}
	static void TearDownTestCase() {
		_wremove(outfile.c_str());
	}
};

TEST_F(TestWriter, FixedNoProduct) {
	VersionBuilder builder(inifile, outfile);
	builder.finfo = { {1,2,3,4}, L"Release 1.2.3.4", L"0", L"VOS__WINDOWS32", L"VFT_APP", L"VFT2_UNKNOWN" };
	EXPECT_TRUE(builder.writeFile());
	std::ifstream file(outfile);
	std::string line;
	EXPECT_TRUE(static_cast<bool>(std::getline(file, line)));
	EXPECT_TRUE(static_cast<bool>(std::getline(file, line)));
	EXPECT_TRUE(static_cast<bool>(std::getline(file, line)));
	EXPECT_EQ(" PRODUCTVERSION\t1,2,3,4", line);
}
TEST_F(TestWriter, SingleLocalProduct) {
	VersionBuilder builder(inifile, outfile);
	builder.finfo = { {1,2,3,4}, L"Release 1.2.3.4", L"0", L"VOS__WINDOWS32", L"VFT_APP", L"VFT2_UNKNOWN" };
	builder.linfo = { {0,1252} };
	builder.linfo[0].ProductVersion = L"Test 5.4";
	EXPECT_TRUE(builder.writeFile());
	std::ifstream file(outfile);
	std::string line;
	EXPECT_TRUE(static_cast<bool>(std::getline(file, line)));
	EXPECT_TRUE(static_cast<bool>(std::getline(file, line)));
	EXPECT_TRUE(static_cast<bool>(std::getline(file, line)));
	EXPECT_EQ(" PRODUCTVERSION\t5,4,0,0", line);
}

wstring TestWriter::inifile;
wstring TestWriter::outfile;

TEST(TestDump, present) {
	VersionBuilder builder{ L"", L"" };
	std::wstringstream ss;
	builder.linfo = { {0,1252} };
	VersionBuilder::localized& loc = builder.linfo[0];
	loc.Comments = L"foo";
	builder.dumpField(ss, loc, 0);
	EXPECT_TRUE(ss.good());
	std::wregex rx(L"\\s*VALUE\\s*\"Comments\"\\s*,\\s*\"foo\"\\s*\n$");
	EXPECT_TRUE(std::regex_match(ss.str(), rx));
}
TEST(TestDump, not_required) {
	VersionBuilder builder{ L"", L"" };
	std::wstringstream ss;
	builder.linfo = { {0,1252} };
	VersionBuilder::localized& loc = builder.linfo[0];
	builder.dumpField(ss, loc, 0);
	EXPECT_TRUE(ss.good());
	EXPECT_TRUE(ss.str().empty());
}
TEST(TestDump, in_loc_0) {
	VersionBuilder builder{ L"", L"" };
	std::wstringstream ss;
	builder.linfo = { {0,1252} };
	builder.linfo[0].Comments = L"foo";
	builder.linfo.push_back(VersionBuilder::localized{ 0x409, 1200 });
	VersionBuilder::localized& loc = builder.linfo[1];
	builder.dumpField(ss, loc, 0);
	EXPECT_TRUE(ss.good());
	std::wregex rx(L"\\s*VALUE\\s*\"Comments\"\\s*,\\s*\"foo\"\\s*\n$");
	EXPECT_TRUE(std::regex_match(ss.str(), rx));
}
TEST(TestDump, required) {
	VersionBuilder builder(L"", L"", L"foo.exe");
	std::wstringstream ss;
	builder.linfo = { {0,1252} };
	VersionBuilder::localized& loc = builder.linfo[0];
	builder.dumpField(ss, loc, 1);
	EXPECT_TRUE(ss.good());
	std::wregex rx(L"\\s*VALUE\\s*\"CompanyName\"\\s*,\\s*\"foo\"\\s*\n$");
	EXPECT_TRUE(std::regex_match(ss.str(), rx));
}
