/** @file win_filebox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.03.21 -

	@brief [ file box ]
*/

#ifndef WIN_FILEBOX_H
#define WIN_FILEBOX_H

#include <Windows.h>
#include <tchar.h>
#include "../../msgs.h"

namespace GUI_WIN
{

/**
	@brief File dialog box
*/
class FileBox
{
private:
	HWND hWnd;
	DWORD flags;

	_TCHAR selected_file[_MAX_PATH];

public:
	FileBox(HWND parent_window);
	~FileBox();
	bool Show(const CMsg::Id *filter, const _TCHAR *title, const _TCHAR *dir, const _TCHAR *ext, bool save, _TCHAR *path = NULL);

	DWORD GetFlags() { return flags; }
//	void  SetFlags(DWORD value) { flags = value; }
	const _TCHAR *GetPath() const { return selected_file; }
	const _TCHAR *GetPathM() const;
};

}; /* namespace GUI_WIN */

#endif /* WIN_FILEBOX_H */
