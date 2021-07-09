#include "pch.h"
#include "CppUnitTest.h"
#include "Config.h"
#include "VersionBuilder.h"
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <vector>

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

	TEST_CLASS(VersionLoaderTest) {
		static wstring inifile;
		static wstring outfile;
	public:
		TEST_CLASS_INITIALIZE(classSetup) {
			wchar_t* tmp = _wtempnam(L".", L"ini");
			inifile = tmp;
			free(tmp);
			std::wofstream ini(inifile);
			ini << L"; top comment\n[fixed]  ; section name comment\n; line comment\nFOO=bar biz baz\n";
			ini << L"filetype = VFT_APP\n\n" << L"[neutral]\nProductname=ess app\n";
			ini << L"[français]\nTranslation= 0x040c, 0x4e4\nproductName = application ess     \n";
		}
		TEST_CLASS_CLEANUP(classTearOff) {
			_wremove(inifile.c_str());
		}
		TEST_METHOD(LocalSectionsTest) {
			VersionBuilder builder(inifile, outfile);
			vector<VersionBuilder::localized> sections = builder.getLocalSections();
			Assert::AreEqual(2u, sections.size());
		}
		TEST_METHOD(LoadSectionTest) {
			VersionBuilder builder(inifile, outfile);
			VersionBuilder::localized loc;
			builder.loadSection(L"neutral", loc);
			for (int i = 0; i < VersionBuilder::NFIELDS; i++) {
				if (VersionBuilder::fieldnames[i] == L"ProductName") {
					Assert::AreEqual(L"ess app", loc.as_array[i]->c_str());
				}
				else {
					Assert::AreEqual(0u, loc.as_array[i]->size());
				}
			}
		}
		TEST_METHOD(LoadSection2Test) {
			VersionBuilder builder(inifile, outfile);
			VersionBuilder::localized loc;
			builder.loadSection(L"français", loc);
			for (int i = 0; i < VersionBuilder::NFIELDS; i++) {
				if (VersionBuilder::fieldnames[i] == L"ProductName") {
					Assert::AreEqual(L"application ess", loc.as_array[i]->c_str());
				}
				else {
					Assert::AreEqual(0u, loc.as_array[i]->size());
				}
			}
		}
		TEST_METHOD(getVersionTest) {
			VersionBuilder builder(inifile, outfile);
			wstring version = builder.readVersion();
			Assert::IsTrue(version.size() > 1);
		}
	};
	wstring VersionLoaderTest::inifile;
	wstring VersionLoaderTest::outfile;
}
