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

bool find_tag(FILE* fd, wstring& tag, int& delta);

namespace BuildTest
{
	TEST_CLASS(ConfigTest)
	{
	public:
		
		TEST_METHOD(TestSlashI)
		{
			wchar_t* args[] = { L"progname", L"/I=foo", L"bar", nullptr};
			Config conf(3, args);
			Assert::AreEqual(L"foo", conf.inifile.c_str());
			Assert::AreEqual(L"versioninfo.rc2", conf.outfile.c_str());
			Assert::AreEqual<int>(Config::run, conf.cr);
		}
		TEST_METHOD(TestSlashO)
		{
			wchar_t* args[] = { L"progname", L"bar", L"/O=foo", nullptr};
			Config conf(3, args);
			Assert::AreEqual(L"version.ini", conf.inifile.c_str());
			Assert::AreEqual(L"foo", conf.outfile.c_str());
			Assert::AreEqual<int>(Config::run, conf.cr);
		}
		TEST_METHOD(TestO)
		{
			wchar_t* args[] = { L"progname", L"bar", L"foo", nullptr};
			Config conf(3, args);
			Assert::AreEqual(L"version.ini", conf.inifile.c_str());
			Assert::AreEqual(L"foo", conf.outfile.c_str());
			Assert::AreEqual<int>(Config::run, conf.cr);
		}

		TEST_METHOD(TestSplit)
		{
			wchar_t* args[] = { L"progname", L"/I", L"=",  L"foo", L"bar",  nullptr };
			Config conf(5, args);
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
			Assert::AreEqual(L"", conf.app_file_name.c_str());
			Assert::AreEqual<int>(Config::help, conf.cr);
			Assert::AreEqual(L"Usage", ss.str().substr(0, 5).c_str());
		}
		TEST_METHOD(TestSlashX)
		{
			wchar_t* args[] = { L"progname", L"bar", L"/X", nullptr};
			std::wstringstream ss;
			std::wstreambuf* old = std::wcerr.rdbuf();
			std::wcerr.rdbuf(ss.rdbuf());
			Config conf(3, args);
			std::wcerr.rdbuf(old);
			Assert::AreEqual(L"version.ini", conf.inifile.c_str());
			Assert::AreEqual(L"versioninfo.rc2", conf.outfile.c_str());
			Assert::AreEqual<int>(Config::error, conf.cr);
			Assert::AreEqual(L"Syntax", ss.str().substr(0, 6).c_str());
		}
		TEST_METHOD(Test4args)
		{
			wchar_t* args[] = { L"progname", L"app", L"foo", L"bar", L"extra", nullptr};
			std::wstringstream ss;
			std::wstreambuf* old = std::wcerr.rdbuf();
			std::wcerr.rdbuf(ss.rdbuf());
			Config conf(5, args);
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
			Assert::AreEqual(static_cast<size_t>(2), sections.size());
		}
		TEST_METHOD(LoadSectionTest) {
			VersionBuilder builder(inifile, outfile);
			VersionBuilder::localized loc;
			builder.loadSection(L"neutral", loc, loc);
			for (int i = 0; i < VersionBuilder::NFIELDS; i++) {
				auto x = builder.as_array[i];
				std::wcout << loc.*x;
				if (builder.field_desc[i].name == L"ProductName") {
					Assert::AreEqual(L"ess app", (loc.*x).c_str());
				}
				else {
					Assert::AreEqual(static_cast<size_t>(0), (loc.*x).size());
				}
			}
		}
		TEST_METHOD(LoadSection2Test) {
			VersionBuilder builder(inifile, outfile);
			VersionBuilder::localized loc;
			builder.loadSection(L"français", loc, loc);
			for (int i = 0; i < VersionBuilder::NFIELDS; i++) {
				auto x = builder.as_array[i];
				if (builder.field_desc[i].name == L"ProductName") {
					Assert::AreEqual(L"application ess", (loc.*x).c_str());
				}
				else {
					Assert::AreEqual(static_cast<size_t>(0), (loc.*x).size());
				}
			}
		}
		TEST_METHOD(getVersionTest) {
			VersionBuilder builder(inifile, outfile);
			wstring version = builder.readVersion();
			Assert::IsTrue(version.size() > 1);
		}
		TEST_METHOD(field_in_section_2) {
			std::wofstream ini(inifile, std::ios::app);
			ini << L"legalcopyright = bar\n";
			ini.close();
			VersionBuilder builder(inifile, outfile);
			builder.getLocalSections();
			Assert::AreEqual(L"bar", builder.linfo[0].LegalCopyright.c_str());
		}
		TEST_METHOD(specialBuild) {
			std::wofstream ini(inifile, std::ios::app);
			ini << L"specialbuild = bar\n";
			ini.close();
			VersionBuilder builder(inifile, outfile);
			builder.loadFixedInfo();
			builder.getLocalSections();
			Assert::AreEqual(L"bar", builder.linfo[0].SpecialBuild.c_str());
			wstring sb = L"VS_FF_SPECIALBUILD";
			Assert::AreNotEqual(builder.finfo.flags.find(sb), builder.finfo.flags.npos);
		}
		TEST_METHOD(privateBuild) {
			std::wofstream ini(inifile, std::ios::app);
			ini << L"Privatebuild = bar\n";
			ini.close();
			VersionBuilder builder(inifile, outfile);
			builder.loadFixedInfo();
			builder.getLocalSections();
			Assert::AreEqual(L"bar", builder.linfo[0].PrivateBuild.c_str());
			wstring pb = L"VS_FF_PRIVATEBUILD";
			Assert::AreNotEqual(builder.finfo.flags.find(pb), builder.finfo.flags.npos);
		}
	};
	wstring VersionLoaderTest::inifile;
	wstring VersionLoaderTest::outfile;

	TEST_CLASS(VersionParser) {
		 FILE* file;
	public:
		TEST_METHOD_INITIALIZE(setup) {
			tmpfile_s(&file);
		}
		TEST_METHOD_CLEANUP(cleanup) {
			fclose(file);
		}
		TEST_METHOD(simpleTag) {
			fputws(L"tag: 1.2.3\n", file);
			fseek(file, 0, SEEK_SET);
			wstring tag;
			int delta;
			Assert::IsTrue(find_tag(file, tag, delta));
			Assert::AreEqual(L"1.2.3", tag.c_str());
			Assert::AreEqual(0, delta);
		}
		TEST_METHOD(secondLine) {
			fputws(L"master,/origin/master\n", file);
			fputws(L"tag: 1.2.3\n", file);
			fseek(file, 0, SEEK_SET);
			wstring tag;
			int delta;
			Assert::IsTrue(find_tag(file, tag, delta));
			Assert::AreEqual(L"1.2.3", tag.c_str());
			Assert::AreEqual(1, delta);
		}
		TEST_METHOD(longLine) {
			fputws(L"master,/origin/master\n", file);
			fputws(L"0123456789012345678901234567890123456789", file);
			fputws(L"0123456789012345678901234567890123456789", file);
			fputws(L"0123456789012345678901234567890123456789", file);
			fputws(L"0123456789012345678901234567890123456789\n", file);
			fputws(L"tag: 1.2.3\n", file);
			fseek(file, 0, SEEK_SET);
			wstring tag;
			int delta;
			Assert::IsTrue(find_tag(file, tag, delta));
			Assert::AreEqual(L"1.2.3", tag.c_str());
			Assert::AreEqual(2, delta);
		}
		TEST_METHOD(multipleRefs) {
			fputws(L"master, tag: 1.2.3,/origin/master\n", file);
			fseek(file, 0, SEEK_SET);
			wstring tag;
			int delta;
			Assert::IsTrue(find_tag(file, tag, delta));
			Assert::AreEqual(L"1.2.3", tag.c_str());
			Assert::AreEqual(0, delta);
		}
	};
	TEST_CLASS(TagParser) {
		static wstring inifile;
		static wstring outfile;
		std::ofstream out;
	public:
		TEST_CLASS_INITIALIZE(classSetup) {
			wchar_t* tmp = _wtempnam(L".", L"ini");
			inifile = tmp;
			free(tmp);
		}
		TEST_CLASS_CLEANUP(classTearOff) {
			_wremove(inifile.c_str());
		}
		TEST_METHOD_INITIALIZE(setup) {
			out.open("git.bat");
			out << "@ECHO OFF\n";
		}
		TEST_METHOD_CLEANUP(cleanup) {
			remove("git.bat");
		}
		TEST_METHOD(alt_2) {
			out << "IF \"%1\"==\"status\" GOTO end\n";
			out << "echo abc\necho def\n";
			out << "echo tag: foo2 5.12.3\n";
			out << ":end";
			out.close();
			VersionBuilder builder(inifile, outfile);
			wstring version = builder.readVersion();
			Assert::IsTrue(version.size() > 1);
			Assert::AreEqual(5, (int)builder.finfo.version[0]);
			Assert::AreEqual(12, (int)builder.finfo.version[1]);
			Assert::AreEqual(3, (int)builder.finfo.version[2]);
			Assert::AreEqual(2, (int)builder.finfo.version[3]);
			Assert::AreEqual(wstring(L"foo2 5.12.3-2"), builder.finfo.str_version);
		}
		TEST_METHOD(normal) {
			out << "IF \"%1\"==\"status\" GOTO end\n";
			out << "echo abc\necho def\n";
			out << "echo tag: foo-2.11\n";
			out << ":end";
			out.close();
			VersionBuilder builder(inifile, outfile);
			wstring version = builder.readVersion();
			Assert::IsTrue(version.size() > 1);
			Assert::AreEqual(2, (int)builder.finfo.version[0]);
			Assert::AreEqual(11, (int)builder.finfo.version[1]);
			Assert::AreEqual(0, (int)builder.finfo.version[2]);
			Assert::AreEqual(2, (int)builder.finfo.version[3]);
			Assert::AreEqual(wstring(L"foo-2.11-2"), builder.finfo.str_version);
		}
		TEST_METHOD(dirty) {
			out << "IF \"%1\"==\"status\" GOTO status\n";
			out << "echo abc\necho def\n";
			out << "echo tag: foo-2.11\n";
			out << "GOTO end\n:status\necho M  foo.cpp\n";
			out << ":end";
			out.close();
			VersionBuilder builder(inifile, outfile);
			wstring version = builder.readVersion();
			Assert::IsTrue(version.size() > 1);
			Assert::AreEqual(2, (int)builder.finfo.version[0]);
			Assert::AreEqual(11, (int)builder.finfo.version[1]);
			Assert::AreEqual(0, (int)builder.finfo.version[2]);
			Assert::AreEqual(3, (int)builder.finfo.version[3]);
			Assert::AreEqual(wstring(L"foo-2.11-3"), builder.finfo.str_version);
		}
	};
	wstring TagParser::inifile;
	wstring TagParser::outfile;

	TEST_CLASS(localizer_test) {
		VersionBuilder builder{ L"", L"" };
		TEST_METHOD(simple_init) {
			VersionBuilder::localized loc{ {0, 1252}, L"foo"};
			Assert::AreEqual(L"foo", (loc.*(builder.as_array[0])).c_str());
		}
		TEST_METHOD(init_move) {
			VersionBuilder::localized loc;
			loc = { {0, 1252}, L"foo" };
			Assert::AreEqual(L"foo", (loc.*(builder.as_array[0])).c_str());
		}
	};

	TEST_CLASS(Writer) {
		static wstring inifile;
		static wstring outfile;

		TEST_CLASS_INITIALIZE(classSetup) {
			wchar_t* tmp = _wtempnam(L".", L"out");
			outfile = tmp;
			free(tmp);
		}
		TEST_CLASS_CLEANUP(classTearOff) {
			_wremove(outfile.c_str());
		}
		TEST_METHOD(FixedNoProduct) {
			VersionBuilder builder(inifile, outfile);
			builder.finfo = { {1,2,3,4}, L"Release 1.2.3.4", L"0", L"VOS__WINDOWS32", L"VFT_APP", L"VFT2_UNKNOWN" };
			Assert::IsTrue(builder.writeFile());
			std::ifstream file(outfile);
			std::string line;
			Assert::IsTrue(static_cast<bool>(std::getline(file, line)));
			Assert::IsTrue(static_cast<bool>(std::getline(file, line)));
			Assert::IsTrue(static_cast<bool>(std::getline(file, line)));
			Assert::AreEqual(" PRODUCTVERSION\t1,2,3,4", line.c_str());
		}
		TEST_METHOD(SingleLocalProduct) {
			VersionBuilder builder(inifile, outfile);
			builder.finfo = { {1,2,3,4}, L"Release 1.2.3.4", L"0", L"VOS__WINDOWS32", L"VFT_APP", L"VFT2_UNKNOWN" };
			builder.linfo = { {0,1252} };
			builder.linfo[0].ProductVersion = L"Test 5.4";
			Assert::IsTrue(builder.writeFile());
			std::ifstream file(outfile);
			std::string line;
			Assert::IsTrue(static_cast<bool>(std::getline(file, line)));
			Assert::IsTrue(static_cast<bool>(std::getline(file, line)));
			Assert::IsTrue(static_cast<bool>(std::getline(file, line)));
			Assert::AreEqual(" PRODUCTVERSION\t5,4,0,0", line.c_str());
		}
	};
	wstring Writer::inifile;
	wstring Writer::outfile;

	TEST_CLASS(dumpTest) {
		TEST_METHOD(present) {
			VersionBuilder builder{ L"", L"" };
			std::wstringstream ss;
			builder.linfo = { {0,1252} };
			VersionBuilder::localized& loc = builder.linfo[0];
			loc.Comments = L"foo";
			builder.dumpField(ss, loc, 0);
			Assert::IsTrue(ss.good());
			std::wregex rx(L"\\s*VALUE\\s*\"Comments\"\\s*,\\s*\"foo\"\\s*\n$");
			Assert::IsTrue(std::regex_match(ss.str(), rx));
		}
		TEST_METHOD(not_required) {
			VersionBuilder builder{ L"", L"" };
			std::wstringstream ss;
			builder.linfo = { {0,1252} };
			VersionBuilder::localized& loc = builder.linfo[0];
			builder.dumpField(ss, loc, 0);
			Assert::IsTrue(ss.good());
			Assert::IsTrue(ss.str().empty());
		}
		TEST_METHOD(in_loc_0) {
			VersionBuilder builder{ L"", L"" };
			std::wstringstream ss;
			builder.linfo = { {0,1252} };
			builder.linfo[0].Comments = L"foo";
			builder.linfo.push_back(VersionBuilder::localized{ 0x409, 1200 });
			VersionBuilder::localized& loc = builder.linfo[1];
			builder.dumpField(ss, loc, 0);
			Assert::IsTrue(ss.good());
			std::wregex rx(L"\\s*VALUE\\s*\"Comments\"\\s*,\\s*\"foo\"\\s*\n$");
			Assert::IsTrue(std::regex_match(ss.str(), rx));
		}
		TEST_METHOD(required) {
			VersionBuilder builder(L"", L"", L"foo.exe");
			std::wstringstream ss;
			builder.linfo = { {0,1252} };
			VersionBuilder::localized& loc = builder.linfo[0];
			builder.dumpField(ss, loc, 1);
			Assert::IsTrue(ss.good());
			std::wregex rx(L"\\s*VALUE\\s*\"CompanyName\"\\s*,\\s*\"foo\"\\s*\n$");
			Assert::IsTrue(std::regex_match(ss.str(), rx));
		}
	};
	TEST_CLASS(testCheck) {
		wstring statusfile;

		TEST_METHOD_INITIALIZE(setup) {
			LPCWSTR temp = _wtempnam(L".", L"status");
			statusfile = temp;
			free((void *) temp);
		}
		TEST_METHOD_CLEANUP(cleanup) {
			_wremove(statusfile.c_str());
		}
		TEST_METHOD(noStatus) {
			_wremove(statusfile.c_str());
			VersionBuilder builder(L"", L"");
			builder.finfo.version[0] = 1;
			int cr = builder.check_status(statusfile);
			Assert::AreEqual(EXIT_FAILURE, cr);
			Assert::IsTrue((bool)std::ifstream(statusfile));
		}
		TEST_METHOD(versionChanged) {
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
			Assert::AreEqual(EXIT_FAILURE, cr);
			std::ifstream in(statusfile);
			in.read((char*)builder.finfo.version, sizeof(builder.finfo.version));
			Assert::AreEqual(1, (int) builder.finfo.version[2]);
		}
		TEST_METHOD(dirtyChanged) {
			VersionBuilder builder(L"", L"");
			builder.finfo.version[0] = 1;
			std::ofstream out(statusfile);
			bool dirty = true;
			out.write((const char*)builder.finfo.version, sizeof(builder.finfo.version));
			out.write((const char*)&dirty, sizeof(dirty));
			out.close();
			int cr = builder.check_status(statusfile);
			Assert::AreEqual(EXIT_FAILURE, cr);
			std::ifstream in(statusfile);
			in.read((char*)builder.finfo.version, sizeof(builder.finfo.version));
			in.read((char*)&dirty, sizeof(dirty));
			Assert::IsFalse(dirty);
		}
		TEST_METHOD(nothingChanged) {
			VersionBuilder builder(L"", L"");
			builder.finfo.version[0] = 1;
			std::ofstream out(statusfile);
			bool dirty = false;
			out.write((const char*)builder.finfo.version, sizeof(builder.finfo.version));
			out.write((const char*)&dirty, sizeof(dirty));
			out.close();
			int cr = builder.check_status(statusfile);
			Assert::AreEqual(EXIT_SUCCESS, cr);
		}
	};
}
