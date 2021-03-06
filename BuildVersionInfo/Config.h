#pragma once

#include <string>
#include <iostream>
#include <Windows.h>

using std::wstring;

class Config {
	static constexpr wchar_t def_inifile[] = L"version.ini";
	static constexpr wchar_t def_outfile[] = L"versioninfo.rc2";
public:
	wstring prog;
	wstring file;
	wstring inifile;
	wstring outfile;
	wstring app_file_name;
	enum {
		error = -1,
		help = 0,
		version,
		status,
		run
	} cr;

	Config(int argc, wchar_t* argv[]) : inifile(def_inifile), outfile(def_outfile) {
		parse(argc, argv);
		switch (cr) {
		case error:
			usage(std::wcerr);
			break;
		case help:
			usage(std::wcout);
			break;
		case version:
			usage(std::wcout, true);
		case status:
		case run:
			;
		}
	}

private:
	void usage(std::wostream& out, bool versiononly = false) {
		if (&out == &std::wcerr) {
			out << L"Syntax error\n";
		}
		else if (versiononly) {
			DWORD sz, handle;
			if (0 != (sz = GetFileVersionInfoSize(file.c_str(), &handle))) {
				UINT len;
				LPVOID data = static_cast<LPVOID>(new char[sz]);
				GetFileVersionInfo(file.c_str(), 0, sz, data);
				LPCWSTR buf;
				VerQueryValue(data, L"\\StringFileInfo\\000004b0\\InternalName", (LPVOID*)&buf, &len);
				out << buf;
				VerQueryValue(data, L"\\StringFileInfo\\000004b0\\FileVersion", (LPVOID*)&buf, &len);
				out << L" " << buf;
				VS_FIXEDFILEINFO* finfo;
				VerQueryValue(data, L"\\", (LPVOID*)&finfo, &len);
				if (finfo->dwFileFlags & VS_FF_PRERELEASE) {
					out << L" (pre-release)";
				}
				out << L'\n';
				delete[] static_cast<char*>(data);
			}
			else {
				LPCWSTR buf;
				::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, ::GetLastError(), LANG_NEUTRAL, (LPWSTR)&buf, 0, nullptr);
				out << buf << '\n';
			}
		}
		else {
			out << L"Usage:\n\t" << prog << L" /H\n";
			out << L"\t\tdisplays this help message\n";
			out << L"\t" << prog << L" /V\n";
			out << L"\t\tdisplays the version of the program\n";
			out << L"\t" << prog << L" [/I:inifile] /S [statusfile]\n";
			out << L"\t\tchecks whether version should be computed again using statusfile (default version.bin)\n\n";

			out << L"\t" << prog << L" appfilename [/I:inifile] [/O:outfile] [outfile [inifile]]\n";
			out << L"\t\tbuilds outfile (default " << def_outfile << L") from inifile (default " << def_inifile << L")";
		}
	}
	wstring getfile(wchar_t**& arg) {
		wchar_t* start = (*arg) + 2;
		if (*start == 0) {
			arg += 1;
			start = *arg;
		}
		if (*start == ':' || *start == '=') {
			start += 1;
			if (*start == 0) {
				arg += 1;
				start = *arg;
			}
		}
		return start;
	}
	void parse(int argc, wchar_t* argv[]) {
		file = argv[0];
		size_t pos;
		if ((pos = file.rfind('\\')) != file.npos) {
			prog = file.substr(pos + 1);
		}
		pos = 0;
		cr = run;
		for (wchar_t** arg = argv + 1; *arg != nullptr; arg++) {
			if (**arg == L'/') {
				switch (towupper((*arg)[1])) {
				case L'H':
				case L'?':
					cr = help;
					return;
				case L'I':
					inifile = getfile(arg);
					break;
				case L'O':
					outfile = getfile(arg);
					break;
				case L'S':
					cr = status;
					break;
				case L'V':
					cr = version;
					return;
				default:
					cr = error;
					return;
				}
			}
			else {
				switch (pos) {
				case 0:
					app_file_name = *arg;
					pos += 1;
					break;
				case 1:
					outfile = *arg;
					pos += 1;
					break;
				case 2:
					inifile = *arg;
					pos += 1;
					break;
				default:
					cr = error;
					return;
				}
			}
		}
		if (cr == run && app_file_name == L"") {
			cr = error;
		}
		if (cr == status && app_file_name == L"") {
			app_file_name = L"version.bin";
		}
	}
};
