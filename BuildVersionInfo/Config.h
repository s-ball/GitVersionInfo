#pragma once

#include <string>
#include <iostream>

using std::wstring;

class Config {
	static constexpr wchar_t def_inifile[] = L"version.ini";
	static constexpr wchar_t def_outfile[] = L"versioninfo.rc2";
public:
	wstring prog;
	wstring inifile;
	wstring outfile;
	enum {
		error = -1,
		help = 0,
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
		}
	}

private:
	void usage(std::wostream& out) {
		if (&out == &std::wcerr) {
			out << L"Syntax error\n";
		}
		out << L"Usage:\n\t" << prog << L" /H\n";
		out << L"\t\tdisplays this help message\n";
		out << L"\t" << prog << L" [/I:inifile] [/O:outfile] [outfile [inifile]]\n";
		out << L"\t\tbuilds outfile (default " << def_outfile << L") from inifile (default " << def_inifile << L")";
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
		prog = argv[0];
		size_t pos;
		if ((pos = prog.rfind('\\')) != prog.npos) {
			prog = prog.substr(pos + 1);
		}
		pos = 0;
		for (wchar_t** arg = argv + 1; *arg != nullptr; arg++) {
			if (**arg == L'/') {
				switch (towupper((*arg)[1])) {
				case L'H':
					cr = help;
					return;
				case L'I':
					inifile = getfile(arg);
					break;
				case L'O':
					outfile = getfile(arg);
					break;
				default:
					cr = error;
					return;
				}
			}
			else {
				switch (pos) {
				case 0:
					outfile = *arg;
					pos += 1;
					break;
				case 1:
					inifile = *arg;
					pos += 1;
					break;
				default:
					cr = error;
					return;
				}
			}
		}
		cr = run;
	}
};
