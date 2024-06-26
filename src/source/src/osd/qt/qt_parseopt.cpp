/** @file qt_parseopt.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.05.18 -

	@brief [ parse options ]
*/

#include "qt_parseopt.h"
#include "../../gui/gui.h"
#include "../../config.h"
#include "../../vm/vm.h"
#include "../../utility.h"
//#include "qt_main.h"
#include "../../depend.h"
#include <QCommandLineParser>

CParseOptions::CParseOptions()
	: CParseOptionsBase()
{

}

CParseOptions::CParseOptions(const CParseOptions &src)
	: CParseOptionsBase(src)
{

}

CParseOptions::CParseOptions(int ac, char *av[])
	: CParseOptionsBase()
{
	get_options(ac, av);
}

CParseOptions::~CParseOptions()
{
}

/**
 *	get and parse options
 */
int CParseOptions::get_options(int ac, char *av[])
{
//	QStringList args = qApp->arguments();
#ifdef _DEBUG
	for(int i=0; i<ac; i++) {
		printf("%d %s\n",i,av[i]);
	}
#endif

	// allocate buffer
	allocate_buffers();

//	QTChar arg0(args[0]);
	set_application_path_and_name(av[0]);

	// resource path
	UTILITY::tcscat(tmp_path_1, _MAX_PATH, _T(RESDIR));
	UTILITY::slim_path(tmp_path_1, tmp_path_2, _MAX_PATH);
	res_path.Set(tmp_path_2);

	int optind = 1;
#if defined(__APPLE__) && defined(__MACH__)
	// When mac, app_path set upper app folder (ex. foo/bar/baz.app/../)
    UTILITY::tcsncpy(tmp_path_1, _MAX_PATH, app_path.Get(), _MAX_PATH);
	UTILITY::add_path_separator(tmp_path_1);
	UTILITY::slim_path(tmp_path_1, tmp_path_2, _MAX_PATH);
	UTILITY::get_ancestor_dir(tmp_path_2, 3);
    app_path.Set(tmp_path_2);
	// Xcode set debug options
	if (ac >= 3 && strstr(av[1],"-NSDocument")) {
		optind += 2;
	} else if (ac >= 2 && strstr(av[1],"-psn")) {
		optind += 1;
	}
#endif
	// parse options
	_TCHAR opt = _T('\0');
	bool finished = false;
	for(; optind < ac; optind++) {
		get_option(av[optind], optind, opt);
		finished = set_file_by_option(opt, av[optind]);
		opt = _T('\0');
		if (finished) break;
	}

	for (;optind < ac && av[optind][0] != '\0'; optind++) {
		check_supported_file_by_extension(av[optind]);
	}

	// check ini file
	remake_ini_file_path();

	return 0;
}

bool CParseOptions::get_module_file_name(_TCHAR *path, int size)
{
	return false;
}
