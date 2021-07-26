#include "gtest/gtest.h"
#include "../BuildVersionInfo/VersionBuilder.h"

class TestCheck : public ::testing::Test {
protected:
	wstring statusfile;

	void SetUp() override {
		LPCWSTR temp = _wtempnam(L".", L"status");
		statusfile = temp;
		free((void*)temp);
	}
	void TearDown() override {
		_wremove(statusfile.c_str());
	}
};

TEST_F(TestCheck, noStatus) {
	_wremove(statusfile.c_str());
	VersionBuilder builder(L"", L"");
	builder.finfo.version[0] = 1;
	int cr = builder.check_status(statusfile);
	EXPECT_EQ(EXIT_FAILURE, cr);
	EXPECT_TRUE((bool)std::ifstream(statusfile));
}
TEST_F(TestCheck, versionChanged) {
	VersionBuilder builder(L"", L"");
	builder.finfo.version[0] = 1;
	std::ofstream out(statusfile);
	bool dirty = false;
	out.write((const char*)builder.finfo.version, sizeof(builder.finfo.version));
	out.write((const char*)&dirty, sizeof(dirty));
	out.close();
	builder.finfo.version[2] = 1;
	int cr = builder.check_status(statusfile);
	builder.finfo.version[2] = 2;
	EXPECT_EQ(EXIT_FAILURE, cr);
	std::ifstream in(statusfile);
	in.read((char*)builder.finfo.version, sizeof(builder.finfo.version));
	EXPECT_EQ(1, (int) builder.finfo.version[2]);
}
TEST_F(TestCheck, dirtyChanged) {
	VersionBuilder builder(L"", L"");
	builder.finfo.version[0] = 1;
	std::ofstream out(statusfile);
	bool dirty = true;
	out.write((const char*)builder.finfo.version, sizeof(builder.finfo.version));
	out.write((const char*)&dirty, sizeof(dirty));
	out.close();
	int cr = builder.check_status(statusfile);
	EXPECT_EQ(EXIT_FAILURE, cr);
	std::ifstream in(statusfile);
	in.read((char*)builder.finfo.version, sizeof(builder.finfo.version));
	in.read((char*)&dirty, sizeof(dirty));
	EXPECT_FALSE(dirty);
}
TEST_F(TestCheck, nothingChanged) {
	VersionBuilder builder(L"", L"");
	builder.finfo.version[0] = 1;
	std::ofstream out(statusfile);
	bool dirty = false;
	out.write((const char*)builder.finfo.version, sizeof(builder.finfo.version));
	out.write((const char*)&dirty, sizeof(dirty));
	out.close();
	int cr = builder.check_status(statusfile);
	EXPECT_EQ(EXIT_SUCCESS, cr);
}
