/** @file win_folderbox.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.03.21 -

	@brief [ folder box ]
*/

#ifndef WIN_FOLDERBOX_H
#define WIN_FOLDERBOX_H

#include <Windows.h>
#include <Shlobj.h>
#include <tchar.h>

namespace GUI_WIN
{

/**
	@brief Folder dialog box
*/
class FolderBox
{
private:
	HWND hWnd;

	_TCHAR m_selected_dir[_MAX_PATH];

	static int CALLBACK ProcSHBrowseForFolder(HWND, UINT, LPARAM, LPARAM);

public:
	FolderBox(HWND parent_window);
	~FolderBox();
	bool Show(const _TCHAR *title, const _TCHAR *default_dir, _TCHAR *selected_dir = NULL, size_t len = 0);
	bool ShowSHBrowseForFolder(const _TCHAR *title, const _TCHAR *default_dir, _TCHAR *selected_dir = NULL, size_t len = 0);
	bool ShowIFileDialog(const _TCHAR *title, const _TCHAR *default_dir, _TCHAR *selected_dir = NULL, size_t len = 0);
	const _TCHAR *GetPath() const { return m_selected_dir; }
	const _TCHAR *GetPathM() const;
};

}; /* namespace GUI_WIN */

#endif /* WIN_FOLDERBOX_H */
