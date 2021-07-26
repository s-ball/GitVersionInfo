#include "gtest/gtest.h"
#include "../BuildVersionInfo/VersionBuilder.h"

class TestLoader : public ::testing::Test {
protected:
	static wstring inifile;
	static wstring outfile;

	static void SetUpTestCase() {
		wchar_t* tmp = _wtempnam(L".", L"ini");
		inifile = tmp;
		free(tmp);
		std::wofstream ini(inifile);
		ini << L"; top comment\n[fixed]  ; section name comment\n; line comment\nFOO=bar biz baz\n";
		ini << L"filetype = VFT_APP\n\n" << L"[neutral]\nProductname=ess app\n";
		ini << L"[français]\nTranslation= 0x040c, 0x4e4\nproductName = application ess     \n";
	}

	static void TearDownTestCase() {
		_wremove(inifile.c_str());
	}
};
/*
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
*/
TEST_F(TestLoader, LocalSectionsTest) {
	VersionBuilder builder(inifile, outfile);
	vector<VersionBuilder::localized> sections = builder.getLocalSections();
	EXPECT_EQ(static_cast<size_t>(2), sections.size());
}
TEST_F(TestLoader, LoadSectionTest) {
	VersionBuilder builder(inifile, outfile);
	VersionBuilder::localized loc;
	builder.loadSection(L"neutral", loc, loc);
	for (int i = 0; i < VersionBuilder::NFIELDS; i++) {
		auto x = builder.as_array[i];
		if (0 == lstrcmp(builder.field_desc[i].name, L"ProductName")) {
			EXPECT_EQ(L"ess app", (loc.*x));
		}
		else {
			if (!(loc.*x).empty()) {
				std::wcerr << loc.*x << "\n";
			}
			EXPECT_EQ(static_cast<size_t>(0), (loc.*x).size());
		}
	}
}
TEST_F(TestLoader, LoadSection2Test) {
	VersionBuilder builder(inifile, outfile);
	VersionBuilder::localized loc;
	builder.loadSection(L"français", loc, loc);
	for (int i = 0; i < VersionBuilder::NFIELDS; i++) {
		auto x = builder.as_array[i];
		if (0 == lstrcmp(builder.field_desc[i].name, L"ProductName")) {
			EXPECT_EQ(L"application ess", (loc.*x));
		}
		else {
			if (!(loc.*x).empty()) {
				std::wcerr << loc.*x << "\n";
			}
			EXPECT_EQ(static_cast<size_t>(0), (loc.*x).size());
		}
	}
}
TEST_F(TestLoader, getVersionTest) {
	VersionBuilder builder(inifile, outfile);
	wstring version = builder.readVersion();
	EXPECT_TRUE(version.size() > 1);
}
TEST_F(TestLoader, field_in_section_2) {
	std::wofstream ini(inifile, std::ios::app);
	ini << L"legalcopyright = bar\n";
	ini.close();
	VersionBuilder builder(inifile, outfile);
	builder.getLocalSections();
	EXPECT_EQ(L"bar", builder.linfo[0].LegalCopyright);
}
TEST_F(TestLoader, specialBuild) {
	std::wofstream ini(inifile, std::ios::app);
	ini << L"specialbuild = bar\n";
	ini.close();
	VersionBuilder builder(inifile, outfile);
	builder.loadFixedInfo();
	builder.getLocalSections();
	EXPECT_EQ(L"bar", builder.linfo[0].SpecialBuild);
	wstring sb = L"VS_FF_SPECIALBUILD";
	EXPECT_NE(builder.finfo.flags.find(sb), builder.finfo.flags.npos);
}
TEST_F(TestLoader, privateBuild) {
	std::wofstream ini(inifile, std::ios::app);
	ini << L"Privatebuild = bar\n";
	ini.close();
	VersionBuilder builder(inifile, outfile);
	builder.loadFixedInfo();
	builder.getLocalSections();
	EXPECT_EQ(L"bar", builder.linfo[0].PrivateBuild);
	wstring pb = L"VS_FF_PRIVATEBUILD";
	EXPECT_NE(builder.finfo.flags.find(pb), builder.finfo.flags.npos);
}

wstring TestLoader::inifile;
wstring TestLoader::outfile;
