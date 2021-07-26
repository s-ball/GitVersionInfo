#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>
#include <array>
#include <iomanip>
#include <regex>
#include <Windows.h>

using std::wstring;
using std::vector;
using std::set;
using std::hex;
using std::dec;
using std::setw;
using std::setfill;

class VersionBuilder {
public:
	static constexpr LPCWSTR FIXED = L"Fixed";
	static constexpr LPCWSTR TRANSLATION = L"Translation";
	static constexpr LPCWSTR VERSION_REGEX = L"(\\d+)(?:\\.(\\d+))?(?:\\.(\\d+))?";
	static constexpr LPCWSTR VERSION_REGEX_ALT = L"(\\d+)\\.(\\d+)\\.(\\d+)";
	struct fixed_info {
		uint16_t version[4];  // used for both FILEVERSION and PRODUCTVERSION
		wstring str_version;  // extracted from git tag
		wstring flags;        // PRIVATE_BUILD is internally handled
		wstring fileos;
		wstring filetype;
		wstring subtype;
	};
	static constexpr int NFIELDS = 12;
	struct localized {
		std::array<DWORD, 2> Translation;
		wstring Comments; 	//Additional information that should be displayed for diagnostic purposes.
		wstring CompanyName; 	// Company that produced the file ? for example, "Microsoft Corporation" or "Standard Microsystems Corporation, Inc." This string is required.
		wstring FileDescription; 	//File description to be presented to users.This string may be displayed in a list box when the user is choosing files to install ? for example, "Keyboard Driver for AT-Style Keyboards".This string is required.
		wstring FileVersion; 	//Version number of the file ? for example, "3.10" or "5.00.RC2".This string is required.
		wstring InternalName; 	//Internal name of the file, if one exists ? for example, a module name if the file is a dynamic - link library.If the file has no internal name, this string should be the original filename, without extension.This string is required.
		wstring LegalCopyright; 	//Copyright notices that apply to the file.This should include the full text of all notices, legal symbols, copyright dates, and so on.This string is optional.
		wstring LegalTrademarks; 	//Trademarksand registered trademarks that apply to the file.This should include the full text of all notices, legal symbols, trademark numbers, and so on.This string is optional.
		wstring OriginalFilename; 	//Original name of the file, not including a path.This information enables an application to determine whether a file has been renamed by a user.The format of the name depends on the file system for which the file was created.This string is required.
		wstring PrivateBuild; 	//Information about a private version of the file ? for example, "Built by TESTER1 on \TESTBED".This string should be present only if VS_FF_PRIVATEBUILD is specified in the fileflags parameter of the root block.
		wstring ProductName; 	//Name of the product with which the file is distributed.This string is required.
		wstring ProductVersion; 	//Version of the product with which the file is distributed ? for example, "3.10" or "5.00.RC2".This string is required.
		wstring SpecialBuild;
	};
	wstring localized::* as_array[NFIELDS] = {
		&localized::Comments, &localized::CompanyName, &localized::FileDescription, &localized::FileVersion,
		&localized::InternalName, &localized::LegalCopyright, &localized::LegalTrademarks,
		&localized::OriginalFilename, &localized::PrivateBuild,
		&localized::ProductName, &localized::ProductVersion, &localized::SpecialBuild
	};
	struct field_description {
		LPCWSTR name;
		bool required;
		const wstring* def;
	};
	fixed_info finfo = { {0} };
	std::vector<localized> linfo;

private:
	wstring gitCmd;

	// for read_string
	wstring inifile;
	wstring outfile;

	wstring appfile;
	wstring appname;
	bool dirty = false;
	int size = 128;
	LPWSTR buffer;
	wstring vers;

public:
	const field_description field_desc[NFIELDS] = {
		{L"Comments", false, nullptr},
		{L"CompanyName", true, &appname},
		{L"FileDescription", true, &appname},
		{L"FileVersion", true, &finfo.str_version},
		{L"InternalName", true, &appname},
		{L"LegalCopyright", false, nullptr},
		{L"LegalTrademarks", false, nullptr},
		{L"OriginalFilename", true, &appfile},
		{L"PrivateBuild", false, nullptr},
		{L"ProductName", true, &appname},
		{L"ProductVersion", true, &finfo.str_version},
		{L"SpecialBuild", false, nullptr}
	};

	
	wstring read_string(LPCWSTR section, LPCWSTR key, LPCWSTR def) {
		for (;;) {
			DWORD cr = ::GetPrivateProfileString(section, key, def, buffer, size, inifile.c_str());
			if (static_cast<int>(cr) < size - 1) {
				return wstring(buffer, cr);
			}
			delete[] buffer;
			size *= 2;
			buffer = new wchar_t[size];
		}
	}

	vector<localized> getLocalSections() {
		vector<localized> cr;
		set<std::array<WORD, 2>> translations;
		wstring sections = read_string(NULL, NULL, NULL);
		LPCWSTR section0 = sections.c_str();
		for (LPCWSTR section = section0; section < section0 + sections.size(); section += lstrlen(section) + 1) {
			if (_wcsicmp(section, FIXED) != 0 && _wcsicmp(section, L"GIT")) {
				std::array<WORD, 2> lang;
				wstring trans = read_string(section, TRANSLATION, L"0x0000, 1200");
				std::wistringstream ss(trans);
				wstring data;
				size_t pos = 0;
				if (std::getline(ss, data, L',')) {
					lang[0] = std::stoi(data, &pos, 0);
					if ((pos > 0) && std::getline(ss, data)) {
						lang[1] = std::stoi(data, &pos, 0);
					}
				}
				if ((pos > 0) && (! ss)) {
					std::wcerr << L"WARNING: section " << section << " ignored: incorrect translation >";
					std::wcerr << trans << ">\n";
					continue;
				}

				if (translations.find(lang) == translations.end()) {
					translations.insert(lang);
					localized loc = { {lang[0], lang[1]} };
					localized& loc0 = cr.empty() ? loc : cr[0];
					cr.push_back(loadSection(section, loc, loc0));
				}
				else {
					std::wcerr << L"WARNING: section " << section << " ignored: duplicated translation >";
					std::wcerr << trans << ">\n";
				}
			}
		}
		linfo = cr;
		if (!linfo.empty()) {
			// Special processing for PrivateBuild and SpecialBuild fields to ensure consistancy with FILEFLAGS
			static const std::wregex priv{ L"VS_FF_PRIVATEBUILD" };
			static const std::wregex special{ L"VS_FF_SPECIALBUILD" };
			if (!linfo[0].PrivateBuild.empty() && !std::regex_search(finfo.flags, priv)) {
				finfo.flags += L"|VS_FF_PRIVATEBUILD";
			}
			if (!linfo[0].SpecialBuild.empty() && !std::regex_search(finfo.flags, special)) {
				finfo.flags += L"|VS_FF_SPECIALBUILD";
			}
		}
		return cr;
	}

	localized & loadSection(LPCWSTR section, localized& loc, localized &loc0) {
		for (int i = 0; i < NFIELDS; i++) {
			auto x = as_array[i];
			loc.*x = read_string(section, field_desc[i].name, nullptr);
			// ensure section 0 contains all fields that exists in other sections
			if (&loc != &loc0 && !field_desc[i].required
					&& (loc0.*x).empty() && !(loc.*x).empty()) {
				loc0.*x = loc.*x;
			}
		}
		return loc;
	}

	void loadFixedInfo() {
		finfo.flags = read_string(FIXED, L"flags", L"0");
		finfo.fileos = read_string(FIXED, L"fileos", L"VOS__WINDOWS32");
		finfo.filetype = read_string(FIXED, L"filetype", L"VFT_APP");
		finfo.subtype = read_string(FIXED, L"subtype", L"VFT2_UNKNOWN");
	}

	std::wostream& dumpField(std::wostream& out, const localized& loc, int i) {
		static const wchar_t B12[] = L"            ";
		wstring field = loc.*(as_array[i]);
		if (field.empty()) field = linfo[0].*(as_array[i]);
		if (field.empty() && field_desc[i].required) {
			field = *(field_desc[i].def);
		}
		if (! field.empty()) {
			out << B12 << L"VALUE \"" << field_desc[i].name << L"\", \"";
			out << field << L"\"\n";
		}
		return out;
	}

	bool writeFile() {
		std::wofstream out(outfile);
		out << L"VS_VERSION_INFO VERSIONINFO\n";
		out << L" FILEVERSION\t" << finfo.version[0];
		for (int i = 1; i < 4; i++) out << L"," << finfo.version[i];
		out << L"\n PRODUCTVERSION\t";
		WORD productVersion[4] = { 0 };
		if (getProductVersion(productVersion)) {
			out << productVersion[0];
			for (int i = 1; i < 4; i++) out << L"," << productVersion[i];
		}
		else {
			out << finfo.version[0];
			for (int i = 1; i < 4; i++) out << L"," << finfo.version[i];
		}
		out << L"\n FILEFLAGSMASK   VS_FFI_FILEFLAGSMASK\n";
		out << L" FILEFLAGS\t" << finfo.flags;
#ifdef _DEBUG
		out << L"|VS_FF_DEBUG";
#endif // _DEBUG
		if (dirty) out << L"|VS_FF_PRERELEASE";
		out << L"\n";
		out << L" FILEOS\t" << finfo.fileos << L'\n';
		out << L" FILETYPE\t" << finfo.filetype << L'\n';
		out << L" FILESUBTYPE\t" << finfo.subtype << L'\n';
		out << L"BEGIN\n    BLOCK \"StringFileInfo\"\n    BEGIN\n";
		static const wchar_t B8[] = L"        ";
		for (localized& loc : linfo) {
			out << B8 << L"BLOCK \"" << hex << setw(4) << setfill(L'0') << loc.Translation[0];
			out << hex << setw(4) << setfill(L'0') << loc.Translation[1] << L"\"\n";
			out << B8 << L"BEGIN\n";
			for (int i = 0; i < NFIELDS; i++) {
				dumpField(out, loc, i);
			}
			out << B8 << L"END\n";
		}
		out << L"    END\n    BLOCK \"VarFileInfo\"\n    BEGIN\n";
		out << B8 << L"VALUE \"Translation\"";
		for (localized& loc : linfo) {
			out << L", 0x" << hex << loc.Translation[0];
			out << L", " << dec << loc.Translation[1];
		}
		out << L"\n    END\nEND\n";
		return (out) ? true : false;
	}

	wstring readVersion();
	int check_status(const wstring& statusfile);
	bool parseVersion(const wstring& strversion, WORD version[4]);

	bool getProductVersion(WORD version[4]) {
		for (const localized& loc : linfo) {
			if (!loc.ProductVersion.empty()) {
				if (parseVersion(loc.ProductVersion, version)) {
					return true;
				}
			}
		}
		return false;
	}

	VersionBuilder(const wstring& inifile, const wstring& outfile, const wstring& appfile = L"app") : inifile(inifile),
		outfile(outfile), appfile(appfile) {
		size_t ext = appfile.rfind(L'.');
		appname = appfile.substr(0, ext);
		buffer = new wchar_t[size];
		gitCmd = read_string(L"git", L"command", L"git");
		finfo.flags = read_string(FIXED, L"flags", L"0");
		vers = read_string(L"git", L"VERSION_TEMPLATE", NULL);
	}
};