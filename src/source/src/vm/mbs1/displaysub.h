/** @file display.cpp

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2011.06.08 -

	@brief [ display sub ]
*/

#ifdef DISPLAY_H

{
	// videomode negative
	dws_videomode_n = 1 - (*videomode);
	// interace or videomode
	dws_int_vid = (REG_INTERLACE_SEL) + dws_videomode_n;

	// odd line ?
	dws_yd = (scanline >= 2 ? sw & 1 : 0);

	// set drawing area
	dws_y0=screen_top + dws_yd;
	dws_y1=screen_bottom;
	INIT_SCRN_DOT(dws_scrn, dws_y0);
	INIT_SCRN_FILTER(dws_y0);
	INIT_SCRNLINE1(dws_y0);
	SET_SCRNLINE2M(dws_b_p.i & BRG_COLOR_IMASK);

	for(dws_y = dws_y0; dws_y < dws_y1; ) {
		dws_p_vinfo = &crt_mon_vinfo[dws_y];
		dws_buf_y_h = dws_p_vinfo->row;
		dws_p_crt_mon = crt_mon[dws_buf_y_h];
		dws_p_vline = &crt_mon_vline[dws_buf_y_h];
		dws_raster = dws_p_vinfo->raster;

		if (scanline == 1) {
			if ((dws_p_vline->flags ^ dws_y) & 1) {
				// clear undrawing line
				for(dws_x = screen_left; dws_x < screen_right; dws_x++) {
					SET_SCRN_DOT(dws_x, color64[0][0]);
				}
				dws_y+=dws_ys;
				ADD_SCRN_DOT(dws_scrn);
				ADD_SCRN_FILTER;
				ADD_SCRNLINE1;
				continue;
			}
		}

		dws_buf_x = screen_left / crtc_ppch;
		dws_x0 = screen_left;
		dws_x1 = screen_right;
		dws_xodd = 0;
		if (buf_stepcols > 1) {
			dws_xodd = ((dws_buf_y_h + (sw >> 1)) & 1);
			dws_buf_x += dws_xodd;
			dws_x0 += (dws_xodd * crtc_ppch);
			dws_xodd = ((dws_buf_x + dws_p_vline->left_col) & 1);
		}

#ifdef DEBUG_CRTMON
		logging->out_debugf(_T("DISPLAY:draw_sub: sw:%d yh:%3d ras:%2d flg:%c l:%2d yd:%2d")
			, sw, dws_buf_y_h, dws_p_vline->raster, dws_l_flg ? _T('t') : _T('f'), dws_l, dws_yd
		);
#endif
		if (NOW_S1_MODE) {
			//
			//
			// S1
			//
			//
			dws_gline = (dws_p_vline->flags & 0x80) == 0 || (dws_y & 1) == 0 ? 0 : 1;

			for(dws_x = dws_x0; dws_x < dws_x1; ) {
				dws_p_crt_mon_x = &dws_p_crt_mon[dws_buf_x];
				if (dws_p_crt_mon_x->disptmg & 1) {
					dws_disp_cursor_char = (dws_p_vinfo->cursor_st_col <= dws_buf_x && dws_buf_x <= dws_p_vinfo->cursor_ed_col);

					dws_text_cram = dws_p_crt_mon_x->color;

					// text character
					if ((dws_text_cram & 0x50) != 0x10) {
						dws_text_code = dws_p_crt_mon_x->text;

						if (dws_text_cram & 0x20) {
							// ig pattern
							dws_ct[0].i = dws_p_crt_mon_x->c.i;
							// reverse dot
							if (dws_text_cram & 0x08) {
								dws_ct[0].i = ~dws_ct[0].i;
							}
							// cursor line ?
							if (dws_disp_cursor_char) {
								dws_ct[0].b.b = ((dws_text_cram & 1) ? 0xff : 0);
								dws_ct[0].b.r = ((dws_text_cram & 2) ? 0xff : 0);
								dws_ct[0].b.g = ((dws_text_cram & 4) ? 0xff : 0);
								dws_ct[0].b.i = ((dws_text_cram & 7) ? 0 : 0xff);
							} else {
								// convert
								if ((dws_text_cram & 0x80) == 0 && dws_ct[0].i) {
								// when width 40
									if (dws_xodd) {
										EXPAND_UINT32_LEFT(dws_ct[0].i, dws_ct[0].i);
									} else {
										EXPAND_UINT32_RIGHT(dws_ct[0].i, dws_ct[0].i);
									}
								}
							}
							CREATE_MASK_1(dws_ct[0],dws_ct_m);
						} else {
							// character
							dws_pattern = chrfont[REG_INTERLACE_SEL][dws_text_code][(dws_raster >> dws_videomode_n) & 0xf];
							// cursor line ?
							if (dws_disp_cursor_char) {
								dws_pattern = ~dws_pattern;
							}
							// reverse dot
							if (dws_text_cram & 0x08) {
								dws_pattern = ~dws_pattern;
							}
							if ((dws_text_cram & 0x80) == 0 && dws_pattern) {
								// when width 40
								if (dws_xodd) {
									EXPAND_UINT8_LEFT(dws_pattern,dws_pattern);
								} else {
									EXPAND_UINT8_RIGHT(dws_pattern,dws_pattern);
								}
							}
#if 1
							dws_ct[0].i = (dws_p_crt_mon_x->c.i & (dws_pattern | ((uint32_t)dws_pattern << 8) | ((uint32_t)dws_pattern << 16) | ((uint32_t)dws_pattern << 24)));
#else
							dws_ct[0].b.b = (dws_p_crt_mon_x->c.b.b & dws_pattern);
							dws_ct[0].b.r = (dws_p_crt_mon_x->c.b.r & dws_pattern);
							dws_ct[0].b.g = (dws_p_crt_mon_x->c.b.g & dws_pattern);
							dws_ct[0].b.i = (dws_p_crt_mon_x->c.b.i & dws_pattern);
#endif
							CREATE_MASK_0(~dws_pattern,dws_ct_m);
						}
					}
					// graphic
					dws_attr = dws_p_crt_mon_x->attr;
					if ((dws_text_cram & 0x50) && (dws_attr & DISP_PAGE_BRG_MASK)) {
						dws_cg[0].i = (dws_p_crt_mon_x->g[dws_gline].i & dws_p_crt_mon_x->brg_mask.i);

						if ((dws_attr & SCRN_MODE_MASK) == SCRN_MODE_64COLOR_MASK) {
							// 64color or supergraphic
							dws_cg[1].i = (dws_p_crt_mon_x->g[1].i & dws_p_crt_mon_x->brg_mask.i);
							if ((dws_attr & 0x02) == 0 && (dws_cg[0].i || dws_cg[1].i)) {
								// when width 320
								if (dws_xodd) {
									EXPAND_UINT32_LEFT(dws_cg[0].i, dws_cg[0].i);
									EXPAND_UINT32_LEFT(dws_cg[1].i, dws_cg[1].i);
								} else {
									EXPAND_UINT32_RIGHT(dws_cg[0].i, dws_cg[0].i);
									EXPAND_UINT32_RIGHT(dws_cg[1].i, dws_cg[1].i);
								}
							}
							if (TVSUPER_ANALOG) {
								// analog RGB, 64 color
								CREATE_MASK_2(dws_cg,dws_cg_m);

								if ((dws_text_cram & 0x50) == 0x10) {
									// display graphic only
									COPY_BRG_2(dws_cg,dws_cd);
								} else if ((dws_text_cram & 0x50) == 0x40) {
									// display text and graphic (text is a top)
									COPY_BRG_2_WITH_MASK(dws_cg,dws_cd,dws_ct_m);
									ADD_BRG_1(dws_ct[0],dws_cd[0]);
									ADD_BRG_1(dws_ct[0],dws_cd[1]);
								} else {
									// display text and graphic (graphic is a top)
									COPY_BRG_1_WITH_MASK(dws_ct[0],dws_cd[0],dws_cg_m);
									COPY_BRG_1(dws_cd[0],dws_cd[1]);
									ADD_BRG_2(dws_cg,dws_cd);
								}
								CREATE_MASK_2(dws_cd,dws_cd_m);

								if (FLG_ORIG_NOPAL64) {
									// non convert
									for(dws_bt = 0; dws_bt < 8; dws_bt++) {
										ENCODE_COLOR_ONE_DOT_2(dws_bt, dws_cd, dws_cdt2);
										dws_cdt2[0][dws_bt] |= ((dws_cdt2[1][dws_bt] & 2) << 2);
									}
								} else {
									// convert palette color
									for(dws_bt = 0; dws_bt < 8; dws_bt++) {
										ENCODE_PALETTE_ONE_DOT_2(dws_p_vline, dws_bt, dws_cd, dws_cdt2);
									}
								}

							} else {
								// In digital RGB, supergraphic (8 color) mode
								CREATE_MASK_1(dws_cg[0],iwk1);
								COPY_BRG_1_WITH_MASK(dws_cg[1],dws_cg[1],iwk1);
								dws_cg[0].i |= dws_cg[1].i;
								CREATE_MASK_1(dws_cg[0],dws_cg_m);

								if ((dws_text_cram & 0x50) == 0x10) {
									// display graphic only
									COPY_BRG_1(dws_cg[0],dws_cd[0]);
								} else if ((dws_text_cram & 0x50) == 0x40) {
									// display text and graphic (text is a top)
									COPY_BRG_1_WITH_MASK(dws_cg[0],dws_cd[0],dws_ct_m);
									ADD_BRG_1(dws_ct[0],dws_cd[0]);
								} else {
									// display text and graphic (graphic is a top)
									COPY_BRG_1_WITH_MASK(dws_ct[0],dws_cd[0],dws_cg_m);
									ADD_BRG_1(dws_cg[0],dws_cd[0]);
								}
								CREATE_MASK_1(dws_cd[0],dws_cd_m);

								// convert palette color
								for(dws_bt = 0; dws_bt < 8; dws_bt++) {
									ENCODE_PALETTE_ONE_DOT_1(dws_p_vline, dws_bt, dws_cd[0], dws_cdt2[0]);
								}

							}
						} else {
							// palette 8 color
							if ((dws_attr & 0x02) == 0 && dws_cg[0].i) {
								// when width 320
								if (dws_xodd) {
									EXPAND_UINT32_LEFT(dws_cg[0].i, dws_cg[0].i);
								} else {
									EXPAND_UINT32_RIGHT(dws_cg[0].i, dws_cg[0].i);
								}
							}
							CREATE_MASK_1(dws_cg[0],dws_cg_m);
							dws_cg[1] = dws_cg[0];

							if ((dws_text_cram & 0x50) == 0x10) {
								// display graphic only
								COPY_BRG_1(dws_cg[0],dws_cd[0]);
							} else if ((dws_text_cram & 0x50) == 0x40) {
								// display text and graphic (text is a top)
								COPY_BRG_1_WITH_MASK(dws_cg[0],dws_cd[0],dws_ct_m);
								ADD_BRG_1(dws_ct[0],dws_cd[0]);
							} else {
								// display text and graphic (graphic is a top)
								COPY_BRG_1_WITH_MASK(dws_ct[0],dws_cd[0],dws_cg_m);
								ADD_BRG_1(dws_cg[0],dws_cd[0]);
							}
							CREATE_MASK_1(dws_cd[0],dws_cd_m);

							if (TVSUPER_ANALOG && FLG_ORIG_NOPAL) {
								// non convert
								for(dws_bt = 0; dws_bt < 8; dws_bt++) {
									ENCODE_COLOR_ONE_DOT_1(dws_bt, dws_cd[0], dws_cdt2[0]);
								}
							} else {
								// convert palette color
								for(dws_bt = 0; dws_bt < 8; dws_bt++) {
									ENCODE_PALETTE_ONE_DOT_1(dws_p_vline, dws_bt, dws_cd[0], dws_cdt2[0]);
								}
							}

						}
					} else {
						// display text only
						COPY_BRG_1(dws_ct[0],dws_cd[0]);

						CREATE_MASK_1(dws_cd[0],dws_cd_m);

						if (TVSUPER_ANALOG && FLG_ORIG_NOPAL) {
							// non convert
							for(dws_bt = 0; dws_bt < 8; dws_bt++) {
								ENCODE_COLOR_ONE_DOT_1(dws_bt, dws_cd[0], dws_cdt2[0]);
							}
						} else {
							// convert palette color
							for(dws_bt = 0; dws_bt < 8; dws_bt++) {
								ENCODE_PALETTE_ONE_DOT_1(dws_p_vline, dws_bt, dws_cd[0], dws_cdt2[0]);
							}
						}

					}

#ifdef _DEBUG_CTRAM
	if (dws_buf_x >= 30 && dws_buf_y_h >= 44 && dws_buf_y_h < 46) {
		logging->out_debugf(_T("usc txt: y:%d t:%02x c:%02x ct:%08x ctm:%08x cd:%08x"),dws_buf_y_h,dws_text_code,dws_text_cram,dws_ct[0].i,dws_ct_m.i,dws_cd[0].i);
	}
#endif

					// render
					if (TVSUPER_ANALOG && ((dws_attr & SCRN_MODE_MASK) == SCRN_MODE_64COLOR_MASK)) {
						// 64color
						dws_dt = dws_x;
						for (dws_bt = 7; dws_bt >= 0; dws_bt--) {
							wk1u8 = dws_cdt2[0][dws_bt];
							wk2u8 = dws_cdt2[1][dws_bt];
							dws_dot = color64[wk1u8][wk2u8];
							SET_SCRNLINE2M(dws_dot);
							if (dws_dt > SCREEN_WIDTH) break;
							SET_SCRN_DOT(dws_dt, dws_dot);
							SET_SCRN_FILTER(dws_dt);
							dws_dt++;
						}
					} else {
						// 16color
						dws_dt = dws_x;
						for (dws_bt = 7; dws_bt >= 0; dws_bt--) {
							wk1u8 = dws_cdt2[0][dws_bt];
							dws_dot = color16[wk1u8];
							SET_SCRNLINE2M(dws_dot);
							if (dws_dt > SCREEN_WIDTH) break;
							SET_SCRN_DOT(dws_dt, dws_dot);
							SET_SCRN_FILTER(dws_dt);
							dws_dt++;
						}
					}
					dws_xodd = ((buf_stepcols - dws_xodd) & 1);
				} else {
					if (dws_p_crt_mon_x->disptmg & 2) {
						dws_raster += 2;
					}
					DISP_OUT_OF_DISPTMG_S(dws_buf_x, dws_x, dws_p_crt_mon_x->color);
				}
				dws_x += (buf_stepcols * crtc_ppch);
				dws_buf_x += buf_stepcols;
			}
		} else {
			//
			//
			// L3
			//
			//
			for(dws_x = dws_x0; dws_x < dws_x1; ) {
				dws_p_crt_mon_x = &dws_p_crt_mon[dws_buf_x];
				dws_b_p = dws_p_crt_mon_x->c;
				if (dws_p_crt_mon_x->disptmg & 1) {
					dws_disp_cursor_char = (dws_p_vinfo->cursor_st_col == dws_buf_x);

#ifdef DEBUG_CRTMON2
					if (dws_disp_cursor_char) {
						logging->out_debugf(_T("  cursor: dws_x:%d dws_l:%d cur_left:%d cur_right:%d dws_y:%d")
							, dws_x, dws_l, cur_left[dws_l], cur_right[dws_l], dws_y
							);
					}
#endif

					dws_text_code = dws_p_crt_mon_x->text;
					dws_text_cram = dws_p_crt_mon_x->color;
					dws_c_f = (dws_text_cram & 0x07) | 0x08;
					DECODE_TO_PALETTE16_NUM(dws_c_f, dws_t_p);
					DECODE_TO_PALETTE16_NUM(dws_p_crt_mon_x->attr, dws_b_p);
//					dws_c_b = bg_color;

					switch(dws_text_cram & 0x30) {
					case 0x30:
					case 0x20:
						// ig
						// disp ig pattern
						DISP_L3_IG_PATTERN(dws_text_code);
						break;
					case 0x10:
						// graphic
						if (dws_text_cram & 8) {
							dws_text_code = ~dws_text_code;
						}
						if (dws_disp_cursor_char) {
							dws_text_code = ~dws_text_code;
						}
						if (dws_text_cram & 0x40) {
							// hireso graphic
							DISP_L3_GRAPHIC_HIRESO(dws_text_code);
						} else {
							// normal graphic
							DISP_L3_GRAPHIC_NORMAL(dws_text_code);
						}
						break;
					default:
						// character
						DISP_L3_TEXT_CHAR(dws_text_code);
						break;
					}
				} else {
					if (dws_p_crt_mon_x->disptmg & 2) {
						dws_raster += 2;
					}
					DISP_OUT_OF_DISPTMG_S(dws_buf_x, dws_x, dws_p_crt_mon_x->attr);
				}
				dws_x += (crtc_ppch * buf_stepcols);
				dws_buf_x += buf_stepcols;
			}
		}

		dws_y+=dws_ys;
		ADD_SCRN_DOT(dws_scrn);
		ADD_SCRN_FILTER;
		ADD_SCRNLINE1;
	}

}

#endif /* DISPLAY_H */
