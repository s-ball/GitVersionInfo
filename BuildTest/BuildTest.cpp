#include "pch.h"
#include "CppUnitTest.h"
#include "Config.h"
#include <sstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BuildTest
{
	TEST_CLASS(ConfigTest)
	{
	public:
		
		TEST_METHOD(TestSlashI)
		{
			wchar_t* args[] = { L"progname", L"/I=foo", nullptr };
			Config conf(2, args);
			Assert::AreEqual(L"foo", conf.inifile.c_str());
			Assert::AreEqual(L"versioninfo.rc2", conf.outfile.c_str());
			Assert::AreEqual<int>(Config::run, conf.cr);
		}
		TEST_METHOD(TestSlashO)
		{
			wchar_t* args[] = { L"progname", L"/O=foo", nullptr };
			Config conf(2, args);
			Assert::AreEqual(L"version.ini", conf.inifile.c_str());
			Assert::AreEqual(L"foo", conf.outfile.c_str());
			Assert::AreEqual<int>(Config::run, conf.cr);
		}
		TEST_METHOD(TestO)
		{
			wchar_t* args[] = { L"progname", L"foo", nullptr };
			Config conf(2, args);
			Assert::AreEqual(L"version.ini", conf.inifile.c_str());
			Assert::AreEqual(L"foo", conf.outfile.c_str());
			Assert::AreEqual<int>(Config::run, conf.cr);
		}

		TEST_METHOD(TestSplit)
		{
			wchar_t* args[] = { L"progname", L"/I", L"=",  L"foo", nullptr };
			Config conf(4, args);
			Assert::AreEqual(L"foo", conf.inifile.c_str());
			Assert::AreEqual(L"versioninfo.rc2", conf.outfile.c_str());
			Assert::AreEqual<int>(Config::run, conf.cr);
		}
		TEST_METHOD(TestSlashH)
		{
			wchar_t* args[] = { L"progname", L"/H", nullptr };
			std::wstringstream ss;
			std::wstreambuf* old = std::wcout.rdbuf();
			std::wcout.rdbuf(ss.rdbuf());
			Config conf(2, args);
			std::wcout.rdbuf(old);
			Assert::AreEqual(L"version.ini", conf.inifile.c_str());
			Assert::AreEqual(L"versioninfo.rc2", conf.outfile.c_str());
			Assert::AreEqual<int>(Config::help, conf.cr);
			Assert::AreEqual(L"Usage", ss.str().substr(0, 5).c_str());
		}
		TEST_METHOD(TestSlashX)
		{
			wchar_t* args[] = { L"progname", L"/X", nullptr };
			std::wstringstream ss;
			std::wstreambuf* old = std::wcerr.rdbuf();
			std::wcerr.rdbuf(ss.rdbuf());
			Config conf(2, args);
			std::wcerr.rdbuf(old);
			Assert::AreEqual(L"version.ini", conf.inifile.c_str());
			Assert::AreEqual(L"versioninfo.rc2", conf.outfile.c_str());
			Assert::AreEqual<int>(Config::error, conf.cr);
			Assert::AreEqual(L"Syntax", ss.str().substr(0, 6).c_str());
		}
		TEST_METHOD(Test3args)
		{
			wchar_t* args[] = { L"progname", L"foo", L"bar", L"extra", nullptr};
			std::wstringstream ss;
			std::wstreambuf* old = std::wcerr.rdbuf();
			std::wcerr.rdbuf(ss.rdbuf());
			Config conf(2, args);
			std::wcerr.rdbuf(old);
			Assert::AreEqual<int>(Config::error, conf.cr);
			Assert::AreEqual(L"Syntax", ss.str().substr(0, 6).c_str());
		}
	};
}
