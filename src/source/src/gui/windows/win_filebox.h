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

	_TCHAR m_selected_file[_MAX_PATH];

	bool show_main(const _TCHAR *filter, const _TCHAR *title, const _TCHAR *dir, const _TCHAR *ext, bool save, _TCHAR *selected, size_t len);
	void set_file_filter(const CMsg::Id *filter, _TCHAR *fil_str);
	void set_file_filter(const char *filter, bool save, _TCHAR *fil_str, _TCHAR *ext);

public:
	FileBox(HWND parent_window);
	~FileBox();
	bool Show(const CMsg::Id *filter, const _TCHAR *title, const _TCHAR *dir, const _TCHAR *ext, bool save, _TCHAR *selected = NULL, size_t len = 0);
	bool Show(const char *filter, const _TCHAR *title, const _TCHAR *dir, bool save, _TCHAR *selected = NULL, size_t len = 0);

	DWORD GetFlags() { return flags; }
//	void  SetFlags(DWORD value) { flags = value; }
	const _TCHAR *GetPath() const { return m_selected_file; }
	const _TCHAR *GetPathM() const;
};

}; /* namespace GUI_WIN */

#endif /* WIN_FILEBOX_H */
