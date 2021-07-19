#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>
#include <Windows.h>

using std::wstring;
using std::vector;
using std::set;

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
		DWORD Translation[2];
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
		wstring* as_array[NFIELDS] = {
			&Comments, &CompanyName, &FileDescription, &FileVersion,
			&InternalName, &LegalCopyright, &LegalTrademarks, &OriginalFilename, &PrivateBuild,
			&ProductName, &ProductVersion, &SpecialBuild
		};
	};
	static constexpr LPCWSTR fieldnames[NFIELDS] = {
			L"Comments", L"CompanyName", L"FileDescription", L"FileVersion",
			L"InternalName", L"LegalCopyright", L"LegalTrademarks", L"OriginalFilename",
			L"PrivateBuild", L"ProductName", L"ProductVersion", L"SpecialBuild"
	};
	fixed_info finfo;

private:
	wstring gitCmd;

	// for read_string
	wstring inifile;
	wstring outfile;
	int size = 128;
	LPWSTR buffer;
	wstring vers;

public:
/*			FILEOS 0x40004L
			FILETYPE 0x1L
			FILESUBTYPE 0x0L
			BEGIN
			BLOCK "StringFileInfo"
			BEGIN
			BLOCK "000004b0"
			BEGIN
			VALUE "CompanyName", "SBA.ORG"
			VALUE "FileDescription", "Simple Demo"
			VALUE "FileVersion", "1.0.0.1"
			VALUE "InternalName", "SimpleWi.exe"
			VALUE "LegalCopyright", "Copyright (C) 2021"
			VALUE "OriginalFilename", "SimpleWi.exe"
			VALUE "ProductName", "Simple Demo"
			VALUE "ProductVersion", "1.0.0.1"
			END
			END
			BLOCK "VarFileInfo"
			BEGIN
			VALUE "Translation", 0x0, 1200
			END
			END
			*/

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
		set<std::pair<DWORD, DWORD>> translations;
		wstring sections = read_string(NULL, NULL, NULL);
		LPCWSTR section0 = sections.c_str();
		for (LPCWSTR section = section0; section < section0 + sections.size(); section += lstrlen(section) + 1) {
			if (_wcsicmp(section, FIXED) != 0) {
				std::pair<DWORD, DWORD> lang;
				wstring trans = read_string(section, TRANSLATION, L"0x0000, 1252");
				std::wistringstream ss(trans);
				wstring data;
				size_t pos = 0;
				if (std::getline(ss, data, L',')) {
					lang.first = std::stoi(data, &pos, 0);
					if ((pos > 0) && std::getline(ss, data)) {
						lang.second = std::stoi(data, &pos, 0);
					}
				}
				if ((pos > 0) && (! ss)) {
					std::wcerr << L"WARNING: section " << section << " ignored: incorrect translation >";
					std::wcerr << trans << ">\n";
					continue;
				}

				if (translations.find(lang) == translations.end()) {
					translations.insert(lang);
					localized loc = {{lang.first, lang.second}};
					cr.push_back(loadSection(section, loc));
				}
				else {
					std::wcerr << L"WARNING: section " << section << " ignored: duplicated translation >";
					std::wcerr << trans << ">\n";
				}
			}
		}
		return cr;
	}

	localized & loadSection(LPCWSTR section, localized& loc) {
		for (int i = 0; i < NFIELDS; i++) {
			*loc.as_array[i] = read_string(section, fieldnames[i], nullptr);
		}
		return loc;
	}

	void loadFixedInfo() {
		finfo.flags = read_string(FIXED, L"flags", L"0");
		finfo.fileos = read_string(FIXED, L"fileos", L"VOS__WINDOWS32");
		finfo.filetype = read_string(FIXED, L"filetype", L"VFT_APP");
		finfo.subtype = read_string(FIXED, L"subtype", L"VFT2_UNKNOWN");
	}

	bool writeFile(const wstring& outfile) {
		std::wofstream out(outfile);
		out << L"VS_VERSION_INFO VERSIONINFO\n";
		out << L" FILEVERSION\t" << finfo.version[0];
		for (int i = 1; i < 4; i++) out << L"," << finfo.version[i];
		out << L" PRODUCTVERSION\t" << finfo.version[0];
		for (int i = 1; i < 4; i++) out << L"," << finfo.version[i];
		out << L" FILEFLAGSMASK   VS_FFI_FILEFLAGSMASK\n";
		out << L" FILEFLAGS\t" << finfo.flags;
#ifdef DEBUG
		out << L"|VER_DEBUG";
#endif // DEBUG
		out << L"\n";
		out << L" FILEOS\t" << finfo.fileos << L'\n';
		out << L" FILETYPE\t" << finfo.filetype << L'\n';
		out << L" FILESUBTYPE\t" << finfo.subtype << L'\n';
		out << L"BEGIN\n";
		out << L"END\n";
	}

	wstring readVersion();

	VersionBuilder(const wstring& inifile, const wstring& outfile) : inifile(inifile) {
		buffer = new wchar_t[size];
		gitCmd = read_string(L"git", L"command", L"git");
		finfo.flags = read_string(FIXED, L"flags", L"0");
		vers = read_string(L"git", L"VERSION_TEMPLATE", NULL);
	}
};