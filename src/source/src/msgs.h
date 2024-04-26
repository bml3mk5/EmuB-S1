/** @file msgs.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.03.01 -

	@brief [ message string ]
*/

#ifndef MSGS_H
#define MSGS_H

#include "common.h"
#include <stdarg.h>
#if defined(USE_QT)
#include <QObject>
#endif

#define CMSG(x) gMessages.Get(CMsg::x)
#define CMSGV(x) gMessages.Get(x)
#define CMSGN(x) gMessages.GetN(CMsg::x)
#define CMSGNV(x) gMessages.GetN(x)

/// @brief id to message string mapping table
class CMsg
#if defined(USE_QT)
	: public QObject
#endif
{
#if defined(USE_QT)
	Q_OBJECT
#endif

public:
	enum Id
	{
		Null = 0,
		Colon_Space,
		None_,
		None_Alt,
		Point,
		Point_Alt,
		Memory_Without_Wait,
		Memory_With_Wait,
		Filter_,
		Filter_Type,
		Direct3D_Filter,
		OpenGL_Filter,
		Direct3D_OFF,
		Direct3D_ON_Sync,
		Direct3D_ON_Async,
		OpenGL,
		OpenGL_OFF,
		OpenGL_ON_Sync,
		OpenGL_ON_Async,
		LB_Need_restart_program_RB,
		LB_Need_PowerOn_RB,
		Need_restart_program,
		Need_restart_program_or_PowerOn,
		CPU_Speed,
		CPU_x0_5,
		CPU_x1,
		CPU_x2,
		CPU_x4,
		CPU_x8,
		CPU_x16,
		CPU_x05_Alt,
		CPU_x1_Alt,
		CPU_x2_Alt,
		CPU_x4_Alt,
		CPU_x8_Alt,
		CPU_x16_Alt,
		CPU_xVDIGIT,
		Synchronize_Device_Speed_With_CPU_Speed,
		Asynchronize_Device_Speed_With_CPU_Speed,
		Checker_Drawing,
		Stripe_Drawing,
		Scanline_Drawing,
		Full_Drawing,
		Afterimage_OFF,
		AfterimageVDIGIT_ON,
		Keepimage_OFF,
		KeepimageVDIGIT_ON,
		Pause,
		LED,
		Show_LED_Inside,
		Show_LED_Outside,
		Show_LED,
		Hide_LED,
		LED_is_disable,
		Inside_LED,
		Outside_LED,
		Hide,
		Show,
		Show_Inside,
		Show_Outside,
		Position,
		LeftTop,
		RightTop,
		LeftBottom,
		RightBottom,
		Capture_Type,
		Snapshot_Path,
		Font_File_ASTERISK,
		Font_Path,
		Folder_,
		Font_,
		Message_Font,
		Info_Font,
		Menu_Font_ASTERISK,
		_Size,
		Load_Wav_File_from_Tape,
		Save_Wav_File_to_Tape,
		Reverse_Wave,
		Half_Wave,
		Correct,
		COS_Wave,
		SIN_Wave,
		Sample_Rate,
		Sample_Bits,
		Show_Message,
		Hide_Message,
		Message_board_is_disable,
		Log,
		Log_,
		Enable_Joypad,
		Enable_Joypad_Key_Assigned,
		Enable_Joypad_PIA_Type,
		Enable_Key_to_Joypad,
		Disable_Joypad,
		Enable_Lightpen,
		Disable_Lightpen,
		Enable_Mouse,
		Disable_Mouse,
		Enable_DirectInput,
		Disable_DirectInput,
		no_label,
		Change_Side_to_A,
		Change_Side_to_B,
		Floppy_Disk_Drive,
		Hard_Disk_Drive,
		When_start_up_mount_disk_at_,
		Ignore_delays_to_find_sector,
		Ignore_delays_to_seek_track,
		Suppress_checking_for_density,
		Suppress_checking_for_media_type,
		Save_a_plain_disk_image_as_it_is,
		Drive,
		Select_Drive,
		FDD_Type,
		FDD_Type_ASTERISK,
		Non_FDD,
		FD3inch_compact_FDD,
		FD3inch_compact_FDD_L3,
		FD5inch_mini_FDD,
		FD5inch_mini_FDD_2D_Type,
		FD5inch_mini_FDD_2HD_Type,
		FD8inch_standard_FDD,
		Unsupported_FDD,
		Control,
		PowerOn_Alt,
		MODE_Switch,
		MODE_Switch_,
		MODE_Switch_Alt,
		Reset_Switch,
		Reset_Switch_Alt,
		System_Mode,
		System_Mode_ASTERISK,
		B_Mode_L3,
		A_Mode_S1,
		A_Mode_S1_Alt,
		B_Mode_L3_Alt,
		NEWON7,
		No_FDD_Alt,
		FD3inch_compact_FDD_Alt,
		FD5inch_mini_FDD_2D_Type_Alt,
		FD5inch_mini_FDD_2HD_Type_Alt,
		FD5inch_mini_FDD_Alt,
		FD8inch_standard_FDD_Alt,
		Pause_Alt,
		Sync_Devices_With_CPU_Speed,
		Sync_Devices_With_CPU_Speed_Alt,
		Auto_Key,
		Open,
		Open_,
		Paste,
		Start,
		Stop,
		Record_Key,
		Play_Alt_E,
		Stop_Playing,
		Record_,
		Stop_Recording,
		Load_State_,
		Load_State_Alt,
		Save_State_,
		Recent_State_Files,
		Exit_,
		Exit_Alt,
		Tape,
		Play_Alt_F7,
		Rec_,
		Eject,
		Rewind,
		Rewind_Alt,
		F_F_,
		F_F_Alt,
		Stop_Alt,
		Real_Mode,
		Recent_Files,
		Insert_Alt,
		New,
		Insert_Blank_2D_,
		Insert_Blank_2HD_,
		Write_Protect,
		Multi_Volume,
		FDDVDIGIT,
		Frame_Rate,
		Auto,
		F60fps,
		F30fps,
		F20fps,
		F15fps,
		F12fps,
		F10fps,
		Record_Screen,
		Rec_60fps,
		Rec_30fps,
		Rec_20fps,
		Rec_15fps,
		Rec_12fps,
		Rec_10fps,
		Capture,
		Window,
		Display,
		Fullscreen,
		Stretch_Screen,
		Stretch_Screen_Alt,
		Cutout_Screen,
		Cutout_Screen_Alt,
		Aspect_Ratio,
		Drawing_Mode,
		Full_Draw,
		Full_Draw_Alt,
		Scanline,
		Scanline_Alt,
		Stripe,
		Stripe_Alt,
		Checker,
		Checker_Alt,
		Afterimage1,
		Afterimage1_Alt,
		Afterimage2,
		Afterimage2_Alt,
		Keepimage1,
		Keepimage2,
		Digital_RGB,
		Analog_RGB,
		Use_Direct3D_Sync,
		Use_Direct3D_Sync_Alt,
		Use_Direct3D_Async,
		Use_Direct3D_Async_Alt,
		Use_OpenGL_Sync,
		Use_OpenGL_Sync_Alt,
		Use_OpenGL_Async,
		Use_OpenGL_Async_Alt,
		Nearest_Neighbour,
		Nearest_Neighbour_Alt,
		Linear,
		Linear_Alt,
		Sound,
		Volume_,
		Volume_Alt,
		Record_Sound,
		Frequency,
		F2000Hz,
		F4000Hz,
		F8000Hz,
		F11025Hz,
		F22050Hz,
		F44100Hz,
		F48000Hz,
		F96000Hz,
		Latency,
		S50msec,
		S75msec,
		S100msec,
		S200msec,
		S300msec,
		S400msec,
		Start_,
		Devices,
		Save,
		Save_,
		Print_to_mpprinter,
		Clear,
		Direct_Send_to_mpprinter,
		Send_to_mpprinter_concurrently,
		Online,
		Enable_Server,
		Connect,
		Ethernet,
		Comm_With_Byte_Data,
		Options_For_Telnet,
		Binary_Mode,
		Send_WILL_ECHO,
		Options,
		Show_LED_Alt,
		Inside_LED_Alt,
		Show_Message_Alt,
		Show_Performance_Meter,
		Use_DirectInput,
		Use_Joypad_Key_Assigned,
		Use_Joypad_Key_Assigned_Alt,
		Use_Joypad_PIA_Type,
		Use_Joypad_PIA_Type_Alt,
		Enable_Lightpen_Alt,
		Use_Mouse_Alt,
		Loosen_Key_Stroke_Game,
		Keybind,
		Keybind_,
		Keybind_Alt,
		Virtual_Keyboard,
		Virtual_Keyboard_,
		Start_Debugger,
		Start_Debugger_Alt,
		Stop_Debugger,
		Configure,
		Configure_,
		Configure_Alt,
		Joypad_Setting,
		Joypad_Setting_,
		S64KB,
		S128KB,
		S256KB,
		S512KB,
		NMI,
		IRQ,
		FIRQ,
		LB_Now_SP,
		LB_Now_RB,
		Help,
		About_,
		Mode,
		Screen,
		Tape_FDD,
		FDD_HDD,
		Network,
		CPU_Memory,
		PowerOff,
		PowerOn,
		Enable_the_state_of_power_off,
		I_O_Port_Address_ASTERISK,
		FDC5__FF00,
		FDC3__FF18,
		FDC3L_FF18,
		PSG6__FF30,
		PSG6L_FF30,
		ExLPT_FF3C,
		ExCOM_FF40,
		PSG9__FF70,
		PSG9L_FF70,
		KANJI_FF75,
		ExPSG_FFE6,
		OS9Ex_FE11,
		Comm__FF77,
		Keybd_FFE0,
		Mouse_FFDC,
		FMSyn_FF1E,
		DisROM_FFCE,
		DisIG_FFE9,
		DisIGL_FFE9,
		RTC___FF38,
		Z80BCD_FF7F,
		MPC68008_FE1A,
		DIP_Switch_ASTERISK,
		Drawing,
		Method,
		Method_ASTERISK,
		Use_ASTERISK,
		CRTC,
		Disptmg_Skew,
		Curdisp_Skew,
		Curdisp_Skew_L3,
		BMP,
		PNG,
		OFF,
		ON,
		OK,
		Cancel,
		Close,
		No,
		Yes,
		Yes_sync,
		Yes_async,
		Update,
		Enable,
		Hostname,
		LPTVDIGIT,
		COMVDIGIT,
		LPTVDIGIT_Hostname,
		COMVDIGIT_Hostname,
		_Port,
		_Print_delay,
		msec,
		S_300baud_F_1200baud,
		S_600baud_F_2400baud,
		S_1200baud_F_4800baud,
		S_2400baud_F_9600baud,
		Connectable_host_to_Debugger,
		Settings_of_serial_ports_on_host,
		Need_re_connect_to_serial_port_when_modified_this,
		Baud_Rate,
		Data_Bit,
		Parity,
		Odd,
		Even,
		Stop_Bit,
		Flow_Control,
		Xon_Xoff,
		Hardware,
		ROM_Path,
		ROM_Path_ASTERISK,
		Use_Extended_Memory_64KB,
		Extended_RAM_ASTERISK,
		No_wait_to_access_the_main_memory,
		Show_message_when_the_CPU_fetches_undefined_opcode,
		Show_message_when_the_address_error_occured_in_MC68008,
		Clear_CPU_registers_at_power_on,
		FM_Synthesis_Card_ASTERISK,
		Extended_PSG_Port_ASTERISK,
		IO_ports_are_FF1E_FF1F_FF16_FF17,
		IO_ports_are_FFE6_FFE7_FFEE_FFEF,
		Clock,
		Connect_interrupt_signal_of_FM_synthesis_to_ASTERISK_ASTERISK,
		Sound_chip,
		This_is_the_common_setting_both_FM_synthesis_card_and_extended_PSG_port,
		Connect_interrupt_signal_of_Z80B_Card_to_ASTERISK,
		MsgBoard_OK,
		MsgBoard_Failed,
		MsgBoard_Use_VSTR_for_VSTR,
		MsgBoard_Couldn_t_load_font_VSTR,
		MsgBoard_Couldn_t_find_fonts_for_VSTR,
		MsgBoard_Couldn_t_load_font_VSTR_for_message,
		MsgBoard_Couldn_t_load_font_VSTR_for_info,
		info,
		message,
		Select_a_folder_to_save_snapshot_images,
		Select_a_font_file_for_showing_messages,
		Select_a_font_folder_for_showing_messages,
		Select_a_folder_containing_the_rom_images,
		Select_a_font,
		File_,
		File_Type_COLON,
		Play_Data_Recorder_Tape,
		Record_Data_Recorder_Tape,
		Play_LB,
		Play_,
		Rec_LB,
		Open_Floppy_Disk_VDIGIT,
		FDD,
		New_Floppy_Disk_VDIGIT,
		Insert_LB,
		Insert_,
		HDD,
		Open_Hard_Disk_VDIGIT,
		New_Hard_Disk_VDIGIT,
		Mount_LB,
		Mount_,
		Unmount,
		Open_HuCARD,
		Open_Cartridge,
		Open_Quick_Disk,
		Open_Media,
		Load_RAM_Pack_Cartridge,
		Load_Memory_Dump,
		Save_RAM_Pack_Cartridge,
		Save_Memory_Dump,
		Open_Text_File,
		Save_Status_Data,
		Load_Status_Data,
		Play_Recorded_Keys,
		Record_Input_Keys,
		Save_Printing_Data,
		Supported_Files,
		All_Files,
		File,
		Supported_Files_ttf_otf,
		Supported_Files_cas_cmt_t88,
		Supported_Files_cas_cmt,
		Supported_Files_wav_cas_tap,
		Supported_Files_wav_cas_mzt_m12,
		Supported_Files_l3_l3b_l3c_wav_t9x,
		Supported_Files_wav_cas,
		L3_File_l3,
		L3B_File_l3b,
		L3C_File_l3c,
		Wave_File_wav,
		T9X_File_t9x,
 		Supported_Files_d88_d77_td0_imd_dsk_fdi_hdm_tfd_xdf_2d_sf7,
		Supported_Files_d88_td0_imd_dsk_fdi_hdm_tfd_xdf_2d_sf7,
		Supported_Files_d88_d77,
		Supported_Files_d88,
		Supported_Files_txt_bas_lpt,
		Supported_Files_l3r,
		Supported_Files_l3k,
		Supported_Files_lpt,
		Supported_Files_rom_bin_hex_gg_col,
		Supported_Files_rom_bin_hex_sms,
		Supported_Files_rom_bin_hex_60,
		Supported_Files_rom_bin_hex_pce,
		Supported_Files_rom_bin_hex,
		Supported_Files_mzt_q20_qdf,
		Supported_Files_bin,
		Supported_Files_ram_bin_hex,
		All_Files_,
		Bracket_Pause,
		num_INS_DEL,
		KATA_HIRA,
		HENKAN,
		MUHENKAN,
		Allow_RIGHT,
		Allow_LEFT,
		Allow_UP,
		Allow_DOWN,
		SPACE,
		num_Comma,
		num_Multiply,
		num_Plus,
		num_Question,
		num_Minus,
		num_Point,
		num_Devide,
		num_Enter,
		num_Equal,
		num_0,
		num_VCHAR,
		num_VDIGIT,
		PF_VDIGIT,
		space,
		up,
		up_right,
		right,
		down_right,
		down,
		down_left,
		left,
		up_left,
		YA_up,
		YA_up_right,
		YA_right,
		YA_down_right,
		YA_down,
		YA_down_left,
		YA_left,
		YA_up_left,
		Y_up,
		YX_up_right,
		X_right,
		YX_down_right,
		Y_down,
		YX_down_left,
		X_left,
		YX_up_left,
		R_up,
		RZ_up_right,
		Z_right,
		RZ_down_right,
		R_down,
		RZ_down_left,
		Z_left,
		RZ_up_left,
		V_up,
		VU_up_right,
		U_right,
		VU_down_right,
		V_down,
		VU_down_left,
		U_left,
		VU_up_left,
		POV_up,
		POV_up_right,
		POV_right,
		POV_down_right,
		POV_down,
		POV_down_left,
		POV_left,
		POV_up_left,
		button,
		button_A,
		button_B,
		button_VCHAR,
		trigger_A,
		trigger_B,
		trigger_VCHAR,
		bit_4,
		bit_5,
		bit_6,
		bit_7,
		bit_VDIGIT,
		Left_Analog_X,
		Left_Analog_Y,
		Right_Analog_X,
		Right_Analog_Y,
		Analog_X,
		Analog_Y,
		Analog_Z,
		Analog_R,
		Analog_U,
		Analog_V,
		Analog_X_Rev,
		Analog_Y_Rev,
		Analog_Z_Rev,
		Analog_R_Rev,
		Analog_U_Rev,
		Analog_V_Rev,
		Enable_X_axis,
		Enable_Y_axis,
		Enable_Z_axis,
		Enable_R_axis,
		Enable_U_axis,
		Enable_V_axis,
		X_axis,
		Y_axis,
		Z_axis,
		R_axis,
		U_axis,
		V_axis,
		Kanji,
		henkan,
		muhenkan,
		katakana,
		eisu,
		kana,
		right_shift,
		left_shift,
		right_ctrl,
		left_ctrl,
		right_option,
		right_alt,
		left_option,
		left_alt,
		right_command,
		right_meta,
		left_command,
		left_meta,
		right_win,
		left_win,
		Next,
		Prev,
		Bind,
		BindVDIGIT,
		S1_Key,
		PIA_on_S1,
		Level3_Key,
		PIA_on_L3,
		Keyboard,
		Joypad,
		Joypad_Key_Assigned,
		Joypad_PIA_Type,
		Key_to_Joypad,
		JoypadVDIGIT,
		Signals_are_negative_logic,
		Recognize_as_another_key_when_pressed_two_buttons,
		Load_Default,
		Load_Preset_1,
		Load_Preset_2,
		Load_Preset_3,
		Load_Preset_4,
		Load_Preset_VDIGIT,
		Save_Preset_1,
		Save_Preset_2,
		Save_Preset_3,
		Save_Preset_4,
		Save_Preset_VDIGIT,
		Button_Mashing_Speed,
		Analog_to_Digital_Threshold,
		Analog_to_Digital_Sensitivity,
		Volume,
		Master,
		Beep,
		PSG,
		PSG6_CR,
		PSG9_CR,
		Relay,
		CMT,
		ExPSG_CR_FM,
		ExPSG_CR_SSG,
		ExPSG_CR_ADPCM,
		ExPSG_CR_Rhythm,
		OPN_CR_FM,
		OPN_CR_SSG,
		OPNA_CR_ADPCM,
		OPNA_CR_Rhythm,
		Mute,
		PSG_like_AY_3_8910,
		OPN_like_YM2203,
		OPNA_like_YM2608,
		PSG_like_AY_3_8910_1MHz,
		PSG_like_AY_3_8910_2MHz,
		OPN_like_YM2203_2MHz,
		OPN_like_YM2203_3MHz,
		OPN_like_YM2203_4MHz,
		OPNA_like_YM2608_4MHz,
		OPNA_like_YM2608_8MHz,
		VSTR_was_loaded,
		VSTR_couldn_t_be_loaded,
		VSTR_is_invalid_file,
		VSTR_is_invalid_version,
		VSTR_is_old_version,
		VSTR_was_saved,
		VSTR_couldn_t_be_saved,
		VSTR_is_not_compatible_use_default_setting,
		Floppy_image_couldn_t_be_opened,
		Floppy_image_on_drive_VDIGIT_couldn_t_be_opened,
		Floppy_image_on_drive_VDIGIT_couldn_t_be_saved,
		Floppy_image_on_drive_VDIGIT_is_saved_as_the_new_file_VSTR,
		There_is_the_same_floppy_disk_in_drive_VDIGIT_and_VDIGIT,
		The_density_in_track_VDIGIT_side_VDIGIT_is_different_from_specified_one,
		The_media_type_in_drive_VDIGIT_is_different_from_specified_one,
		Tape_image_couldn_t_be_opened,
		Tape_image_couldn_t_be_saved,
		Print_image_couldn_t_be_saved,
		Status_image_couldn_t_be_saved,
		Status_image_couldn_t_be_loaded,
		Load_State_Cannot_open,
		Load_State_Unsupported_file,
		Load_State_Invalid_version,
		Load_State_No_longer_support_a_status_file_for_VSTR,
		Auto_key_file_couldn_t_be_opened,
		Record_key_file_couldn_t_be_saved,
		VSTR_is_VDIGIT_bytes_smaller_than_assumed_one,
		VSTR_is_different_image_from_assumed_one,
		This_is_not_record_key_file,
		Record_key_file_is_invalid_version,
		Record_key_file_has_invalid_parameter,
		This_record_key_file_is_not_supported,
		The_record_key_file_for_VSTR_is_no_longer_supported,
		The_version_of_the_emulator_used_for_recording_is_VDIGIT_VDIGIT_VDIGIT,
		Couldn_t_start_recording_audio,
		Couldn_t_start_recording_video,
		Now_saving_video_file_,
		Video_file_was_saved,
		Screen_was_saved_successfully,
		Select_a_sample_rate_on_sound_menu_in_advance,
		You_can_set_properties_after_pressing_start_button,
		Need_install_library,
		Codec_Type,
		Quality,
		Max_368Kbps,
		High_256Kbps,
		Normal_128Kbps,
		Low_96Kbps,
		Min_64Kbps,
		Max_50Mbps,
		High_10Mbps,
		Normal_1Mbps,
		Low_500Kbps,
		Min_100Kbps,
		Max,
		High,
		Normal,
		Middle,
		Low,
		Min,
		Debugger_was_started,
		Cannot_start_debugger,
		Debugger_was_stopped,
		Save_to_VSTR,
		About_bml3mk5,
		About_mbs1,
		Hide_bml3mk5,
		Hide_mbs1,
		Quit_bml3mk5,
		Quit_mbs1,
		Hide_Others,
		Show_All,
		Services,
		Preferences_,
		Language_ASTERISK,
		Default,
		End
	};
private:
	const _TCHAR *msgs[CMsg::End];
#if defined(USE_QT)
	_TCHAR tmp_msg[32][256];
	int tmp_idx = 0;
#endif

public:
	explicit CMsg();
	~CMsg();

	const _TCHAR *Get(Id id);
	const _TCHAR *Get(Id id, bool translate);
	const _TCHAR *GetN(Id id) const;
	int Sprintf(_TCHAR *str, size_t size, Id id, ...);
	int Vsprintf(_TCHAR *str, size_t size, Id id, va_list ap);
};

extern CMsg gMessages;

#endif /* MSGS_H */
