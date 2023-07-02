﻿/** @file win_vkeyboardbase.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2016.12.21 -

	@brief [ virtual keyboard ]
*/

#include <string.h>
//#include <malloc.h>
#include "win_vkeyboardbase.h"
#include "../../emu.h"
#include "../../fifo.h"
//#include "../../logging.h"

//#if defined(_BML3MK5)
//#include "../../gui/vkeyboard_bml3mk5.h"
//#elif defined(_MBS1)
//#include "../../gui/vkeyboard_mbs1.h"
//#endif

extern EMU *emu;

namespace Vkbd {

static const int csBitmapIds[BITMAPIDS_END] = {
	IDB_VKEYBOARD,
	IDB_LED_PARTS,
	IDB_VKEY_MODE,
	IDB_VKEY_BREAK,
	IDB_VKEY_POWER,
#if defined(_MBS1)
	IDB_VKEY_RESET
#endif
};

OSDBase::OSDBase() : Base()
{
	hBrush = NULL;
}

OSDBase::~OSDBase()
{
}

void OSDBase::load_bitmap()
{
	// set bitmaps
//	if (!hVkbd) return;

//	HDC hdc = ::GetDC(hVkbd);
	for(int i=0; i<BITMAPIDS_END; i++) {
		pBitmaps[i] = new CBitmap(csBitmapIds[i], 1);
	}
//	::ReleaseDC(hVkbd, hdc);

	// brush
#ifdef _BML3MK5
	hBrush = ::CreateSolidBrush(RGB(0x80,0x80,0x80));
#else
	hBrush = ::CreateSolidBrush(RGB(0x30,0x30,0x30));
#endif
}

bool OSDBase::create_surface()
{
	if (pSurface) return true;
//	if (!hVkbd) return false;

	CBitmap *bmp = pBitmaps[BITMAPIDS_BASE];
//	HDC hdc = ::GetDC(hVkbd);
	pSurface = new CSurface(bmp->Width(), bmp->Height());
	::BitBlt(pSurface->GetDC(), 0, 0, pSurface->Width(), pSurface->Height(), bmp->GetDC(), 0, 0, SRCCOPY);
//	::ReleaseDC(hVkbd, hdc);
	return true;
}

void OSDBase::unload_bitmap()
{
	Base::unload_bitmap();

	if (hBrush) {
		::DeleteObject(hBrush);
		hBrush = NULL;
	}
}

/// @param[in] info : information of mouse
/// @param[in] onoff : pressed a key?
void OSDBase::need_update_window_base(PressedInfo_t *info, bool onoff)
{
	const Bitmap_t *bp = NULL;

	Rect_t dstrect = info->re;
	int parts_num = info->parts_num;

	if (onoff) {
		// key pressed / led on
		if (parts_num >= 0) {
			// another parts
			bp = &cBmpParts[parts_num];
			blit_surface(pBitmaps[bp->idx], bp->x, bp->y, &dstrect);
		} else {
			// set base parts shifted down 3px
			fill_rect(&dstrect);
			dstrect.top += 3;
			blit_surface(pBitmaps[BITMAPIDS_BASE], info->re.left, info->re.top,	&dstrect);
		}
	} else {
		// key released / led off
		blit_surface(pBitmaps[BITMAPIDS_BASE], dstrect.left, dstrect.top, &dstrect);
	}
}

void OSDBase::fill_rect(Rect_t *re)
{
	if (!pSurface) return;

	RECT wre = { re->left, re->top, re->right, re->bottom };
	::FillRect(pSurface->GetDC(), &wre, hBrush);
}

bool OSDBase::blit_surface(CBitmap *pSrcBmp, short src_x, short src_y, Rect_t *dst_re)
{
	if (!pSurface || !pSrcBmp) return false;

	::BitBlt(pSurface->GetDC(), dst_re->left, dst_re->top, dst_re->right - dst_re->left, dst_re->bottom - dst_re->top,
		pSrcBmp->GetDC(), src_x, src_y, SRCCOPY);

	return true;
}

} /* namespace Vkbd */
