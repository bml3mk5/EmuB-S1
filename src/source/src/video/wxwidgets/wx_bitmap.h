/** @file wx_bitmap.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2017.02.26 -

	@brief [ cbitmap ]
*/

#ifndef _WX_CBITMAP_H_
#define _WX_CBITMAP_H_

#include "../../cbitmap.h"

/**
	@brief manage bitmap
*/
class CBitmap : public CBitmapBase
{
protected:

public:
	CBitmap();
	CBitmap(const _TCHAR *file_name, SDL_PixelFormat *format);
	CBitmap(CBitmap &src, int x, int y, int w, int h);
	~CBitmap();

	bool Load(const _TCHAR *file_name, SDL_PixelFormat *format);
};

#endif
