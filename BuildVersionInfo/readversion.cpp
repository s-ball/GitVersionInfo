#include "VersionBuilder.h"
#include <string>
#include <regex>
#include <cstdio>

bool find_tag(FILE* fd, wstring& tag, int &delta) {
	static std::wregex find_tag(L"tag: ([^,\r\n]+)");
	wchar_t line[64];
	wstring resul;
	delta = 0;
	while (NULL != fgetws(line, sizeof(line) / sizeof(line[0]), fd)) {
		int l = lstrlen(line);
		resul += line;
		if (l > 0 && line[l - 1] == L'\n') {
			std::wsmatch m;
			if (std::regex_search(resul, m, find_tag)) {
				tag = m[1].str();
				return true;
			}
			delta += 1;
			resul.clear();
		}
	}
	return false;
}

bool extract_version(const wstring& tag, LPCWSTR rxstring, WORD version[4]) {
	std::wregex rx(rxstring);
	std::wsmatch m;
	if (std::regex_search(tag, m, rx)) {
		for (size_t i = 1; i < m.size(); i++) {
			if (i >= 4) break;
			if (m[i].matched) {
				version[i - 1] = (WORD) std::stoul(m[i].str());
			}
		}
		return true;
	}
	return false;
}
wstring VersionBuilder::readVersion() {
	wstring cmd = gitCmd + L" log --format=\"%D\"";
	FILE* out = _wpopen(cmd.c_str(), L"r");
	wstring tag;
	int delta;
	if (::find_tag(out, tag, delta)) {
		if (vers.empty()) {
			if (!extract_version(tag, this->VERSION_REGEX_ALT, finfo.version)) {
				extract_version(tag, VERSION_REGEX, finfo.version);
			}
		}
		else {
			extract_version(tag, vers.c_str(), finfo.version);
		}
		finfo.version[3] = delta;
		finfo.str_version = tag;
		if (delta != 0) {
			std::wstringstream ss;
			ss << L'-' << delta;
			finfo.str_version += ss.str();
 		}
	}
	fclose(out);
	return tag;
}
