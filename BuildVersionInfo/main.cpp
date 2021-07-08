#include <iostream>
#include "Config.h"

int wmain(int argc, wchar_t* argv[]) {
	Config conf(argc, argv);
	if (conf.cr == Config::run) {
		std::wcout << conf.prog << L": " << conf.inifile << L" -> " << conf.outfile << '\n';
	}
	return conf.cr == Config::error ? EXIT_FAILURE : EXIT_SUCCESS;
}
