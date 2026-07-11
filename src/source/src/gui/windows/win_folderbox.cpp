/** @file win_folderbox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2015.03.21 -

	@brief [ folder box ]
*/

#include "win_folderbox.h"
#include "../../cchar.h"
#include "../../loadlibrary.h"
#include "../../emu.h"
#include "../../utility.h"

#ifndef PCIDLIST_ABSOLUTE
#define PCIDLIST_ABSOLUTE LPCITEMIDLIST
#endif

extern EMU *emu;

static HRESULT (WINAPI *F_SHCreateItemFromParsingName)(PCWSTR, IBindCtx*, REFIID, void**) = NULL;

namespace GUI_WIN
{

FolderBox::FolderBox(HWND parent_window)
{
	hWnd = parent_window;
	memset(m_selected_dir, 0, sizeof(m_selected_dir));
}

FolderBox::~FolderBox()
{
}

/// Show the folder dialog
///
/// @param[in] title : dialog title
/// @param[in] default_dir : default directory
/// @param[out] selected_dir : selected directory (nullable)
/// @param[in] len : buffer length of selected_dir
bool FolderBox::Show(const _TCHAR *title, const _TCHAR *default_dir, _TCHAR *selected_dir, size_t len)
{
#if 1 // !defined(__MINGW32__)
	HMODULE hShell = NULL;
	DWORD ver = GetVersion();

	if ( LOWORD(ver) >= 6 ) {	// Vista or later
		int n = 0;
		while(F_SHCreateItemFromParsingName == NULL && n == 0) {
			n++;
			LOAD_LIB(hShell, NULL, _T("shell32"), 0);
			GET_ADDR(F_SHCreateItemFromParsingName, HRESULT (WINAPI *)(PCWSTR, IBindCtx*, REFIID, void**), hShell, _T("SHCreateItemFromParsingName"));
		}
	}
	if (F_SHCreateItemFromParsingName) {
		return ShowIFileDialog(title, default_dir, selected_dir, len);
	} else
#endif
	{
		return ShowSHBrowseForFolder(title, default_dir, selected_dir, len);
	}
}

/// callback for SHBrowseForFolder()
int CALLBACK FolderBox::ProcSHBrowseForFolder(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if(uMsg == BFFM_INITIALIZED) {
		SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
	}
	return 0;
}

/// Show the folder selecting dialog (old style)
///
/// @param[in] title : dialog title
/// @param[in] default_dir : default directory
/// @param[out] selected_dir : selected directory (nullable)
/// @param[in] len : buffer length of selected_dir
bool FolderBox::ShowSHBrowseForFolder(const _TCHAR *title, const _TCHAR *default_dir, _TCHAR *selected_dir, size_t len)
{
	BROWSEINFO  binfo;
	PCIDLIST_ABSOLUTE idlist;

	memset(&binfo, 0, sizeof(BROWSEINFO));
	binfo.hwndOwner=hWnd;
	binfo.pidlRoot=NULL;
	binfo.pszDisplayName=(LPSTR)default_dir;
	binfo.lpszTitle=title;
	binfo.ulFlags=BIF_RETURNONLYFSDIRS;
	binfo.lpfn=&ProcSHBrowseForFolder;
	binfo.lParam=(LPARAM)default_dir;
	binfo.iImage=(int)NULL;

	idlist=SHBrowseForFolder(&binfo);
	if (idlist != NULL) {
		SHGetPathFromIDList(idlist, m_selected_dir);
		CoTaskMemFree((LPVOID)idlist);

		UTILITY::add_path_separator(m_selected_dir, _MAX_PATH);

		if (selected_dir && len > 0) {
			UTILITY::tcscpy(selected_dir, len, m_selected_dir);
		}
		return true;
	}
	return false;
}

/// Show the folder selecting dialog
///
/// @param[in] title : dialog title
/// @param[in] default_dir : default directory
/// @param[out] selected_dir : selected directory (nullable)
/// @param[in] len : buffer length of selected_dir
bool FolderBox::ShowIFileDialog(const _TCHAR *title, const _TCHAR *default_dir, _TCHAR *selected_dir, size_t len)
{
    HRESULT hr = S_FALSE;
#if 1 // !defined(__MINGW32__)
	IFileOpenDialog *fileDialog = NULL;
	IShellItem *folder;
	FILEOPENDIALOGOPTIONS options;

	do {
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileDialog));
		if (FAILED(hr)) break;

		hr = fileDialog->GetOptions(&options);
		if (FAILED(hr)) break;
		hr = fileDialog->SetOptions(options | FOS_PICKFOLDERS);
		if (FAILED(hr)) break;

		CTchar ctitle(title);
		hr = fileDialog->SetTitle(ctitle.GetW());

		CTchar cpath(default_dir);
		F_SHCreateItemFromParsingName(cpath.GetW(), NULL, IID_PPV_ARGS(&folder));
		fileDialog->SetFolder(folder);

		hr = fileDialog->Show(hWnd);
		if (FAILED(hr)) break;

		hr = fileDialog->GetResult(&folder);
		if (FAILED(hr)) break;

		LPOLESTR pathOLE = NULL;
		hr = folder->GetDisplayName(SIGDN_FILESYSPATH, &pathOLE);
		if (FAILED(hr)) break;

		CTchar npath(pathOLE);
		UTILITY::tcscpy(m_selected_dir, _MAX_PATH, npath.Get());
		CoTaskMemFree(pathOLE);

		fileDialog->Release();

		UTILITY::add_path_separator(m_selected_dir, _MAX_PATH);

		if (selected_dir && len > 0) {
			UTILITY::tcscpy(selected_dir, len, m_selected_dir);
		}

	} while(0);
#endif
	return (hr == S_OK);
}

/// get the selected directory path
/// @return path
const _TCHAR *FolderBox::GetPathM() const
{
#if defined(USE_UTF8_ON_MBCS)
	static _TCHAR tfile[_MAX_PATH];
	UTILITY::conv_from_native_path(m_selected_dir, tfile, _MAX_PATH);
#else
	const _TCHAR *tfile = m_selected_dir;
#endif
	return tfile;
}

}; /* namespace GUI_WIN */
