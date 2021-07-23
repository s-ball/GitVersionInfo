#include <iostream>
#include "Config.h"
#include "VersionBuilder.h"

int wmain(int argc, wchar_t* argv[]) {
	Config conf(argc, argv);
	if (conf.cr == Config::status) {
		VersionBuilder builder(conf.inifile, conf.outfile, conf.app_file_name);
		builder.readVersion();
		return builder.check_status(conf.app_file_name);
	}
	if (conf.cr == Config::run) {
		std::wcout << conf.prog << L": " << conf.inifile << L" -> " << conf.outfile << '\n';
		if (conf.cr == Config::error) {
			return EXIT_FAILURE;
		}
		VersionBuilder builder(conf.inifile, conf.outfile, conf.app_file_name);
		builder.readVersion();
		builder.loadFixedInfo();
		builder.getLocalSections();
		builder.writeFile();
	}
	return EXIT_SUCCESS;
}
