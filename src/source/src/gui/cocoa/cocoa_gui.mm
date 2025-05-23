/** @file cocoa_gui.mm

	HITACHI BASIC MASTER LEVEL3 Mark5 / MB-S1 Emulator 'EmuB-6892/EmuB-S1'
	Skelton for retropc emulator
	SDL edition + Cocoa GUI

	@author Sasaji
	@date   2015.04.24 -

	@brief [ gui for macosx ]
*/

#import <Cocoa/Cocoa.h>
#import "cocoa_gui.h"
#import "../../emu.h"
#import "../gui.h"
#import "cocoa_volumepanel.h"
#import "cocoa_keybindpanel.h"
#import "cocoa_configpanel.h"
#import "cocoa_recvidpanel.h"
#import "cocoa_recaudpanel.h"
#import "cocoa_seldrvpanel.h"
#import "cocoa_joysetpanel.h"
#import "cocoa_savedatarec.h"
#import "cocoa_ledbox.h"
#ifdef USE_VKEYBOARD
#import "cocoa_vkeyboard.h"
#endif
#import "../../config.h"
#import "../../clocale.h"
#import "../../version.h"
#import "../../main.h"
#import "../../msgs.h"
#import "../../depend.h"
#import "../../labels.h"
#import "../../utility.h"
#import <SDL_syswm.h>

extern EMU *emu;

static NSWindow *get_main_window()
{
	NSWindow *window = nil;
#ifndef USE_SDL2
	NSArray *windows = [NSApp windows];
	for(int i=0; i<[windows count]; i++) {
		NSWindow *nw = [windows objectAtIndex:i];
		NSRange rg = [[nw className] rangeOfString:@"SDL"];
		if (rg.length > 0) {
			window = nw;
			break;
		}
	}
#else
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(((EMU_OSD *)emu)->get_window(), &info);
	window = info.info.cocoa.window;
#endif
	return window;
}

//
@implementation NSApplication (SDLApplication)
- (void)orderFrontStandardAboutPanel:(id)sender
{
	GUI *gui = emu->get_gui();
	char name[128],ver[256],libver[256];
	UTILITY::sprintf(name, sizeof(name), "%s", APP_NAME);
	// version
	UTILITY::sprintf(ver, sizeof(ver), "Version: %s \"%s\""
			,APP_VERSION,PLATFORM);
#ifdef _DEBUG
	strcat(ver, " (DEBUG Version)");
#endif
	// edition
	emu->get_edition_string(libver, sizeof(libver));
	strcat(libver, "\n using ");
	// library
	size_t vlen = strlen(libver);
	gui->GetLibVersionString(&libver[vlen], (int)(sizeof(libver) - vlen), ", ");

	NSAttributedString *cre = [[NSAttributedString alloc] initWithString:
							   [NSString stringWithUTF8String:libver]];

	NSDictionary *dic = [NSDictionary dictionaryWithObjectsAndKeys:
		[NSString stringWithUTF8String:name],@"ApplicationName"
		,[NSString stringWithUTF8String:ver],@"ApplicationVersion"
		,@"",@"Version"
		,cre,@"Credits"
		,nil];

	[self orderFrontStandardAboutPanelWithOptions:dic];
}
@end

#ifdef GUI_TYPE_COCOA

@implementation CocoaMenuItem
@synthesize drv;
@synthesize num;

- (id)initWithTitle:(NSString *)new_title action:(SEL)new_action drv:(int)new_drv num:(int)new_num keyEquivalent:(NSString *)new_accl
{
	self.drv = new_drv;
	self.num = new_num;
	[super initWithTitle:new_title action:new_action keyEquivalent:new_accl];
	return self;
}
- (id)initWithTitleById:(CMsg::Id)new_titleid action:(SEL)new_action drv:(int)new_drv num:(int)new_num keyEquivalent:(NSString *)new_accl
{
	NSString *new_title = [[NSString alloc] initWithUTF8String:gMessages.Get(new_titleid)];
	[self initWithTitle:new_title action:new_action keyEquivalent:new_accl];
	return self;
}
- (void)setTitleById:(CMsg::Id)new_titleid
{
	NSString *new_title = [[NSString alloc] initWithUTF8String:gMessages.Get(new_titleid)];
	[self setTitle:new_title];
}

@end


@implementation CocoaMenu
+ (CocoaMenu *)create_menu:(const char *)new_title
{
	CocoaMenu *menu = [[CocoaMenu alloc] initWithTitle:[NSString stringWithUTF8String:new_title]];
	return menu;
}
+ (CocoaMenu *)create_menu_by_id:(CMsg::Id)new_titleid
{
	return [CocoaMenu create_menu:gMessages.Get(new_titleid)];
}
+ (CocoaMenu *)create_menu:(const char *)new_title :(id)new_delegate
{
	CocoaMenu *menu = [[CocoaMenu alloc] initWithTitle:[NSString stringWithUTF8String:new_title]];
	[menu setDelegate:new_delegate];
	return menu;
}
+ (CocoaMenu *)create_menu_by_id:(CMsg::Id)new_titleid :(id)new_delegate
{
	return [CocoaMenu create_menu:gMessages.Get(new_titleid):new_delegate];
}
- (CocoaMenuItem *)add_menu_item:(const char *)new_title :(NSObject *)new_target :(SEL)new_action :(int)new_drv :(int)new_num :(unichar)new_accl
{
	CocoaMenuItem *menuItem;
	NSString *title = [NSString stringWithUTF8String:(new_title)];
	NSString *accl;
	bool with_shift_key = false;
	if (new_accl != 0) {
		if (new_accl >= 0x100 && new_accl < 0x200) {
			// shift + key
			new_accl -= 0x100;
			with_shift_key = true;
		}
		accl = [NSString stringWithCharacters:&new_accl length:1];
	} else {
		accl = @"";
	}

	menuItem = [[CocoaMenuItem alloc] initWithTitle:title action:new_action drv:new_drv num:new_num keyEquivalent:accl];

	[menuItem setKeyEquivalentModifierMask:(NSEventModifierFlagOption | (with_shift_key ? NSEventModifierFlagShift : 0))];
	[menuItem setTarget:new_target];
    [self addItem:menuItem];
	//  [menuItem release];
	//	[accl release];
	//	[title release];
	return menuItem;
}
- (CocoaMenuItem *)add_menu_item_by_id:(CMsg::Id)new_titleid :(NSObject *)new_target :(SEL)new_action :(int)new_drv :(int)new_num :(unichar)new_accl
{
	return [self add_menu_item:gMessages.Get(new_titleid):new_target:new_action:new_drv:new_num:new_accl];
}
- (void)add_sub_menu:(CocoaMenu *)submenu :(const char *)new_title
{
	[self add_sub_menu:submenu:new_title:nil:nil:0:0];
}
- (void)add_sub_menu_by_id:(CocoaMenu *)submenu :(CMsg::Id)new_titleid
{
	[self add_sub_menu_by_id:submenu:new_titleid:nil:nil:0:0];
}
- (void)add_sub_menu:(CocoaMenu *)submenu :(const char *)new_title :(NSObject *)new_target :(SEL)new_action :(int)new_drv :(int)new_num
{
	CocoaMenuItem *menuItem;
	NSString *title = [[NSString alloc] initWithUTF8String:(new_title)];

	menuItem = [[CocoaMenuItem alloc] initWithTitle:title action:new_action drv:new_drv num:new_num keyEquivalent:@""];
	if (new_target != nil) [menuItem setTarget:new_target];
	[menuItem setSubmenu:submenu];
	[self addItem:menuItem];
	//	[submenu release];
	//	[menuItem release];
}
- (void)add_sub_menu_by_id:(CocoaMenu *)submenu :(CMsg::Id)new_titleid :(NSObject *)new_target :(SEL)new_action :(int)new_drv :(int)new_num
{
	[self add_sub_menu:submenu:gMessages.Get(new_titleid):new_target:new_action:new_drv:new_num];
}
- (void)setTitleById:(CMsg::Id)new_id
{
	NSString *title = [[NSString alloc] initWithUTF8String:gMessages.Get(new_id)];
	[self setTitle:title];
}

@end


@implementation CocoaMenuDelegate
+ (CocoaMenuDelegate *)create
{
	CocoaMenuDelegate *me = [[CocoaMenuDelegate alloc]init];
	return me;
}
- (BOOL)menuHasKeyEquivalent:(NSMenu *)menu forEvent:(NSEvent *)event target:(id *)new_target action:(SEL *)new_action
{
	if ([event modifierFlags] & NSEventModifierFlagOption) {
		if (new_target != nil) *new_target = nil;
		if (new_action != nil) *new_action = nil;
		return YES;
	} else {
		return NO;
	}
}

@end


@implementation CocoaController
@synthesize gui;

- (void)UpdateRecentFiles:(CocoaMenuItem *)menuItem :(CRecentPathList &)list :(int)drv :(SEL)action
{
	CocoaMenu *menu = (CocoaMenu *)[menuItem submenu];
	if (menu != nil) {
		bool flag = false;
		char str[_MAX_PATH];
		[menu removeAllItems];
		for(int i = 0; i < list.Count(); i++) {
			if (!gui->GetRecentFileStr(list[i]->path.Get(), list[i]->num, str, 64)) break;
			[menu add_menu_item:str:self:action:drv:i:0];
			flag = true;
		}
		if (!flag) {
			[menu add_menu_item_by_id:CMsg::None_:nil:nil:0:0:0];
		}
	}
}

// Control

- (void)Reset:(id)sender
{
	gui->PostEtReset();
}
- (void)SpecialReset:(id)sender
{
	gui->PostEtSpecialReset();
}
- (void)WarmReset:(id)sender
{
	gui->PostEtWarmReset(-1);
}
- (void)Dipswitch:(id)sender
{
	gui->Dipswitch([sender num]);
}
- (void)TogglePause:(id)sender
{
	gui->TogglePause();
}
- (void)CPUPower:(id)sender
{
	gui->PostEtCPUPower([sender num]);
}
#ifdef _MBS1
- (void)ChangeSystemMode:(id)sender
{
	gui->ChangeSystemMode([sender num]);
}
#endif
- (void)ChangeFddType:(id)sender
{
	gui->ChangeFddType([sender num]);
}
- (void)ToggleSyncIRQ:(id)sender
{
	gui->PostEtToggleSyncIRQ();
}
#ifdef _MBS1
- (void)ToggleMemNoWait:(id)sender
{
	gui->ToggleMemNoWait();
}
#endif
- (void)ShowOpenAutoKeyDialog:(id)sender
{
	gui->ShowOpenAutoKeyDialog();
}
- (void)StartAutoKey:(id)sender
{
	gui->PostEtStartAutoKeyMessage();
}
- (void)StopAutoKey:(id)sender
{
	gui->PostEtStopAutoKeyMessage();
}
- (void)ShowPlayRecKeyDialog:(id)sender
{
	gui->ShowPlayRecKeyDialog();
}
- (void)StopPlayRecKey:(id)sender
{
	gui->StopPlayRecKey();
}
- (void)ShowRecordRecKeyDialog:(id)sender
{
	gui->ShowRecordRecKeyDialog();
}
- (void)ShowRecordStateAndRecKeyDialog:(id)sender
{
	if (gui->ShowSaveStateDialog(true)) {
		gui->ShowRecordRecKeyDialog();
	}
}
- (void)StopRecordRecKey:(id)sender
{
	gui->StopRecordRecKey();
}
- (void)ShowLoadStateDialog:(id)sender
{
	gui->ShowLoadStateDialog();
}
- (void)ShowSaveStateDialog:(id)sender
{
	gui->ShowSaveStateDialog(false);
}
- (void)LoadRecentState:(id)sender
{
	gui->PostEtLoadRecentStatusMessage([sender num]);
}
- (void)UpdateRecentStateList:(id)sender
{
}

// Tape

#ifdef USE_DATAREC
- (void)ShowLoadDataRecDialog:(id)sender
{
	gui->ShowLoadDataRecDialog();
}
- (void)ShowSaveDataRecDialog:(id)sender
{
	gui->ShowSaveDataRecDialog();
}
- (void)RewindDataRec:(id)sender
{
	gui->PostEtRewindDataRecMessage();
}
- (void)FastForwardDataRec:(id)sender
{
	gui->PostEtFastForwardDataRecMessage();
}
- (void)CloseDataRec:(id)sender
{
	gui->PostEtCloseDataRecMessage();
}
- (void)StopDataRec:(id)sender
{
	gui->PostEtStopDataRecMessage();
}
- (void)ToggleRealModeDataRec:(id)sender
{
	gui->PostEtToggleRealModeDataRecMessage();
}
- (void)LoadRecentDataRec:(id)sender
{
	gui->PostEtLoadRecentDataRecMessage([sender num]);
}
- (void)UpdateRecentDataRecList:(id)sender
{
}
#endif

// FDD

#ifdef USE_FD1
- (void)ShowOpenFloppyDiskDialog:(id)sender
{
	int drv = [sender drv];

	gui->ShowOpenFloppyDiskDialog(drv);
}
- (void)ChangeSideFloppyDisk:(id)sender
{
	int drv = [sender drv];
	gui->PostEtChangeSideFloppyDisk(drv);
}
- (void)ShowOpenBlankFloppyDiskDialog:(id)sender
{
	int drv = [sender drv];
	int num = [sender num];

	gui->ShowOpenBlankFloppyDiskDialog(drv, (uint8_t)num);
}
- (void)CloseFloppy:(id)sender
{
	int drv = [sender drv];
	gui->PostEtCloseFloppyMessage(drv);
}
- (void)ToggleWriteProtectFloppyDisk:(id)sender
{
	int drv = [sender drv];
	gui->PostEtToggleWriteProtectFloppyDisk(drv);
}
- (void)OpenRecentFloppy:(id)sender
{
	int drv = [sender drv];
	int num = [sender num];
	gui->PostEtOpenRecentFloppyMessage(drv, num);
}
- (void)OpenFloppySelectedVolume:(id)sender
{
	int drv = [sender drv];
	int num = [sender num];
	gui->PostEtOpenFloppySelectedVolume(drv, num);
}
- (void)UpdateRecentFloppyList:(id)sender
{
}
- (void)UpdateVolumeFloppyList:(id)sender
{
}
#endif

// HDD

#ifdef USE_HD1
- (void)ShowOpenHardDiskDialog:(id)sender
{
	int drv = [sender drv];

	gui->ShowOpenHardDiskDialog(drv);
}
- (void)CloseHardDisk:(id)sender
{
	int drv = [sender drv];
	gui->PostEtCloseHardDiskMessage(drv);
}
- (void)ShowOpenBlankHardDiskDialog:(id)sender
{
	int drv = [sender drv];
	int num = [sender num];

	gui->ShowOpenBlankHardDiskDialog(drv, (uint8_t)num);
}
- (void)OpenRecentHardDisk:(id)sender
{
	int drv = [sender drv];
	int num = [sender num];
	gui->PostEtOpenRecentHardDiskMessage(drv, num);
}
- (void)UpdateRecentHardDiskList:(id)sender
{
}
#endif

// Screen

- (void)ChangeFrameRate:(id)sender
{
	int num = [sender num];
	gui->ChangeFrameRate(num);
}
- (void)ResizeRecordVideoSurface:(id)sender
{
	int num = [sender num];
	gui->PostEtResizeRecordVideoSurface(num);
}
- (void)ChangeRecordVideoFrameRate:(id)sender
{
	int num = [sender num];
	gui->PostEtStartRecordVideo(num);
}
- (void)ShowRecordVideoDialog:(id)sender
{
	int num = [sender num];
	gui->ShowRecordVideoAndAudioDialog(num);
}
- (void)StopRecordVideo:(id)sender
{
	gui->PostEtStopRecordVideo();
}
- (void)CaptureScreen:(id)sender
{
	gui->PostEtCaptureScreen();
}
- (void)ChangeWindowMode:(id)sender
{
	int num = [sender num];
	gui->ChangeWindowMode(num);
}
- (void)ChangeFullScreenMode:(id)sender
{
	int num = [sender num];
	gui->ChangeFullScreenMode(num);
}
- (void)ToggleStretchScreen:(id)sender
{
	int num = [sender num];
	gui->ChangeStretchScreen(num);
}
//- (void)ToggleCutoutScreen:(id)sender
//{
//	gui->ChangeStretchScreen(2);
//}
- (void)ChangePixelAspectMode:(id)sender
{
	int num = [sender num];
	gui->ChangePixelAspect(num);
}
- (void)ChangeScanLine:(id)sender
{
	int num = [sender num];
	gui->PostEtChangeDrawMode(num);
}
#ifdef USE_AFTERIMAGE
- (void)ChangeAfterImage:(id)sender
{
	int num = [sender num];
	gui->PostEtChangeAfterImage(num);
}
#endif
#ifdef USE_KEEPIMAGE
- (void)ChangeKeepImage:(id)sender
{
	int num = [sender num];
	gui->PostEtChangeKeepImage(num);
}
#endif
#ifdef _MBS1
- (void)ChangeRGBType:(id)sender
{
	int num = [sender num];
	gui->ChangeRGBType(num);
}
#endif
- (void)ChangeUseOpenGL:(id)sender
{
	int num = [sender num];
	gui->ChangeUseOpenGL(num);
}
- (void)ChangeOpenGLFilter:(id)sender
{
	gui->ChangeOpenGLFilter(-1);
}

// Sound

- (void)ShowVolumeDialog:(id)sender
{
	gui->ShowVolumeDialog();
}
- (void)ShowRecordAudioDialog:(id)sender
{
	gui->ShowRecordAudioDialog();
}
- (void)StopRecordSound:(id)sender
{
	gui->PostEtStopRecordSound();
}
- (void)ChangeSoundFrequency:(id)sender
{
	int num = [sender num];
	gui->ChangeSoundFrequency(num);
}
- (void)ChangeSoundLatency:(id)sender
{
	int num = [sender num];
	gui->ChangeSoundLatency(num);
}

// Devices

- (void)ShowSavePrinterDialog:(id)sender
{
	int drv = [sender drv];

	gui->ShowSavePrinterDialog(drv);
}
- (void)PrintPrinter:(id)sender
{
	int drv = [sender drv];
	gui->PostEtPrintPrinterMessage(drv);
}
- (void)ClearPrinterBuffer:(id)sender
{
	int drv = [sender drv];
	gui->PostEtClearPrinterBufferMessage(drv);
}
- (void)EnablePrinterDirect:(id)sender
{
	int drv = [sender drv];
	gui->PostEtEnablePrinterDirectMessage(drv);
}
- (void)TogglePrinterOnline:(id)sender
{
	int drv = [sender drv];
	gui->PostEtTogglePrinterOnlineMessage(drv);
}
- (void)EnableCommServer:(id)sender
{
	int drv = [sender drv];
//	gui->PostEtEnableCommServerMessage(drv);
	gui->ToggleEnableCommServer(drv);
}
- (void)ConnectComm:(id)sender
{
	int drv = [sender drv];
	int num = [sender num];
//	gui->PostEtConnectCommMessage(drv);
	gui->ToggleConnectComm(drv, num);
}
- (void)CommThroughMode:(id)sender
{
	int drv = [sender drv];
	gui->ToggleCommThroughMode(drv);
}
- (void)CommBinaryMode:(id)sender
{
	int drv = [sender drv];
	gui->ToggleCommBinaryMode(drv);
}
- (void)SendCommTelnetCommand:(id)sender
{
	int drv = [sender drv];
	int num = [sender num];
	gui->SendCommTelnetCommand(drv, num);
}
- (void)UpdateCommConnectList:(id)sender
{
}

// Options

- (void)ToggleLedBox:(id)sender
{
	int num = [sender num];
	switch(num) {
	case 1:
		gui->ToggleShowLedBox();
		break;
	case 2:
		gui->ToggleInsideLedBox();
		break;
	default:
	gui->ToggleLedBox();
		break;
	}
}
#if 0
- (void)ToggleShowLedBox:(id)sender
{
	gui->ToggleShowLedBox();
}
- (void)ToggleInsideLedBox:(id)sender
{
	gui->ToggleInsideLedBox();
}
#endif
- (void)ToggleMessageBoard:(id)sender
{
	gui->ToggleMessageBoard();
}
#ifdef USE_PERFORMANCE_METER
- (void)TogglePMeter:(id)sender
{
	gui->TogglePMeter();
}
#endif
- (void)ChangeUseJoypad:(id)sender
{
	int num = [sender num];
	gui->ChangeUseJoypad(num);
}
#ifdef USE_KEY2JOYSTICK
- (void)ToggleEnableKey2Joypad:(id)sender
{
	gui->ToggleEnableKey2Joypad();
}
#endif
#ifdef USE_LIGHTPEN
- (void)ToggleEnableLightpen:(id)sender
{
	gui->ToggleEnableLightpen();
}
#endif
#ifdef USE_MOUSE
- (void)ToggleUseMouse:(id)sender
{
	gui->PostMtToggleUseMouse();
}
#endif
- (void)ToggleLoosenKeyStroke:(id)sender
{
	gui->ToggleLoosenKeyStroke();
}
- (void)ShowJoySettingDialog:(id)sender
{
	gui->ShowJoySettingDialog();
}
- (void)ShowKeybindDialog:(id)sender
{
	gui->ShowKeybindDialog();
}
- (void)ShowConfigureDialog:(id)sender
{
	gui->ShowConfigureDialog();
}
- (void)ShowVirtualKeyboard:(id)sender
{
	gui->ShowVirtualKeyboard();
}
- (void)ShowLoggingDialog:(id)sender
{
	gui->ShowLoggingDialog();
}

#ifdef USE_DEBUGGER
- (void)OpenDebugger:(id)sender
{
//	int num = [sender num];

	gui->OpenDebugger();
}
- (void)CloseDebugger:(id)sender
{
	gui->CloseDebugger();
}
#endif

/// called from emu thread
- (void)PerformUpdateScreen
{
	gui->UpdateScreen();
}

///
/// Menu state and enable check
///
- (BOOL)validateMenuItem:(CocoaMenuItem *)menuItem
{
	int state = NSControlStateValueOff;
	BOOL enable = TRUE;
	SEL act = [menuItem action];
	int drv = [menuItem drv];

	if (act == @selector(Reset:)) {
		if (!pConfig->now_power_off) {
			state = NSControlStateValueOn;
		}
#ifdef _BML3MK5
	} else if (act == @selector(Dipswitch:)) {
		int num = (1 << [menuItem num]);
		if (pConfig->dipswitch & num) {
			state = NSControlStateValueOn;
		}
#endif
#ifdef _MBS1
	} else if (act == @selector(ChangeSystemMode:)) {
		if ([menuItem num] == gui->GetSystemMode()) {
			state = NSControlStateValueOn;
		}
#endif
	} else if (act == @selector(TogglePause:)) {
		if (gui->NowPause()) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(ToggleSyncIRQ:)) {
		if (gui->NowSyncIRQ()) {
			state = NSControlStateValueOn;
		}
#ifdef _MBS1
	} else if (act == @selector(ToggleMemNoWait:)) {
		if (gui->NowMemNoWait()) {
			state = NSControlStateValueOn;
		}
#endif
	} else if (act == @selector(CPUPower:)) {
		if ([menuItem num] == pConfig->cpu_power) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(ChangeFddType:)) {
		if ([menuItem num] == gui->NextFddType()) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(ShowOpenAutoKeyDialog:)
		    || act == @selector(StartAutoKey:)) {
		if (gui->IsRunningAutoKey()) {
			enable = FALSE;
		}
//	} else if (act == @selector(StopAutoKey:)) {
//		if (!gui->IsRunningAutoKey()) {
//			enable = FALSE;
//		}
	} else if (act == @selector(ShowPlayRecKeyDialog:)) {
		if (gui->NowPlayingRecKey()) {
			state = NSControlStateValueOn;
		}
//	} else if (act == @selector(StopPlayRecKey:)) {
//		if (!gui->NowPlayingRecKey()) {
//			enable = FALSE;
//		}
	} else if (act == @selector(ShowRecordStateAndRecKeyDialog:)) {
		if (gui->NowRecordingRecKey()) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(UpdateRecentStateList:)) {
		[self UpdateRecentFiles:menuItem:pConfig->GetRecentStatePathList():0:@selector(LoadRecentState:)];
//	} else if (act == @selector(StopRecordRecKey:)) {
//		if (!gui->NowRecordingRecKey()) {
//			enable = FALSE;
//		}
#ifdef USE_DATAREC
	} else if (act == @selector(ShowLoadDataRecDialog:)) {
		if (gui->IsOpenedLoadDataRecFile()) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(ShowSaveDataRecDialog:)) {
		if (gui->IsOpenedSaveDataRecFile()) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(ToggleRealModeDataRec:)) {
		if (pConfig->NowRealModeDataRec()) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(RewindDataRec:)
			|| act == @selector(FastForwardDataRec:)
			|| act == @selector(CloseDataRec:)
			|| act == @selector(StopDataRec:)) {
		if (!(gui->IsOpenedSaveDataRecFile() || gui->IsOpenedLoadDataRecFile())) {
			enable = FALSE;
		}
	} else if (act == @selector(UpdateRecentDataRecList:)) {
		[self UpdateRecentFiles:menuItem:pConfig->GetRecentDataRecPathList():0:@selector(LoadRecentDataRec:)];
#endif
#ifdef USE_FD1
	} else if (act == @selector(ShowOpenFloppyDiskDialog:)) {
		if (gui->InsertedFloppyDisk(drv)) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(ChangeSideFloppyDisk:)) {
		CMsg::Id new_id;
		int side = gui->GetSideFloppyDisk(drv);
		if (side == 1 && pConfig->fdd_type == 1) {
			new_id = CMsg::Change_Side_to_A;
		} else {
			new_id = CMsg::Change_Side_to_B;
		}
		[menuItem setTitleById:new_id];
		if (side < 0 || pConfig->fdd_type != 1) {
			enable = FALSE;
		}
	} else if (act == @selector(CloseFloppy:)) {
		if (!gui->InsertedFloppyDisk(drv)) {
			enable = FALSE;
		}
	} else if (act == @selector(ToggleWriteProtectFloppyDisk:)) {
		if (!gui->InsertedFloppyDisk(drv)) {
			enable = FALSE;
		} else if (gui->WriteProtectedFloppyDisk(drv)) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(UpdateRecentFloppyList:)) {
		[self UpdateRecentFiles:menuItem:pConfig->GetRecentFloppyDiskPathList(drv):drv:@selector(OpenRecentFloppy:)];
	} else if (act == @selector(UpdateVolumeFloppyList:)) {
		CocoaMenu *menu = (CocoaMenu *)[menuItem submenu];
		if (menu != nil) {
			bool flag = false;
			char str[32];
			[menu removeAllItems];
			D88File *d88_file = gui->GetD88File(drv);
			int bank_nums = d88_file->GetBanks().Count();
			for(int num = 0; num < bank_nums; num++) {
				gui->GetMultiVolumeStr(num, d88_file->GetBank(num)->GetName(), str, 32);
				[menu add_menu_item:str:self:@selector(OpenFloppySelectedVolume:):drv:num:0];
				flag = true;
			}
			if (!flag) {
				[menu add_menu_item_by_id:CMsg::None_:nil:nil:0:0:0];
			}
		}
	} else if (act == @selector(OpenFloppySelectedVolume:)) {
		int num = [menuItem num];
		D88File *d88_file = gui->GetD88File(drv);
		if (d88_file->GetCurrentBank() == num) {
			state = NSControlStateValueOn;
		}
		if (d88_file->GetBanks().Count() == 1) {
			enable = FALSE;
		}
#endif
#ifdef USE_HD1
	} else if (act == @selector(ShowOpenHardDiskDialog:)) {
		if (gui->MountedHardDisk(drv)) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(CloseHardDisk:)) {
		if (!gui->MountedHardDisk(drv)) {
			enable = FALSE;
		}
	} else if (act == @selector(UpdateRecentHardDiskList:)) {
		[self UpdateRecentFiles:menuItem:pConfig->GetRecentHardDiskPathList(drv):drv:@selector(OpenRecentHardDisk:)];
#endif
	} else if (act == @selector(ChangeFrameRate:)) {
		int num = [menuItem num];
		if (gui->GetFrameRateNum() == num) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(ResizeRecordVideoSurface:)) {
		int num = [menuItem num];
		if (gui->GetRecordVideoSurfaceNum() == num) {
			state = NSControlStateValueOn;
		}
		if (gui->NowRecordingVideo() || gui->NowRecordingSound()) {
			enable = FALSE;
		}
	} else if (act == @selector(ShowRecordVideoDialog:)) {
		int num = [menuItem num];
		if (gui->GetRecordVideoFrameNum() == num) {
			state = NSControlStateValueOn;
		}
		if (gui->NowRecordingVideo() || gui->NowRecordingSound()) {
			enable = FALSE;
		}
//	} else if (act == @selector(StopRecordVideo:)) {
//		if (!(gui->NowRecordingVideo() || gui->NowRecordingSound())) {
//			enable = FALSE;
//		}
	} else if (act == @selector(ChangeWindowMode:)) {
		int num = [menuItem num];
		if (gui->GetWindowMode() == num) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(ChangeFullScreenMode:)) {
		int num = [menuItem num];
		if (gui->IsFullScreen()) {
			if (gui->GetFullScreenMode() == num) {
				state = NSControlStateValueOn;
			}
			enable = FALSE;
		}
	} else if (act == @selector(ToggleStretchScreen:)) {
		int num = [menuItem num];
		if (gui->GetStretchScreen() == num) {
			state = NSControlStateValueOn;
		}
//	} else if (act == @selector(ToggleCutoutScreen:)) {
//		if (gui->GetStretchScreen() == 2) {
//			state = NSControlStateValueOn;
//		}
	} else if (act == @selector(ChangePixelAspectMode:)) {
		int num = [menuItem num];
		if (gui->GetPixelAspectMode() == num) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(ChangeScanLine:)) {
		int num = [menuItem num];
		if (gui->GetDrawMode() == num) {
			state = NSControlStateValueOn;
		}
#ifdef USE_AFTERIMAGE
	} else if (act == @selector(ChangeAfterImage:)) {
		int num = [menuItem num];
		if (gui->GetAfterImageMode() == num) {
			state = NSControlStateValueOn;
		}
#endif
#ifdef USE_KEEPIMAGE
	} else if (act == @selector(ChangeKeepImage:)) {
		int num = [menuItem num];
		if (gui->GetKeepImageMode() == num) {
			state = NSControlStateValueOn;
		}
#endif
#ifdef _MBS1
	} else if (act == @selector(ChangeRGBType:)) {
		int num = [menuItem num];
		if (gui->GetRGBTypeMode() == num) {
			state = NSControlStateValueOn;
		}
#endif
	} else if (act == @selector(ChangeUseOpenGL:)) {
		int num = [menuItem num];
		if (gui->GetOpenGLMode() == num) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(ChangeOpenGLFilter:)) {
		int num = [menuItem num];
		if (gui->GetOpenGLFilter() == num) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(ShowRecordAudioDialog:)) {
		if (gui->NowRecordingVideo() || gui->NowRecordingSound()) {
			state = NSControlStateValueOn;
			enable = FALSE;
		}
//	} else if (act == @selector(StopRecordSound:)) {
//		if (!(gui->NowRecordingVideo() || gui->NowRecordingSound())) {
//			enable = FALSE;
//		}
	} else if (act == @selector(ChangeSoundFrequency:)) {
		int num = [menuItem num];
		if (gui->GetSoundFrequencyNum() == num) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(ChangeSoundLatency:)) {
		int num = [menuItem num];
		if (gui->GetSoundLatencyNum() == num) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(ShowSavePrinterDialog:) || act == @selector(PrintPrinter:)) {
		int drv = [menuItem drv];
		if (gui->GetPrinterBufferSize(drv) <= 0) {
			enable = FALSE;
		}
	} else if (act == @selector(EnablePrinterDirect:)) {
		int drv = [menuItem drv];
		if (gui->IsEnablePrinterDirect(drv)) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(TogglePrinterOnline:)) {
		int drv = [menuItem drv];
		if (gui->IsOnlinePrinter(drv)) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(EnableCommServer:)) {
		int drv = [menuItem drv];
		if (gui->IsEnableCommServer(drv)) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(ConnectComm:)) {
		int drv = [menuItem drv];
		int num = [menuItem num];
		if (gui->NowConnectingComm(drv, num)) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(CommThroughMode:)) {
		int drv = [menuItem drv];
		if (gui->NowCommThroughMode(drv)) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(CommBinaryMode:)) {
		int drv = [menuItem drv];
		if (gui->NowCommBinaryMode(drv)) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(UpdateCommConnectList:)) {
		CocoaMenu *menu = (CocoaMenu *)[menuItem submenu];
		if (menu != nil) {
			int uarts = gui->EnumUarts();
			[menu removeAllItems];
			[menu add_menu_item_by_id:CMsg::Ethernet:self:@selector(ConnectComm:):drv:0:0];
			if (uarts > 0) {
				[menu addItem:[NSMenuItem separatorItem]];
			}
			char buf[128];
			for(int i=0; i<uarts; i++) {
				gui->GetUartDescription(i, buf, sizeof(buf));
				[menu add_menu_item:buf:self:@selector(ConnectComm:):drv:i+1:0];
			}
		}
	} else if (act == @selector(ToggleLedBox:)) {
		int num = [menuItem num];
		switch(num) {
		case 1:
		if (gui->IsShownLedBox()) {
			state = NSControlStateValueOn;
		}
			break;
		case 2:
		if (gui->IsInsidedLedBox()) {
			state = NSControlStateValueOn;
		}
			break;
		}
//	} else if (act == @selector(ToggleInsideLedBox:)) {
//		if (gui->IsInsidedLedBox()) {
//			state = NSControlStateValueOn;
//		}
	} else if (act == @selector(ToggleMessageBoard:)) {
		if (gui->IsShownMessageBoard()) {
			state = NSControlStateValueOn;
		}
#ifdef USE_PERFORMANCE_METER
	} else if (act == @selector(TogglePMeter:)) {
		if (gui->IsShownPMeter()) {
			state = NSControlStateValueOn;
		}
#endif
#ifdef USE_JOYSTICK
	} else if (act == @selector(ChangeUseJoypad:)) {
		int num = [menuItem num];
		if (gui->IsEnableJoypad(num)) {
			state = NSControlStateValueOn;
		}
#endif
#ifdef USE_KEY2JOYSTICK
	} else if (act == @selector(ToggleEnableKey2Joypad:)) {
		if (gui->IsEnableKey2Joypad()) {
			state = NSControlStateValueOn;
		}
#endif
#ifdef USE_LIGHTPEN
	} else if (act == @selector(ToggleEnableLightpen:)) {
		if (gui->IsEnableLightpen()) {
			state = NSControlStateValueOn;
		}
#endif
#ifdef USE_MOUSE
	} else if (act == @selector(ToggleUseMouse:)) {
		if (gui->IsEnableMouse()) {
			state = NSControlStateValueOn;
		}
#endif
	} else if (act == @selector(ToggleLoosenKeyStroke:)) {
		if (gui->IsLoosenKeyStroke()) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(ShowVirtualKeyboard:)) {
		if (gui->IsShownVirtualKeyboard()) {
			state = NSControlStateValueOn;
		}
	} else if (act == @selector(ShowLoggingDialog:)) {
		if (gui->IsShownLoggingDialog()) {
			state = NSControlStateValueOn;
		}
#ifdef USE_DEBUGGER
	} else if (act == @selector(OpenDebugger:)) {
//		int num = [menuItem num];
		if (gui->IsDebuggerOpened()) {
			enable = FALSE;
		}
#endif
	}
	[menuItem setState:state];
	return enable;
}

@end

#ifdef USE_DELEGATE
@implementation CocoaRecentDataRecList
- (id)initWithTarget:(CocoaController *)new_target
{
	[super init];
	target = new_target;
	return self;
}

- (void)menuWillOpen:(CocoaMenu *)menu
{
	if (menu == nil) return;

	for(int i = 0; i < MAX_HISTORY; i++) {
		if (strlen(pConfig->recent_datarec_path[i]) == 0) break;
		[Util add_menu_item:menu:pConfig->recent_datarec_path[i]:target:@selector(LoadRecentDataRec:):0:i:0];
	}
}
- (void)menuDidClose:(CocoaMenu *)menu
{
	if (menu == nil) return;

	[menu removeAllItems];
}
@end

@implementation CocoaRecentFloppyDiskList
- (id)initWithTarget:(CocoaController *)new_target :(int)new_drv
{
	[super init];
	target = new_target;
	drv = new_drv;
	return self;
}

- (void)menuWillOpen:(CocoaMenu *)menu
{
	if (menu == nil) return;

	for(int i = 0; i < MAX_HISTORY; i++) {
		if (strlen(pConfig->recent_disk_path[drv][i]) == 0) break;
		[Util add_menu_item:menu:pConfig->recent_disk_path[drv][i]:target:@selector(OpenRecentFloppy:):drv:i:0];
	}
}
- (void)menuDidClose:(CocoaMenu *)menu
{
	if (menu == nil) return;

	[menu removeAllItems];
}
@end

@implementation CocoaVolumeFloppyDiskList
- (id)initWithTarget:(CocoaController *)new_target :(int)new_drv
{
	[super init];
	target = new_target;
	drv = new_drv;
	return self;
}

- (void)menuWillOpen:(CocoaMenu *)menu
{
	if (menu == nil) return;

	GUI *gui = target.gui;

	for(int num = 0; num < gui->d88_file[drv].bank_num; num++) {
		char name[32];
		sprintf(name, "%02d: %s", num+1
			, strlen(gui->d88_file[drv].bank[num].name) > 0 ? gui->d88_file[drv].bank[num].name : CMSG(no_label));;
		CocoaMenuItem *item = [Util add_menu_item:menu:name:target:@selector(OpenFloppySelectedVolume:):drv:num:0];
		if (item && gui->d88_file[drv].cur_bank == num) {
			[item setState:NSControlStateValueOn];
		}
	}
}
- (void)menuDidClose:(CocoaMenu *)menu
{
	if (menu == nil) return;

	[menu removeAllItems];
}
@end
#endif

void add_main_menu(CocoaMenu *submenu, const char *new_title)
{
	CocoaMenuItem *menuItem;
	NSString *title = [[NSString alloc] initWithUTF8String:(new_title)];

	menuItem = [[CocoaMenuItem alloc] initWithTitle:title action:nil drv:0 num:0 keyEquivalent:@""];
	[menuItem setSubmenu:submenu];
	[[NSApp mainMenu] addItem:menuItem];
//	[submenu release];
//	[menuItem release];
}
void add_main_menu_by_id(CocoaMenu *submenu, CMsg::Id new_titleid)
{
	add_main_menu(submenu, gMessages.Get(new_titleid));
}

NSArray *get_file_filter(const char *str)
{
	int pos = 0;
	char word[8];
	int word_len = 0;
	int len = (int)strlen(str);
	NSMutableArray *file_types = [NSMutableArray array];
	do {
		pos = UTILITY::get_token(str, pos, len, word, (int)sizeof(word), ';', &word_len);
		if (word_len > 0) {
			[file_types addObject:[NSString stringWithUTF8String:word]];
		}
	} while (pos >= 0);
	
	return file_types;
}


GUI::GUI(int argc, char **argv, EMU *new_emu) : GUI_BASE(argc, argv, new_emu)
{
	recv = [[CocoaController alloc] init];
	[recv setGui:this];
	logging_dlg = nil;
}

GUI::~GUI()
{
	[recv release];
}

#ifndef USE_SDL2
int GUI::CreateWidget(SDL_Surface *screen, int width, int height)
{
	set_delegate_to_sdl_window(this);
	return 0;
}
#else
int GUI::CreateWidget(SDL_Window *window, int width, int height)
{
	set_delegate_to_sdl_window(this);
	return 0;
}
#endif

int GUI::CreateMenu()
{
	GUI_BASE::CreateMenu();

    remove_window_menu();
	translate_apple_menu();
    setup_menu();
    return 0;
}
void GUI::ShowMenu()
{
//	if ([NSMenu menuBarVisible] != YES) {
//		[NSMenu setMenuBarVisible:YES];
//	}
}
void GUI::HideMenu()
{
//	if ([NSMenu menuBarVisible] != NO) {
//		[NSMenu setMenuBarVisible:NO];
//	}
}
void GUI::PreProcessEvent()
{
	// nothing to do
}
int GUI::ProcessEvent(SDL_Event *e)
{
	int rc = GUI_BASE::ProcessEvent(e);
	if (rc <= 0) return rc;

	// TODO: cannot visible menubar in fullscreen
#if 0
	if (e->type == SDL_MOUSEBUTTONDOWN) {
		SDL_MouseButtonEvent *me = (SDL_MouseButtonEvent *)e;
		if (me->button == SDL_BUTTON_RIGHT) {
			ShowMenu();
			[NSMenu popUpContextMenu:popupMenu withEvent:nil forView:nil];
		}
	} else
#endif
#if 0
	if (e->type == SDL_MOUSEMOTION) {
		SDL_MouseMotionEvent *me = (SDL_MouseMotionEvent *)e;
		if (IsFullScreen()) {
			if(me->y == 0) {
				CGReleaseAllDisplays();
				NSWindow *w = get_main_window();
				if (w) {
					[w setLevel:0];
				}
			}
			else if(me->y > 20) {
				CGCaptureAllDisplays();
				NSWindow *w = get_main_window();
				if (w) {
					long lv = CGShieldingWindowLevel();
					[w setLevel:lv];
				}
			}
			return 0;
		}
	}
#endif
	return 1;
}

void GUI::ScreenModeChanged(bool fullscreen)
{
}

void GUI::SetFocusToMainWindow()
{
#ifndef USE_SDL2
	NSWindow *win = get_main_window();
	if (win) {
		[win makeKeyAndOrderFront:nil];
	}
#else
	SDL_RaiseWindow(((EMU_OSD *)emu)->get_window());
#endif
}

/* Create menu */
void GUI::setup_menu(void)
{
	int i;
	int drv;
	char name[128];

	// control menu

	CocoaMenu *controlMenu = [CocoaMenu create_menu_by_id:CMsg::Control];

	[controlMenu add_menu_item_by_id:CMsg::PowerOn:recv:@selector(Reset:):0:0:NSF3FunctionKey];
#ifdef _BML3MK5
	[controlMenu add_menu_item_by_id:CMsg::MODE_Switch:recv:@selector(Dipswitch:):0:2:'m'];
#endif
	[controlMenu add_menu_item_by_id:CMsg::Reset_Switch:recv:@selector(WarmReset:):0:0:'r'];

	[controlMenu addItem:[NSMenuItem separatorItem]];

#ifdef _MBS1
	CocoaMenu *systemMenu = [CocoaMenu create_menu_by_id:CMsg::System_Mode];
	[systemMenu add_menu_item_by_id:CMsg::A_Mode_S1:recv:@selector(ChangeSystemMode:):0:1:'m'];
	[systemMenu add_menu_item_by_id:CMsg::B_Mode_L3:recv:@selector(ChangeSystemMode:):0:0:'m'];
	[controlMenu add_sub_menu_by_id:systemMenu:CMsg::System_Mode];

	[controlMenu addItem:[NSMenuItem separatorItem]];
#endif

	CocoaMenu *fddTypeMenu = [CocoaMenu create_menu_by_id:CMsg::FDD_Type];
	[fddTypeMenu add_menu_item_by_id:CMsg::Non_FDD:recv:@selector(ChangeFddType:):0:0:'f'];
	[fddTypeMenu add_menu_item_by_id:CMsg::FD3inch_compact_FDD:recv:@selector(ChangeFddType:):0:1:'f'];
#ifdef _BML3MK5
	[fddTypeMenu add_menu_item_by_id:CMsg::FD5inch_mini_FDD:recv:@selector(ChangeFddType:):0:2:'f'];
	[fddTypeMenu add_menu_item_by_id:CMsg::FD8inch_standard_FDD:recv:@selector(ChangeFddType:):0:3:'f'];
#endif
#ifdef _MBS1
	[fddTypeMenu add_menu_item_by_id:CMsg::FD5inch_mini_FDD_2D_Type:recv:@selector(ChangeFddType:):0:2:'f'];
	[fddTypeMenu add_menu_item_by_id:CMsg::FD5inch_mini_FDD_2HD_Type:recv:@selector(ChangeFddType:):0:3:'f'];
#endif
	[controlMenu add_sub_menu_by_id:fddTypeMenu:CMsg::FDD_Type];

	[controlMenu addItem:[NSMenuItem separatorItem]];

	[controlMenu add_menu_item_by_id:CMsg::Pause:recv:@selector(TogglePause:):0:0:'q'];

	[controlMenu addItem:[NSMenuItem separatorItem]];

	CocoaMenu *cpuSpeedMenu = [CocoaMenu create_menu_by_id:CMsg::CPU_Speed];
	[cpuSpeedMenu add_menu_item_by_id:CMsg::CPU_x0_5:recv:@selector(CPUPower:):0:0:'9'];
	[cpuSpeedMenu add_menu_item_by_id:CMsg::CPU_x1:recv:@selector(CPUPower:):0:1:'1'];
	[cpuSpeedMenu add_menu_item_by_id:CMsg::CPU_x2:recv:@selector(CPUPower:):0:2:'2'];
	[cpuSpeedMenu add_menu_item_by_id:CMsg::CPU_x4:recv:@selector(CPUPower:):0:3:'3'];
	[cpuSpeedMenu add_menu_item_by_id:CMsg::CPU_x8:recv:@selector(CPUPower:):0:4:'4' ];
	[cpuSpeedMenu add_menu_item_by_id:CMsg::CPU_x16:recv:@selector(CPUPower:):0:5:'5'];
	[cpuSpeedMenu addItem:[NSMenuItem separatorItem]];
	[cpuSpeedMenu add_menu_item_by_id:CMsg::Sync_Devices_With_CPU_Speed:recv:@selector(ToggleSyncIRQ:):0:0:'0'];
	[controlMenu add_sub_menu_by_id:cpuSpeedMenu:CMsg::CPU_Speed];

	[controlMenu addItem:[NSMenuItem separatorItem]];

#ifdef _MBS1
//	[controlMenu add_menu_item_by_id:CMsg::Memory_No_Wait:recv:@selector(ToggleMemNoWait:):0:0:'0'];

//	[controlMenu addItem:[NSMenuItem separatorItem]];
#endif

	CocoaMenu *autoKeyMenu = [CocoaMenu create_menu_by_id:CMsg::Auto_Key];
	[autoKeyMenu add_menu_item_by_id:CMsg::Open_:recv:@selector(ShowOpenAutoKeyDialog:):0:0:0];
	[autoKeyMenu add_menu_item_by_id:CMsg::Paste:recv:@selector(StartAutoKey:):0:0:0];
	[autoKeyMenu add_menu_item_by_id:CMsg::Stop:recv:@selector(StopAutoKey:):0:0:0];
	[controlMenu add_sub_menu_by_id:autoKeyMenu:CMsg::Auto_Key];

	[controlMenu addItem:[NSMenuItem separatorItem]];

	CocoaMenu *recKeyMenu = [CocoaMenu create_menu_by_id:CMsg::Record_Key];
	[recKeyMenu add_menu_item_by_id:CMsg::Play_:recv:@selector(ShowPlayRecKeyDialog:):0:0:'e'];
	[recKeyMenu add_menu_item_by_id:CMsg::Stop_Playing:recv:@selector(StopPlayRecKey:):0:0:0];
	[recKeyMenu addItem:[NSMenuItem separatorItem]];
	[recKeyMenu add_menu_item_by_id:CMsg::Record_:recv:@selector(ShowRecordStateAndRecKeyDialog:):0:1:0];
	[recKeyMenu add_menu_item_by_id:CMsg::Stop_Recording:recv:@selector(StopRecordRecKey:):0:0:0];
	[controlMenu add_sub_menu_by_id:recKeyMenu:CMsg::Record_Key];

	[controlMenu addItem:[NSMenuItem separatorItem]];

	[controlMenu add_menu_item_by_id:CMsg::Load_State_:recv:@selector(ShowLoadStateDialog:):0:0:'o'];
	[controlMenu add_menu_item_by_id:CMsg::Save_State_:recv:@selector(ShowSaveStateDialog:):0:0:0];

	[controlMenu addItem:[NSMenuItem separatorItem]];

#ifdef USE_DELEGATE
	CocoaMenu *stateRecentMenu = [CocoaMenu create_menu_by_id:CMsg::Recent_State_Files):[[CocoaRecentStateList alloc] initWithTarget:recv]];
	[controlMenu add_sub_menu_by_id:stateRecentMenu:CMsg::Recent_State_Files)];
#else
	CocoaMenu *stateRecentMenu = [CocoaMenu create_menu_by_id:CMsg::Recent_State_Files];
	[controlMenu add_sub_menu_by_id:stateRecentMenu:CMsg::Recent_State_Files:recv:@selector(UpdateRecentStateList:):0:0];
#endif

	/* Put menu into the menubar */
	add_main_menu_by_id(controlMenu,CMsg::Control);

	// tape menu

#ifdef USE_DATAREC
	CocoaMenu *tapeMenu = [CocoaMenu create_menu_by_id:CMsg::Tape];

	[tapeMenu add_menu_item_by_id:CMsg::Play_:recv:@selector(ShowLoadDataRecDialog:):0:0:NSF7FunctionKey];
	[tapeMenu add_menu_item_by_id:CMsg::Rec_:recv:@selector(ShowSaveDataRecDialog:):0:0:0];
	[tapeMenu add_menu_item_by_id:CMsg::Eject:recv:@selector(CloseDataRec:):0:0:0];

	[tapeMenu addItem:[NSMenuItem separatorItem]];

	[tapeMenu add_menu_item_by_id:CMsg::Rewind:recv:@selector(RewindDataRec:):0:0:NSF5FunctionKey];
	[tapeMenu add_menu_item_by_id:CMsg::F_F_:recv:@selector(FastForwardDataRec:):0:0:NSF8FunctionKey];
	[tapeMenu add_menu_item_by_id:CMsg::Stop:recv:@selector(StopDataRec:):0:0:NSF6FunctionKey];

	[tapeMenu addItem:[NSMenuItem separatorItem]];

	[tapeMenu add_menu_item_by_id:CMsg::Real_Mode:recv:@selector(ToggleRealModeDataRec:):0:0:0];

	[tapeMenu addItem:[NSMenuItem separatorItem]];

#ifdef USE_DELEGATE
	CocoaMenu *tapeRecentMenu = [CocoaMenu create_menu_by_id:CMsg::Recent_Files:[[CocoaRecentDataRecList alloc] initWithTarget:recv]];
	[tapeMenu add_sub_menu_by_id:tapeRecentMenu:CMsg::Recent_Files];
#else
	CocoaMenu *tapeRecentMenu = [CocoaMenu create_menu_by_id:CMsg::Recent_Files];
	[tapeMenu add_sub_menu_by_id:tapeRecentMenu:CMsg::Recent_Files:recv:@selector(UpdateRecentDataRecList:):0:0];
#endif

	/* Put menu into the menubar */
	add_main_menu_by_id(tapeMenu,CMsg::Tape);
#endif

	// fdd menu

	for(drv = 0; drv < USE_FLOPPY_DISKS; drv++) {
		UTILITY::sprintf(name, sizeof(name), CMSG(FDDVDIGIT), drv);

		CocoaMenu *fddMenu = [CocoaMenu create_menu:name];

		[fddMenu add_menu_item_by_id:CMsg::Insert_:recv:@selector(ShowOpenFloppyDiskDialog:):drv:0:(NSF9FunctionKey + drv)];
		[fddMenu add_menu_item_by_id:CMsg::Change_Side_to_A:recv:@selector(ChangeSideFloppyDisk:):drv:0:0];
		[fddMenu add_menu_item_by_id:CMsg::Eject:recv:@selector(CloseFloppy:):drv:0:0];

		CocoaMenu *newBMenu = [CocoaMenu create_menu_by_id:CMsg::New];
		[newBMenu add_menu_item_by_id:CMsg::Insert_Blank_2D_:recv:@selector(ShowOpenBlankFloppyDiskDialog:):drv:0x00:0];
		[newBMenu add_menu_item_by_id:CMsg::Insert_Blank_2HD_:recv:@selector(ShowOpenBlankFloppyDiskDialog:):drv:0x20:0];
		[fddMenu add_sub_menu_by_id:newBMenu:CMsg::New];

		[fddMenu addItem:[NSMenuItem separatorItem]];

		[fddMenu add_menu_item_by_id:CMsg::Write_Protect:recv:@selector(ToggleWriteProtectFloppyDisk:):drv:0:0];

		[fddMenu addItem:[NSMenuItem separatorItem]];

#ifdef USE_DELEGATE
		CocoaMenu *fddVolumeMenu = [CocoaMenu create_menu_by_id:CMsg::Multi_Volume:[[CocoaVolumeFloppyDiskList alloc] initWithTarget:recv:drv]];
		[fddMenu add_sub_menu_by_id:fddVolumeMenu:CMsg::Multi_Volume];
#else
		CocoaMenu *fddVolumeMenu = [CocoaMenu create_menu_by_id:CMsg::Multi_Volume];
		[fddMenu add_sub_menu_by_id:fddVolumeMenu:CMsg::Multi_Volume:recv:@selector(UpdateVolumeFloppyList:):drv:0];
#endif
		[fddMenu addItem:[NSMenuItem separatorItem]];

#ifdef USE_DELEGATE
		CocoaMenu *fddRecentMenu = [CocoaMenu create_menu_by_id:CMsg::Recent_Files:[[CocoaRecentFloppyDiskList alloc] initWithTarget:recv:drv]];
		[fddMenu add_sub_menu_by_id:fddRecentMenu:CMsg::Recent_Files];
#else
		CocoaMenu *fddRecentMenu = [CocoaMenu create_menu_by_id:CMsg::Recent_Files];
		[fddMenu add_sub_menu_by_id:fddRecentMenu:CMsg::Recent_Files:recv:@selector(UpdateRecentFloppyList:):drv:0];
#endif
		/* Put menu into the menubar */
		add_main_menu(fddMenu,name);
	}

	// screen menu

	CocoaMenu *screenMenu = [CocoaMenu create_menu_by_id:CMsg::Screen];

	CocoaMenu *frameRateMenu = [CocoaMenu create_menu_by_id:CMsg::Frame_Rate];
	[frameRateMenu add_menu_item_by_id:CMsg::Auto:recv:@selector(ChangeFrameRate:):0:-1:0];
	[frameRateMenu add_menu_item_by_id:CMsg::F60fps:recv:@selector(ChangeFrameRate:):0:0:0];
	[frameRateMenu add_menu_item_by_id:CMsg::F30fps:recv:@selector(ChangeFrameRate:):0:1:0];
	[frameRateMenu add_menu_item_by_id:CMsg::F20fps:recv:@selector(ChangeFrameRate:):0:2:0];
	[frameRateMenu add_menu_item_by_id:CMsg::F15fps:recv:@selector(ChangeFrameRate:):0:3:0];
	[frameRateMenu add_menu_item_by_id:CMsg::F12fps:recv:@selector(ChangeFrameRate:):0:4:0];
	[frameRateMenu add_menu_item_by_id:CMsg::F10fps:recv:@selector(ChangeFrameRate:):0:5:0];
	[screenMenu add_sub_menu_by_id:frameRateMenu:CMsg::Frame_Rate];

	[screenMenu addItem:[NSMenuItem separatorItem]];

	CocoaMenu *recScreenMenu = [CocoaMenu create_menu_by_id:CMsg::Record_Screen];
	for(i = 0; i < 2; i++) {
		GetRecordVideoSizeStr(i, name);
		[recScreenMenu add_menu_item:name:recv:@selector(ResizeRecordVideoSurface:):0:i:0];
	}
#ifdef USE_REC_VIDEO
	[recScreenMenu addItem:[NSMenuItem separatorItem]];
	[recScreenMenu add_menu_item_by_id:CMsg::Rec_60fps:recv:@selector(ShowRecordVideoDialog:):0:0:0];
	[recScreenMenu add_menu_item_by_id:CMsg::Rec_30fps:recv:@selector(ShowRecordVideoDialog:):0:1:0];
	[recScreenMenu add_menu_item_by_id:CMsg::Rec_20fps:recv:@selector(ShowRecordVideoDialog:):0:2:0];
	[recScreenMenu add_menu_item_by_id:CMsg::Rec_15fps:recv:@selector(ShowRecordVideoDialog:):0:3:0];
	[recScreenMenu add_menu_item_by_id:CMsg::Rec_12fps:recv:@selector(ShowRecordVideoDialog:):0:4:0];
	[recScreenMenu add_menu_item_by_id:CMsg::Rec_10fps:recv:@selector(ShowRecordVideoDialog:):0:5:0];
	[recScreenMenu add_menu_item_by_id:CMsg::Stop:recv:@selector(StopRecordVideo:):0:0:0];
#endif
	[recScreenMenu addItem:[NSMenuItem separatorItem]];
	[recScreenMenu add_menu_item_by_id:CMsg::Capture:recv:@selector(CaptureScreen:):0:0:0];
	[screenMenu add_sub_menu_by_id:recScreenMenu:CMsg::Record_Screen];

	[screenMenu addItem:[NSMenuItem separatorItem]];

	CocoaMenu *windowModeMenu = [CocoaMenu create_menu_by_id:CMsg::Window];
	for(i = 0; i < GetWindowModeCount(); i++) {
		GetWindowModeStr(i, name);
		[windowModeMenu add_menu_item:name:recv:@selector(ChangeWindowMode:):0:i:0];
	}
	[screenMenu add_sub_menu_by_id:windowModeMenu:CMsg::Window];

	CocoaMenu *screenModeMenu = [CocoaMenu create_menu_by_id:CMsg::Fullscreen];
	[screenModeMenu add_menu_item_by_id:CMsg::Stretch_Screen:recv:@selector(ToggleStretchScreen:):0:1:'x'];
	[screenModeMenu add_menu_item_by_id:CMsg::Cutout_Screen:recv:@selector(ToggleStretchScreen:):0:2:'x'];
	[screenModeMenu addItem:[NSMenuItem separatorItem]];
	for(int disp_no = 0; disp_no < GetDisplayDeviceCount(); disp_no++) {
		GetDisplayDeviceStr(CMSG(Display), disp_no, name);
		CocoaMenu *dispDevMenu = [CocoaMenu create_menu:name];
		for(i = 0; i < GetFullScreenModeCount(disp_no); i++) {
			GetFullScreenModeStr(disp_no, i, name);
			int num = disp_no * VIDEO_MODE_MAX + i;
			[dispDevMenu add_menu_item:name:recv:@selector(ChangeFullScreenMode:):0:num:0];
		}
		GetDisplayDeviceStr(CMSG(Display), disp_no, name);
		[screenModeMenu add_sub_menu:dispDevMenu:name];
	}
	[screenMenu add_sub_menu_by_id:screenModeMenu:CMsg::Fullscreen];

	CocoaMenu *pixelAspectMenu = [CocoaMenu create_menu_by_id:CMsg::Aspect_Ratio];
	for(i = 0; i < GetPixelAspectModeCount(); i++) {
		GetPixelAspectModeStr(i, name);
		[pixelAspectMenu add_menu_item:name:recv:@selector(ChangePixelAspectMode:):0:i:0];
	}
	[screenMenu add_sub_menu_by_id:pixelAspectMenu:CMsg::Aspect_Ratio];

	[screenMenu addItem:[NSMenuItem separatorItem]];

	CocoaMenu *drawingMenu = [CocoaMenu create_menu_by_id:CMsg::Drawing_Mode];
		[drawingMenu add_menu_item_by_id:CMsg::Full_Draw:recv:@selector(ChangeScanLine:):0:0:'s'];
		[drawingMenu add_menu_item_by_id:CMsg::Scanline:recv:@selector(ChangeScanLine:):0:1:'s'];
		[drawingMenu add_menu_item_by_id:CMsg::Stripe:recv:@selector(ChangeScanLine:):0:2:'s'];
		[drawingMenu add_menu_item_by_id:CMsg::Checker:recv:@selector(ChangeScanLine:):0:3:'s'];
	[screenMenu add_sub_menu_by_id:drawingMenu:CMsg::Drawing_Mode];

#ifdef USE_AFTERIMAGE
	[screenMenu addItem:[NSMenuItem separatorItem]];

	[screenMenu add_menu_item_by_id:CMsg::Afterimage1:recv:@selector(ChangeAfterImage:):0:1:'t'];
	[screenMenu add_menu_item_by_id:CMsg::Afterimage2:recv:@selector(ChangeAfterImage:):0:2:'t'];
#endif

#ifdef USE_KEEPIMAGE
	[screenMenu addItem:[NSMenuItem separatorItem]];
	[screenMenu add_menu_item_by_id:CMsg::Keepimage1:recv:@selector(ChangeKeepImage:):0:1:0];
	[screenMenu add_menu_item_by_id:CMsg::Keepimage2:recv:@selector(ChangeKeepImage:):0:2:0];
#endif
#ifdef _MBS1
	[screenMenu addItem:[NSMenuItem separatorItem]];
	[screenMenu add_menu_item_by_id:CMsg::Digital_RGB:recv:@selector(ChangeRGBType:):0:0:0];
	[screenMenu add_menu_item_by_id:CMsg::Analog_RGB:recv:@selector(ChangeRGBType:):0:1:0];
#endif

#ifdef USE_OPENGL
	[screenMenu addItem:[NSMenuItem separatorItem]];
	[screenMenu add_menu_item_by_id:CMsg::Use_OpenGL_Sync:recv:@selector(ChangeUseOpenGL:):0:1:'y'];
	[screenMenu add_menu_item_by_id:CMsg::Use_OpenGL_Async:recv:@selector(ChangeUseOpenGL:):0:2:'y'];
	CocoaMenu *glFilterMenu = [CocoaMenu create_menu_by_id:CMsg::OpenGL_Filter];
		[glFilterMenu add_menu_item_by_id:CMsg::Nearest_Neighbour:recv:@selector(ChangeOpenGLFilter:):0:0:'u'];
		[glFilterMenu add_menu_item_by_id:CMsg::Linear:recv:@selector(ChangeOpenGLFilter:):0:1:'u'];
	[screenMenu add_sub_menu_by_id:glFilterMenu:CMsg::OpenGL_Filter];
#endif

	/* Put menu into the menubar */
	add_main_menu_by_id(screenMenu,CMsg::Screen);

	// Sound menu

	CocoaMenu *soundMenu = [CocoaMenu create_menu_by_id:CMsg::Sound];

	[soundMenu add_menu_item_by_id:CMsg::Volume_:recv:@selector(ShowVolumeDialog:):0:0:'v'];
#ifdef USE_REC_AUDIO
	[soundMenu addItem:[NSMenuItem separatorItem]];
	CocoaMenu *recSoundMenu = [CocoaMenu create_menu_by_id:CMsg::Record_Sound];
	[recSoundMenu add_menu_item_by_id:CMsg::Start_:recv:@selector(ShowRecordAudioDialog:):0:0:0];
	[recSoundMenu add_menu_item_by_id:CMsg::Stop:recv:@selector(StopRecordSound:):0:0:0];
	[soundMenu add_sub_menu_by_id:recSoundMenu:CMsg::Record_Sound];
#endif
	[soundMenu addItem:[NSMenuItem separatorItem]];
	[soundMenu add_menu_item_by_id:CMsg::F8000Hz:recv:@selector(ChangeSoundFrequency:):0:2:0];
	[soundMenu add_menu_item_by_id:CMsg::F11025Hz:recv:@selector(ChangeSoundFrequency:):0:3:0];
	[soundMenu add_menu_item_by_id:CMsg::F22050Hz:recv:@selector(ChangeSoundFrequency:):0:4:0];
	[soundMenu add_menu_item_by_id:CMsg::F44100Hz:recv:@selector(ChangeSoundFrequency:):0:5:0];
	[soundMenu add_menu_item_by_id:CMsg::F48000Hz:recv:@selector(ChangeSoundFrequency:):0:6:0];
	[soundMenu add_menu_item_by_id:CMsg::F96000Hz:recv:@selector(ChangeSoundFrequency:):0:7:0];
	[soundMenu addItem:[NSMenuItem separatorItem]];
	[soundMenu add_menu_item_by_id:CMsg::S50msec:recv:@selector(ChangeSoundLatency:):0:0:0];
	[soundMenu add_menu_item_by_id:CMsg::S75msec:recv:@selector(ChangeSoundLatency:):0:1:0];
	[soundMenu add_menu_item_by_id:CMsg::S100msec:recv:@selector(ChangeSoundLatency:):0:2:0];
	[soundMenu add_menu_item_by_id:CMsg::S200msec:recv:@selector(ChangeSoundLatency:):0:3:0];
	[soundMenu add_menu_item_by_id:CMsg::S300msec:recv:@selector(ChangeSoundLatency:):0:4:0];
	[soundMenu add_menu_item_by_id:CMsg::S400msec:recv:@selector(ChangeSoundLatency:):0:5:0];

	/* Put menu into the menubar */
	add_main_menu_by_id(soundMenu,CMsg::Sound);

	// Devices menu

	CocoaMenu *deviceMenu = [CocoaMenu create_menu_by_id:CMsg::Devices];

	for(drv = 0; drv < MAX_PRINTER; drv++) {
		UTILITY::sprintf(name, sizeof(name), CMSG(LPTVDIGIT), drv);
		CocoaMenu *lptMenu = [CocoaMenu create_menu:name];
		[lptMenu add_menu_item_by_id:CMsg::Save_:recv:@selector(ShowSavePrinterDialog:):drv:0:0];
		[lptMenu add_menu_item_by_id:CMsg::Print_to_mpprinter:recv:@selector(PrintPrinter:):drv:0:0];
		[lptMenu add_menu_item_by_id:CMsg::Clear:recv:@selector(ClearPrinterBuffer:):drv:0:0];
		[lptMenu addItem:[NSMenuItem separatorItem]];
		[lptMenu add_menu_item_by_id:CMsg::Online:recv:@selector(TogglePrinterOnline:):drv:0:0];
		[lptMenu addItem:[NSMenuItem separatorItem]];
		[lptMenu add_menu_item_by_id:CMsg::Send_to_mpprinter_concurrently:recv:@selector(EnablePrinterDirect:):drv:0:0];
		[deviceMenu add_sub_menu:lptMenu:name];
	}
	[deviceMenu addItem:[NSMenuItem separatorItem]];
	for(drv = 0; drv < MAX_COMM; drv++) {
		UTILITY::sprintf(name, sizeof(name), CMSG(COMVDIGIT), drv);
		CocoaMenu *comMenu = [CocoaMenu create_menu:name];
		[comMenu add_menu_item_by_id:CMsg::Enable_Server:recv:@selector(EnableCommServer:):drv:0:0];
		CocoaMenu *comConnectMenu = [CocoaMenu create_menu_by_id:CMsg::Connect];
		[comMenu add_sub_menu_by_id:comConnectMenu:CMsg::Connect:recv:@selector(UpdateCommConnectList:):drv:0];
		[comMenu addItem:[NSMenuItem separatorItem]];
		[comMenu add_menu_item_by_id:CMsg::Comm_With_Byte_Data:recv:@selector(CommThroughMode:):drv:0:0];
		[comMenu addItem:[NSMenuItem separatorItem]];
		CocoaMenu *telnetMenu = [CocoaMenu create_menu_by_id:CMsg::Options_For_Telnet];
			[telnetMenu add_menu_item_by_id:CMsg::Binary_Mode:recv:@selector(CommBinaryMode:):drv:0:0];
			[telnetMenu addItem:[NSMenuItem separatorItem]];
			[telnetMenu add_menu_item_by_id:CMsg::Send_WILL_ECHO:recv:@selector(SendCommTelnetCommand:):drv:1:0];
		[comMenu add_sub_menu_by_id:telnetMenu:CMsg::Options_For_Telnet];
		[deviceMenu add_sub_menu:comMenu:name];
	}
	/* Put menu into the menubar */
	add_main_menu_by_id(deviceMenu,CMsg::Devices);

	// Options menu

	CocoaMenu *optionMenu = [CocoaMenu create_menu_by_id:CMsg::Options];

	[optionMenu add_menu_item_by_id:CMsg::Show_LED:recv:@selector(ToggleLedBox:):0:1:'l'];
	[optionMenu add_menu_item_by_id:CMsg::Inside_LED:recv:@selector(ToggleLedBox:):0:2:'l'];
	[optionMenu add_menu_item_by_id:CMsg::Show_Message:recv:@selector(ToggleMessageBoard:):0:0:'z'];
	[optionMenu add_menu_item_by_id:CMsg::Log_:recv:@selector(ShowLoggingDialog:):0:0:0];
#ifdef USE_PERFORMANCE_METER
	[optionMenu add_menu_item_by_id:CMsg::Show_Performance_Meter:recv:@selector(TogglePMeter:):0:0:0];
#endif
#ifdef USE_LIGHTPEN
	[optionMenu addItem:[NSMenuItem separatorItem]];
	[optionMenu add_menu_item_by_id:CMsg::Enable_Lightpen:recv:@selector(ToggleEnableLightpen:):0:0:0];
#endif
#ifdef USE_MOUSE
	[optionMenu addItem:[NSMenuItem separatorItem]];
	[optionMenu add_menu_item_by_id:CMsg::Enable_Mouse:recv:@selector(ToggleUseMouse:):0:0:0];
#endif
#if defined(USE_JOYSTICK) || defined(USE_KEY2JOYSTICK)
	[optionMenu addItem:[NSMenuItem separatorItem]];
#endif
#ifdef USE_JOYSTICK
	[optionMenu add_menu_item_by_id:CMsg::Use_Joypad_Key_Assigned:recv:@selector(ChangeUseJoypad:):0:1:'j'];
#ifdef USE_PIAJOYSTICK
	[optionMenu add_menu_item_by_id:CMsg::Use_Joypad_PIA_Type:recv:@selector(ChangeUseJoypad:):0:2:'j'];
#endif
#endif
#ifdef USE_KEY2JOYSTICK
	[optionMenu add_menu_item_by_id:CMsg::Enable_Key_to_Joypad:recv:@selector(ToggleEnableKey2Joypad:):0:0:0];
#endif
	[optionMenu addItem:[NSMenuItem separatorItem]];
	[optionMenu add_menu_item_by_id:CMsg::Loosen_Key_Stroke_Game:recv:@selector(ToggleLoosenKeyStroke:):0:0:0];
	[optionMenu add_menu_item_by_id:CMsg::Virtual_Keyboard_:recv:@selector(ShowVirtualKeyboard:):0:0:0];
#ifdef USE_DEBUGGER
	[optionMenu addItem:[NSMenuItem separatorItem]];
	[optionMenu add_menu_item_by_id:CMsg::Start_Debugger:recv:@selector(OpenDebugger:):0:0:'d'];
	[optionMenu add_menu_item_by_id:CMsg::Stop_Debugger:recv:@selector(CloseDebugger:):0:0:0];
#endif
	[optionMenu addItem:[NSMenuItem separatorItem]];
#if defined(USE_PIAJOYSTICK) || defined(USE_KEY2JOYSTICK)
	[optionMenu add_menu_item_by_id:CMsg::Joypad_Setting_:recv:@selector(ShowJoySettingDialog:):0:0:0];
#endif
	[optionMenu add_menu_item_by_id:CMsg::Keybind_:recv:@selector(ShowKeybindDialog:):0:0:'k'];
	[optionMenu add_menu_item_by_id:CMsg::Configure_:recv:@selector(ShowConfigureDialog:):0:0:'c'];

	/* Put menu into the menubar */
	add_main_menu_by_id(optionMenu,CMsg::Options);

	[[NSApp mainMenu] setDelegate:[CocoaMenuDelegate create]];

	/* create popup menu */
//	popupMenu = [CocoaMenu create_menu_by_id:CMsg::AAA)];
//	[popupMenu add_menu_item_by_id:CMsg::BBB):nil:nil:0:0:0];
}

bool GUI::NeedUpdateScreen()
{
//	logging->out_logf(LOG_DEBUG, "GUI::NeedUpdateScreen: %d", need_update_screen);
	if (need_update_screen > 0) {
		need_update_screen = 0;
		// post message to main thread
		[recv performSelectorOnMainThread:@selector(PerformUpdateScreen) withObject:nil waitUntilDone:FALSE];
		// send signal to main loop
		g_need_update_screen = true;
		SDL_CondSignal(g_cond_allow_update_screen);
		return true;
	} else {
		return false;
	}
}
void GUI::DecreaseUpdateScreenCount(void)
{
	// nothing to do
}

#ifdef USE_DATAREC
bool GUI::ShowLoadDataRecDialog(void)
{
	NSInteger	result;
	NSOpenPanel *panel;

	PostEtSystemPause(true);
	GoWindowMode();

	panel = [NSOpenPanel openPanel];
	// title
	[panel setTitle:[NSString stringWithUTF8String:CMSG(Play_Data_Recorder_Tape)]];
	// filtering file types
	NSArray *fileTypes = get_file_filter(LABELS::datarec_exts);
	[panel setAllowedFileTypes:fileTypes];
	[panel setAllowsOtherFileTypes:YES];
	// set current folder
	[panel setDirectoryURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:pConfig->GetInitialDataRecPath()]]];

	// Display modal dialog
	result = [panel runModal];

	if(result == NSModalResponseOK) {
		// get file path (use NSURL)
		NSURL *filePath = [panel URL];

		PostEtLoadDataRecMessage([[filePath path] UTF8String]);
	} else {
		PostEtSystemPause(false);
	}
	SetFocusToMainWindow();
	return (result == NSModalResponseOK);
}

bool GUI::ShowSaveDataRecDialog(void)
{
	NSInteger	result;
	CocoaSaveDatarec *panel;

	PostEtSystemPause(true);
	GoWindowMode();

	panel = [[CocoaSaveDatarec alloc] init];

	// Display modal dialog
	result = [panel runModal];

	if(result == NSModalResponseOK) {
		// get file path (use NSURL)
		NSURL *filePath = [panel URL];

		PostEtSaveDataRecMessage([[filePath path] UTF8String]);
	} else {
		PostEtSystemPause(false);
	}
	SetFocusToMainWindow();
	return (result == NSModalResponseOK);
}
#endif

#ifdef USE_FD1
bool GUI::ShowOpenFloppyDiskDialog(int drv)
{
	NSInteger	result;
	NSOpenPanel *panel;

	PostEtSystemPause(true);
	GoWindowMode();

	panel = [NSOpenPanel openPanel];
	char title[128];
	UTILITY::sprintf(title,sizeof(title),CMSG(Open_Floppy_Disk_VDIGIT),drv);
	// title
	[panel setTitle:[NSString stringWithUTF8String:title]];
	// filtering file types
	NSArray *fileTypes = get_file_filter(LABELS::floppy_disk_exts);
	[panel setAllowedFileTypes:fileTypes];
	[panel setAllowsOtherFileTypes:YES];
	// set current folder
	[panel setDirectoryURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:pConfig->GetInitialFloppyDiskPath()]]];

	// Display modal dialog
	result = [panel runModal];

	if(result == NSModalResponseOK) {
		// get file path (use NSURL)
		NSURL *filePath = [panel URL];

		PostEtOpenFloppyMessage(drv, [[filePath path] UTF8String], 0, 0, true);
	} else {
		PostEtSystemPause(false);
	}
	SetFocusToMainWindow();
	return (result == NSModalResponseOK);
}

int GUI::ShowSelectFloppyDriveDialog(int drv)
{
	CocoaSelDrvPanel *panel;

//	PostEtSystemPause(true);
	GoWindowMode();

	panel = [[CocoaSelDrvPanel alloc] initWithPrefix:drv prefix:CMSG(FDD)];

	// Display modal dialog
	[panel runModal];
	int new_drv = [panel selectedDrive];

	[panel release];

	PostEtSystemPause(false);
	SetFocusToMainWindow();
	return new_drv;
}

bool GUI::ShowOpenBlankFloppyDiskDialog(int drv, uint8_t type)
{
	NSInteger	result;
	NSSavePanel *panel;

	PostEtSystemPause(true);
	GoWindowMode();

	panel = [NSSavePanel savePanel];
	char title[128];
	UTILITY::sprintf(title,sizeof(title),CMSG(New_Floppy_Disk_VDIGIT),drv);
	// title
	[panel setTitle:[NSString stringWithUTF8String:title]];
	// filtering file types
	NSArray *fileTypes = get_file_filter(LABELS::blank_floppy_disk_exts);
	[panel setAllowedFileTypes:fileTypes];
//	[panel setAllowsOtherFileTypes:YES];
	[panel setExtensionHidden:NO];
	// set current folder
	[panel setDirectoryURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:pConfig->GetInitialFloppyDiskPath()]]];
	// set default file name
	char file_name[128];
	UTILITY::create_date_file_path(NULL, file_name, 128, _T("d88"));
	[panel setNameFieldStringValue:[NSString stringWithUTF8String:file_name]];

	// Display modal dialog
	result = [panel runModal];

	// get file path (use NSURL)
	NSURL *filePath = [panel URL];

	bool rc = (result == NSModalResponseOK);
	if(rc) {
		rc = emu->create_blank_floppy_disk([[filePath path] UTF8String], type);
	}
	if(rc) {
		PostEtOpenFloppyMessage(drv, [[filePath path] UTF8String], 0, 0, true);
	} else {
		PostEtSystemPause(false);
	}
	SetFocusToMainWindow();
	return rc;
}
#endif

#ifdef USE_HD1
bool GUI::ShowOpenHardDiskDialog(int drv)
{
	NSInteger	result;
	NSOpenPanel *panel;

	PostEtSystemPause(true);
	GoWindowMode();

	panel = [NSOpenPanel openPanel];
	char title[128];
	UTILITY::sprintf(title,sizeof(title),CMSG(Open_Hard_Disk_VDIGIT),drv);
	// title
	[panel setTitle:[NSString stringWithUTF8String:title]];
	// filtering file types
	NSArray *fileTypes = get_file_filter(LABELS::hard_disk_exts);
	[panel setAllowedFileTypes:fileTypes];
	[panel setAllowsOtherFileTypes:YES];
	// set current folder
	[panel setDirectoryURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:pConfig->GetInitialHardDiskPath()]]];

	// Display modal dialog
	result = [panel runModal];

	if(result == NSModalResponseOK) {
		// get file path (use NSURL)
		NSURL *filePath = [panel URL];

		PostEtOpenHardDiskMessage(drv, [[filePath path] UTF8String], 0);
	} else {
		PostEtSystemPause(false);
	}
	SetFocusToMainWindow();
	return (result == NSModalResponseOK);
}

bool GUI::ShowOpenBlankHardDiskDialog(int drv, uint8_t type)
{
	NSInteger	result;
	NSSavePanel *panel;

	PostEtSystemPause(true);
	GoWindowMode();

	panel = [NSSavePanel savePanel];
	char title[128];
	UTILITY::sprintf(title,sizeof(title),CMSG(New_Hard_Disk_VDIGIT),drv);
	// title
	[panel setTitle:[NSString stringWithUTF8String:title]];
	// filtering file types
	NSArray *fileTypes = get_file_filter(LABELS::blank_hard_disk_exts);
	[panel setAllowedFileTypes:fileTypes];
//	[panel setAllowsOtherFileTypes:YES];
	[panel setExtensionHidden:NO];
	// set current folder
	[panel setDirectoryURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:pConfig->GetInitialHardDiskPath()]]];
	// set default file name
	char file_name[128];
	UTILITY::create_date_file_path(NULL, file_name, 128, _T("hdf"));
	[panel setNameFieldStringValue:[NSString stringWithUTF8String:file_name]];

	// Display modal dialog
	result = [panel runModal];

	// get file path (use NSURL)
	NSURL *filePath = [panel URL];

	bool rc = (result == NSModalResponseOK);
	if(rc) {
		rc = emu->create_blank_hard_disk([[filePath path] UTF8String], type);
	}
	if(rc) {
		PostEtOpenHardDiskMessage(drv, [[filePath path] UTF8String], 0);
	} else {
		PostEtSystemPause(false);
	}
	SetFocusToMainWindow();
	return rc;
}
#endif	// USE_HD1

bool GUI::ShowLoadStateDialog(void)
{
	NSInteger	result;
	NSOpenPanel *panel;

	PostEtSystemPause(true);
	GoWindowMode();

	panel = [NSOpenPanel openPanel];
	// title
	[panel setTitle:[NSString stringWithUTF8String:CMSG(Load_Status_Data)]];
	// filtering file types
	NSArray *fileTypes = get_file_filter(LABELS::state_file_exts);
	[panel setAllowedFileTypes:fileTypes];

	// set current folder
	[panel setDirectoryURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:pConfig->GetInitialStatePath()]]];

	// Display modal dialog
	result = [panel runModal];

	if(result == NSModalResponseOK) {
		// get file path (use NSURL)
		NSURL *filePath = [panel URL];

		PostEtLoadStatusMessage([[filePath path] UTF8String]);
	} else {
		PostEtSystemPause(false);
	}
	SetFocusToMainWindow();
	return (result == NSModalResponseOK);
}

bool GUI::ShowSaveStateDialog(bool cont)
{
	NSInteger	result;
	NSSavePanel *panel;

	PostEtSystemPause(true);
	GoWindowMode();

	panel = [NSSavePanel savePanel];
	// title
	[panel setTitle:[NSString stringWithUTF8String:CMSG(Save_Status_Data)]];
	// filtering file types
	NSArray *fileTypes = get_file_filter(LABELS::state_file_exts);
	[panel setAllowedFileTypes:fileTypes];
	[panel setExtensionHidden:NO];

	// set current folder
	[panel setDirectoryURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:pConfig->GetInitialStatePath()]]];

	// Display modal dialog
	result = [panel runModal];

	if(result == NSModalResponseOK) {
		// get file path (use NSURL)
		NSURL *filePath = [panel URL];

		PostEtSaveStatusMessage([[filePath path] UTF8String], cont);
	} else {
		PostEtSystemPause(false);
	}
	SetFocusToMainWindow();
	return (result == NSModalResponseOK);
}

bool GUI::ShowOpenAutoKeyDialog(void)
{
	NSInteger	result;
	NSOpenPanel *panel;

	PostEtSystemPause(true);
	GoWindowMode();

	panel = [NSOpenPanel openPanel];
	// title
	[panel setTitle:[NSString stringWithUTF8String:CMSG(Open_Text_File)]];
	// filtering file types
	NSArray *fileTypes = get_file_filter(LABELS::autokey_file_exts);
	[panel setAllowedFileTypes:fileTypes];

	// set current folder
	[panel setDirectoryURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:pConfig->GetInitialAutoKeyPath()]]];

	// Display modal dialog
	result = [panel runModal];

	if(result == NSModalResponseOK) {
		// get file path (use NSURL)
		NSURL *filePath = [panel URL];

		PostEtLoadAutoKeyMessage([[filePath path] UTF8String]);
	} else {
		PostEtSystemPause(false);
	}
	SetFocusToMainWindow();
	return (result == NSModalResponseOK);
}

bool GUI::ShowPlayRecKeyDialog(void)
{
	NSInteger	result;
	NSOpenPanel *panel;

	PostEtSystemPause(true);
	GoWindowMode();

	panel = [NSOpenPanel openPanel];
	// title
	[panel setTitle:[NSString stringWithUTF8String:CMSG(Play_Recorded_Keys)]];
	// filtering file types
	NSArray *fileTypes = get_file_filter(LABELS::key_rec_file_exts);
	[panel setAllowedFileTypes:fileTypes];
	// set current folder
	[panel setDirectoryURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:pConfig->GetInitialStatePath()]]];

	// Display modal dialog
	result = [panel runModal];

	if(result == NSModalResponseOK) {
		// get file path (use NSURL)
		NSURL *filePath = [panel URL];

		PostEtLoadRecKeyMessage([[filePath path] UTF8String]);
	} else {
		PostEtSystemPause(false);
	}
	SetFocusToMainWindow();
	return (result == NSModalResponseOK);
}

bool GUI::ShowRecordRecKeyDialog(void)
{
	NSInteger	result;
	NSSavePanel *panel;

	PostEtSystemPause(true);
	GoWindowMode();

	panel = [NSSavePanel savePanel];
	// title
	[panel setTitle:[NSString stringWithUTF8String:CMSG(Record_Input_Keys)]];
	// filtering file types
	NSArray *fileTypes = get_file_filter(LABELS::key_rec_file_exts);
	[panel setAllowedFileTypes:fileTypes];
	[panel setExtensionHidden:NO];

	// set current folder
	[panel setDirectoryURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:pConfig->GetInitialStatePath()]]];

	// Display modal dialog
	result = [panel runModal];

	if(result == NSModalResponseOK) {
		// get file path (use NSURL)
		NSURL *filePath = [panel URL];

		PostEtSaveRecKeyMessage([[filePath path] UTF8String], false);
	} else {
		PostEtSystemPause(false);
	}
	SetFocusToMainWindow();
	return (result == NSModalResponseOK);
}

bool GUI::ShowSavePrinterDialog(int drv)
{
	NSInteger	result;
	NSSavePanel *panel;

	PostEtSystemPause(true);
	GoWindowMode();

	panel = [NSSavePanel savePanel];
	// title
	[panel setTitle:[NSString stringWithUTF8String:CMSG(Save_Printing_Data)]];
	// filtering file types
	NSArray *fileTypes = get_file_filter(LABELS::printing_file_exts);
	[panel setAllowedFileTypes:fileTypes];
	[panel setAllowsOtherFileTypes:YES];
	[panel setExtensionHidden:NO];

	// set current folder
	[panel setDirectoryURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:pConfig->GetInitialPrinterPath()]]];

	// Display modal dialog
	result = [panel runModal];

	if(result == NSModalResponseOK) {
		// get file path (use NSURL)
		NSURL *filePath = [panel URL];

		PostEtSavePrinterMessage(drv, [[filePath path] UTF8String]);
	} else {
		PostEtSystemPause(false);
	}
	SetFocusToMainWindow();
	return (result == NSModalResponseOK);
}

bool GUI::ShowRecordVideoDialog(int fps_num)
{
	NSInteger result;
	CocoaRecVideoPanel *panel;

	PostEtSystemPause(true);
	GoWindowMode();

	panel = [[CocoaRecVideoPanel alloc] initWithCont:false];

	// Display modal dialog
	result = [panel runModal];

	[panel release];

	if (result == NSModalResponseOK) {
		PostEtStartRecordVideo(fps_num);
	} else {
		PostEtSystemPause(false);
	}
	SetFocusToMainWindow();
	return (result == NSModalResponseOK);
}

bool GUI::ShowRecordAudioDialog(void)
{
	NSInteger result;
	CocoaRecAudioPanel *panel;

	PostEtSystemPause(true);
	GoWindowMode();

	panel = [[CocoaRecAudioPanel alloc] init];

	// Display modal dialog
	result = [panel runModal];

	[panel release];

	if (result == NSModalResponseOK) {
		PostEtStartRecordSound();
	} else {
		PostEtSystemPause(false);
	}
	SetFocusToMainWindow();
	return (result == NSModalResponseOK);
}

bool GUI::ShowRecordVideoAndAudioDialog(int fps_num)
{
	NSInteger result;

	PostEtSystemPause(true);
	GoWindowMode();

	CocoaRecVideoPanel *panelv;
	panelv = [[CocoaRecVideoPanel alloc] initWithCont:true];

	// Display modal dialog
	result = [panelv runModal];

	[panelv release];

	if (result != NSModalResponseOK) {
		PostEtSystemPause(false);
		return false;
	}

	CocoaRecAudioPanel *panela;
	panela = [[CocoaRecAudioPanel alloc] init];

	// Display modal dialog
	result = [panela runModal];

	[panela release];

	if (result == NSModalResponseOK) {
		// start video and audio
		PostEtStartRecordVideo(fps_num);
	} else {
		PostEtSystemPause(false);
	}
	SetFocusToMainWindow();
	return (result == NSModalResponseOK);
}

bool GUI::ShowVolumeDialog(void)
{
	CocoaVolumePanel *panel;

	GoWindowMode();

	panel = [[CocoaVolumePanel alloc] init];
	[panel runModal];
	//	[panel orderFront:self];
	[panel release];
	SetFocusToMainWindow();

	return true;
}

bool GUI::ShowJoySettingDialog(void)
{
	NSInteger	result;
	CocoaJoySettingPanel *panel;
	
	PostEtSystemPause(true);
	GoWindowMode();
	
	panel = [[CocoaJoySettingPanel alloc] init];
	result = [panel runModal];
	
	[panel release];
	PostEtSystemPause(false);
	SetFocusToMainWindow();
	
	return (result == NSModalResponseOK);
}

bool GUI::ShowKeybindDialog(void)
{
	NSInteger	result;
	CocoaKeybindPanel *panel;

	PostEtSystemPause(true);
	GoWindowMode();

	panel = [[CocoaKeybindPanel alloc] init];

	// Display modal dialog
	result = [panel runModal];

	[panel release];
	PostEtSystemPause(false);
	SetFocusToMainWindow();

	return (result == NSModalResponseOK);
}

bool GUI::ShowConfigureDialog(void)
{
	NSInteger	result;
	CocoaConfigPanel *panel;

	PostEtSystemPause(true);
	GoWindowMode();

	panel = [[CocoaConfigPanel alloc] init];

	// Display modal dialog
	result = [panel runModal];

	[panel release];
	PostEtSystemPause(false);
	SetFocusToMainWindow();

	return (result == NSModalResponseOK);
}

bool GUI::ShowLoggingDialog(void)
{
	if (!logging_dlg) {
		logging_dlg = [[CocoaLoggingPanel alloc] init];
	}
	if (!IsShownLoggingDialog()) {
		[logging_dlg run];
	} else {
		[logging_dlg close];
	}
	return true;
}

bool GUI::IsShownLoggingDialog(void)
{
	return logging_dlg ? [logging_dlg isVisible] == TRUE : false;
}

void GUI::GoWindowMode(void)
{
	if (IsFullScreen()) {
		ChangeWindowMode(-1);
	}
}

#ifdef USE_MOUSE
/// Change Mouse Cursor
void GUI::ToggleUseMouse(void)
{
	GUI_BASE::ToggleUseMouse();
	if (IsEnableMouse()) {
		CGDisplayHideCursor(kCGDirectMainDisplay);
	} else {
		CGDisplayShowCursor(kCGDirectMainDisplay);
	}
}
#endif

#endif /* GUI_TYPE_COCOA */

void remove_window_menu(void)
{
    NSMenu *mainMenu;
    NSInteger i;

    mainMenu = [NSApp mainMenu];

    i = [mainMenu indexOfItemWithTitle:@"Window"];
    if (i >= 0) [mainMenu removeItemAtIndex:i];
    i = [mainMenu indexOfItemWithTitle:@"View"];
    if (i >= 0) [mainMenu removeItemAtIndex:i];
}

#ifdef _MBS1
#define APPLE_MENU_STRING _TX("About mbs1"),_TX("Hide mbs1"),_TX("Hide Others"),_TX("Show All"),_TX("Quit mbs1"),_TX("Services"),_TX("Preferences…"),_TX("Window")
#else
#define APPLE_MENU_STRING _TX("About bml3mk5"),_TX("Hide bml3mk5"),_TX("Hide Others"),_TX("Show All"),_TX("Quit bml3mk5"),_TX("Services"),_TX("Preferences…"),_TX("Window")
#endif

void translate_apple_menu(void)
{
	NSMenu *appleMenu;
	NSArray *items;
    NSInteger i;
    NSInteger count;

    appleMenu = [[[NSApp mainMenu] itemAtIndex:0] submenu];
	items = [appleMenu itemArray];
	count = [items count];
	for(i=0; i<count; i++) {
		NSMenuItem *item = [items objectAtIndex:(NSUInteger)i];
		if ([item isSeparatorItem]) continue;
		if ([[item keyEquivalent] isEqual: @"q"]) {
			// replace shortcutkey: command + q -> option + F4
			// because use command key in virtual machine
//			const unichar c[2] = { NSF4FunctionKey, 0 };
//			[item setKeyEquivalent:[NSString stringWithCharacters:c length:1]];
//			[item setKeyEquivalentModifierMask:NSAlternateKeyMask];
		} else {
			[item setKeyEquivalent:@""];
		}
		[item setTitle:[NSString stringWithUTF8String:_tgettext([[item title] UTF8String])]];
	}
}

@implementation CocoaWindowDelegate
@synthesize parentObject;
@synthesize guiObject;
+ (id)allocWithParent:(id)new_obj gui:(GUI_BASE *)new_gui
{
	id me = [[CocoaWindowDelegate alloc] init];
	[me setParentObject:new_obj];
	[me setGuiObject:new_gui];
	return me;
}
- (BOOL)windowShouldClose:(id)sender
{
	return parentObject != nil ? [parentObject windowShouldClose:sender] : NO;
}
- (void)windowDidBecomeKey:(NSNotification *)aNotification
{
	if (parentObject != nil) [parentObject windowDidBecomeKey:aNotification];
}
- (void)windowDidResignKey:(NSNotification *)aNotification
{
	if (parentObject != nil) [parentObject windowDidResignKey:aNotification];
}
-(NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    return NSDragOperationCopy;
}
-(NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender
{
    return NSDragOperationCopy;
}
- (void)performDragOperation:(id<NSDraggingInfo>)sender
{
	NSPasteboard *pb = [sender draggingPasteboard];
	NSArray *files = [pb propertyListForType:NSFilenamesPboardType];
	for(int i=0; i<[files count]; i++) {
		NSString *file = (NSString *)[files objectAtIndex:i];
		guiObject->OpenFileByExtention([file UTF8String]);
	}
}
- (void)windowDidMiniaturize:(NSNotification *)aNotification
{
#ifdef USE_SDL2
	if (parentObject != nil) [parentObject windowDidMiniaturize:aNotification];
#endif
	// another window also miniaturize
	NSWindow *me = [aNotification object];
	NSArray *windows;
	windows = [NSApp windows];
	for(int i=0; i<[windows count]; i++) {
		NSWindow *window = (NSWindow *)[windows objectAtIndex:i];
		if (window != me && [window isVisible]) [window miniaturize:self];
	}
}
- (void)windowDidDeminiaturize:(NSNotification *)aNotification
{
#ifdef USE_SDL2
	if (parentObject != nil) [parentObject windowDidDeminiaturize:aNotification];
#endif
	// another window also de-miniaturize
	NSWindow *me = [aNotification object];
	NSArray *windows;
	windows = [NSApp windows];
	for(int i=0; i<[windows count]; i++) {
		NSWindow *window = (NSWindow *)[windows objectAtIndex:i];
		if (window != me && [window isMiniaturized]) [window deminiaturize:self];
	}
}
- (void)windowDidMove:(NSNotification *)aNotification
{
#ifdef USE_SDL2
	if (parentObject != nil) [parentObject windowDidMove:aNotification];
#else
	NSWindow *me = [aNotification object];
	NSArray *windows;
	windows = [NSApp windows];
	for(int i=0; i<[windows count]; i++) {
		NSWindow *window = (NSWindow *)[windows objectAtIndex:i];
		if (window != me && [[window className] isEqualToString:@"CocoaLedBox"]) [window performSelector:@selector(move)];
	}
#endif
}
- (void)windowDidBecomeMain:(NSNotification *)aNotification
{
	NSWindow *me = [aNotification object];
	NSArray *windows;
	windows = [NSApp windows];
	for(int i=0; i<[windows count]; i++) {
		NSWindow *window = (NSWindow *)[windows objectAtIndex:i];
		if (window != me && [window isVisible]) {
			[window setLevel:NSFloatingWindowLevel];
		}
	}
}
- (void)windowDidResignMain:(NSNotification *)aNotification
{
	NSWindow *me = [aNotification object];
	NSArray *windows;
	windows = [NSApp windows];
	for(int i=0; i<[windows count]; i++) {
		NSWindow *window = (NSWindow *)[windows objectAtIndex:i];
		if (window != me && [window isVisible]) {
			[window setLevel:NSNormalWindowLevel];
			[window orderWindow:NSWindowAbove relativeTo:[me windowNumber]];
		}
	}
}
#ifdef USE_SDL2
-(void) windowDidExpose:(NSNotification *) aNotification
{
	if (parentObject != nil) [parentObject windowDidExpose:aNotification];
}
-(void) windowDidResize:(NSNotification *) aNotification
{
	if (parentObject != nil) [parentObject windowDidResize:aNotification];
}
-(void) windowWillEnterFullScreen:(NSNotification *) aNotification
{
	if (parentObject != nil) [parentObject windowWillEnterFullScreen:aNotification];
}
-(void) windowDidEnterFullScreen:(NSNotification *) aNotification
{
	if (parentObject != nil) [parentObject windowDidEnterFullScreen:aNotification];
}
-(void) windowWillExitFullScreen:(NSNotification *) aNotification
{
	if (parentObject != nil) [parentObject windowWillExitFullScreen:aNotification];
}
-(void) windowDidExitFullScreen:(NSNotification *) aNotification
{
	if (parentObject != nil) [parentObject windowDidExitFullScreen:aNotification];
}
-(NSApplicationPresentationOptions)window:(NSWindow *)window willUseFullScreenPresentationOptions:(NSApplicationPresentationOptions)proposedOptions
{
	return parentObject != nil ? [parentObject window:window willUseFullScreenPresentationOptions:proposedOptions] : 0;
}
#endif
@end

void set_delegate_to_sdl_window(GUI_BASE *new_gui)
{
	NSWindow *sdl_window = NULL;
	NSArray *windows = [NSApp windows];
	for(int i=0; i<[windows count]; i++) {
		NSWindow *window = (NSWindow *)[windows objectAtIndex:i];
		NSString *name = [window className];
logging->out_debugf("COCOA window %d: %s %llx", i, [name UTF8String], window);
		NSRange ra = [name rangeOfString:@"SDL"];
		if (ra.length > 0) {
			NSObject *delegate = [window delegate];
logging->out_debugf("COCOA delegete: %s", [[delegate className] UTF8String]);
			// set delegate wrapper (if fullscreen, delegate is nil)
			if (delegate && ![[delegate className] isEqualToString:@"CocoaWindowDelegate"]) {
				[window setDelegate:[CocoaWindowDelegate allocWithParent:delegate gui:new_gui]];
				[window registerForDraggedTypes:[NSArray arrayWithObject:NSFilenamesPboardType]];
logging->out_debugf("COCOA delegete wrapped");
			}
			sdl_window = window;
		}
	}
#if !(defined(USE_SDL2) && defined(USE_SDL2_LEDBOX))
	// send window object to ledbox
	ledbox_set_owner_window(sdl_window);
#endif
}

#ifdef GUI_TYPE_AGAR
#include "../agar/ag_gui.h"
#endif
bool GUI::StartAutoKey(void)
{
	bool rc = false;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSPasteboard *pb = [NSPasteboard generalPasteboard];
	NSString *type = [pb availableTypeFromArray:[NSArray arrayWithObject:NSStringPboardType]];
	if (type != nil) {
		NSString *str = [pb stringForType:NSStringPboardType];
		rc = emu->start_auto_key([str UTF8String]);
	}
	[pool release];
	return rc;
}

bool GUI::ShowVirtualKeyboard(void)
{
#ifdef USE_VKEYBOARD
	if (!vkeyboard) {
		vkeyboard = new Vkbd::VKeyboard();

		uint8_t *buf;
		int siz;
		emu->get_vm_key_status_buffer(&buf, &siz);
		FIFOINT *his = emu->get_vm_key_history();
		vkeyboard->SetStatusBufferPtr(buf, siz, VM_KEY_STATUS_VKEYBOARD);
		vkeyboard->SetHistoryBufferPtr(his);
		if (!vkeyboard->Create(emu->resource_path())) {
			logging->out_log(LOG_ERROR, _T("Cannot open virtual keyboard window."));
		}
		vkeyboard->Show();
	} else {
		vkeyboard->Close();
	}
	return true;
#else
	return false;
#endif
}
