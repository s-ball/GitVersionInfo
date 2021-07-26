#include "gtest/gtest.h"
#include "../BuildVersionInfo/Config.h"

TEST(ConfigTest, TestSlashI)
{
	wchar_t* args[] = { L"progname", L"/I=foo", L"bar", nullptr };
	Config conf(3, args);
	EXPECT_EQ(L"foo", conf.inifile);
	EXPECT_EQ(L"versioninfo.rc2", conf.outfile);
	EXPECT_EQ(Config::run, conf.cr);
}
TEST(ConfigTest, TestSlashO)
{
	wchar_t* args[] = { L"progname", L"bar", L"/O=foo", nullptr };
	Config conf(3, args);
	EXPECT_EQ(L"version.ini", conf.inifile);
	EXPECT_EQ(L"foo", conf.outfile);
	EXPECT_EQ(Config::run, conf.cr);
}
TEST(ConfigTest, TestO)
{
	wchar_t* args[] = { L"progname", L"bar", L"foo", nullptr };
	Config conf(3, args);
	EXPECT_EQ(L"version.ini", conf.inifile);
	EXPECT_EQ(L"foo", conf.outfile);
	EXPECT_EQ(Config::run, conf.cr);
}

TEST(ConfigTest, TestSplit)
{
	wchar_t* args[] = { L"progname", L"/I", L"=",  L"foo", L"bar",  nullptr };
	Config conf(5, args);
	EXPECT_EQ(L"foo", conf.inifile);
	EXPECT_EQ(L"versioninfo.rc2", conf.outfile);
	EXPECT_EQ(Config::run, conf.cr);
}
TEST(ConfigTest, TestSlashH)
{
	wchar_t* args[] = { L"progname", L"/H", nullptr };
	std::wstringstream ss;
	std::wstreambuf* old = std::wcout.rdbuf();
	std::wcout.rdbuf(ss.rdbuf());
	Config conf(2, args);
	std::wcout.rdbuf(old);
	EXPECT_EQ(L"version.ini", conf.inifile);
	EXPECT_EQ(L"versioninfo.rc2", conf.outfile);
	EXPECT_EQ(L"", conf.app_file_name);
	EXPECT_EQ(Config::help, conf.cr);
	EXPECT_EQ(L"Usage", ss.str().substr(0, 5));
}
TEST(ConfigTest, TestSlashX)
{
	wchar_t* args[] = { L"progname", L"bar", L"/X", nullptr };
	std::wstringstream ss;
	std::wstreambuf* old = std::wcerr.rdbuf();
	std::wcerr.rdbuf(ss.rdbuf());
	Config conf(3, args);
	std::wcerr.rdbuf(old);
	EXPECT_EQ(L"version.ini", conf.inifile);
	EXPECT_EQ(L"versioninfo.rc2", conf.outfile);
	EXPECT_EQ(Config::error, conf.cr);
	EXPECT_EQ(L"Syntax", ss.str().substr(0, 6));
}
TEST(ConfigTest, Test4args)
{
	wchar_t* args[] = { L"progname", L"app", L"foo", L"bar", L"extra", nullptr };
	std::wstringstream ss;
	std::wstreambuf* old = std::wcerr.rdbuf();
	std::wcerr.rdbuf(ss.rdbuf());
	Config conf(5, args);
	std::wcerr.rdbuf(old);
	EXPECT_EQ(Config::error, conf.cr);
	EXPECT_EQ(L"Syntax", ss.str().substr(0, 6));
}
