#include "VersionBuilder.h"
#include <fstream>
#include <memory.h>


using std::ios;

int VersionBuilder::check_status(const wstring& statusfile) {
	uint16_t version[4] = { 0 };
	bool dirty = false;
	int cr = EXIT_SUCCESS;
	std::fstream st(statusfile, std::ios::binary | std::ios::in);
	if (!st) {
		cr = EXIT_FAILURE;
	}
	else {
		st.read((char*)version, sizeof(version));
		st.read((char*)&dirty, sizeof(dirty));
		if (dirty != this->dirty || 0 != memcmp(version, finfo.version, sizeof(version))) {
			cr = EXIT_FAILURE;
		}
	}
	if (cr != EXIT_SUCCESS) {
		if (st) {
			st.close();
		}
		st.open(statusfile, std::ios::binary | std::ios::out | std::ios::trunc);
		if (st) {
			st.write((const char*)finfo.version, sizeof(version));
			st.write((const char*)&this->dirty, sizeof(dirty));
		}
	}
	return cr;
}