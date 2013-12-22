#include "dxwnd.h"
#include "dxwcore.hpp"
#include "syslibs.h"
#include "hddraw.h"
#include "dxhook.h"
#include "dxhelper.h"

#include "stdio.h"

static void Stopper(char *s, int line)
{
	char sMsg[81];
	sprintf_s(sMsg, 80, "break: \"%s\"", s);
	MessageBox(0, sMsg, "break", MB_OK | MB_ICONEXCLAMATION);
}

//#define STOPPER_TEST // comment out to eliminate
#ifdef STOPPER_TEST
#define STOPPER(s) Stopper(s, __LINE__)
#else
#define STOPPER(s)
#endif

/*
	dlg->m_DCEmulationMode = 0;
	if(t->flags2 & HOOKGDI) dlg->m_DCEmulationMode = 1;
	if(t->flags3 & EMULATEDC) dlg->m_DCEmulationMode = 2;
	if(t->flags & MAPGDITOPRIMARY) dlg->m_DCEmulationMode = 3;
*/

static HookEntry_Type Hooks[]={
	{"GetDeviceCaps", (FARPROC)GetDeviceCaps, (FARPROC *)&pGDIGetDeviceCaps, (FARPROC)extGetDeviceCaps},
	{"ScaleWindowExtEx", (FARPROC)ScaleWindowExtEx, (FARPROC *)&pGDIScaleWindowExtEx, (FARPROC)extScaleWindowExtEx},
	{"SaveDC", (FARPROC)SaveDC, (FARPROC *)&pGDISaveDC, (FARPROC)extGDISaveDC},
	{"RestoreDC", (FARPROC)RestoreDC, (FARPROC *)&pGDIRestoreDC, (FARPROC)extGDIRestoreDC},
	{"AnimatePalette", (FARPROC)AnimatePalette, (FARPROC *)&pAnimatePalette, (FARPROC)extAnimatePalette},
	{"CreatePalette", (FARPROC)CreatePalette, (FARPROC *)&pGDICreatePalette, (FARPROC)extGDICreatePalette},
	{"SelectPalette", (FARPROC)SelectPalette, (FARPROC *)&pGDISelectPalette, (FARPROC)extSelectPalette},
	{"RealizePalette", (FARPROC)RealizePalette, (FARPROC *)&pGDIRealizePalette, (FARPROC)extRealizePalette},
	{"GetSystemPaletteEntries", (FARPROC)GetSystemPaletteEntries, (FARPROC *)&pGDIGetSystemPaletteEntries, (FARPROC)extGetSystemPaletteEntries},
	{"SetSystemPaletteUse", (FARPROC)SetSystemPaletteUse, (FARPROC *)&pSetSystemPaletteUse, (FARPROC)extSetSystemPaletteUse},
	{"StretchDIBits", (FARPROC)StretchDIBits, (FARPROC *)&pStretchDIBits, (FARPROC)extStretchDIBits},
	//{"SetDIBitsToDevice", (FARPROC)NULL, (FARPROC *)&pSetDIBitsToDevice, (FARPROC)extSetDIBitsToDevice},
	//{"CreateCompatibleBitmap", (FARPROC)NULL, (FARPROC *)&pCreateCompatibleBitmap, (FARPROC)extCreateCompatibleBitmap},
	//{"SetMapMode", (FARPROC)NULL, (FARPROC *)NULL, (FARPROC)extSetMapMode},
	{0, NULL, 0, 0} // terminator
}; 

static HookEntry_Type RemapHooks[]={
	{"SetViewportOrgEx", (FARPROC)SetViewportOrgEx, (FARPROC *)&pSetViewportOrgEx, (FARPROC)extSetViewportOrgEx}, // needed in ShowBanner
	{"SetViewportExtEx", (FARPROC)NULL, (FARPROC *)&pSetViewportExtEx, (FARPROC)extSetViewportExtEx},
	{"GetViewportOrgEx", (FARPROC)NULL, (FARPROC *)&pGetViewportOrgEx, (FARPROC)extGetViewportOrgEx},
	{"GetWindowOrgEx", (FARPROC)NULL, (FARPROC *)&pGetWindowOrgEx, (FARPROC)extGetWindowOrgEx},
	{"SetWindowOrgEx", (FARPROC)NULL, (FARPROC *)&pSetWindowOrgEx, (FARPROC)extSetWindowOrgEx},
	{"GetCurrentPositionEx", (FARPROC)NULL, (FARPROC *)&pGetCurrentPositionEx, (FARPROC)extGetCurrentPositionEx},
	{"StretchDIBits", (FARPROC)StretchDIBits, (FARPROC *)&pStretchDIBits, (FARPROC)extStretchDIBits}, // unuseful
	{"SetDIBitsToDevice", (FARPROC)NULL, (FARPROC *)&pSetDIBitsToDevice, (FARPROC)extSetDIBitsToDevice}, // does the stretching
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type ScaledHooks[]={
	{"Rectangle", (FARPROC)Rectangle, (FARPROC *)&pGDIRectangle, (FARPROC)extRectangle},
	{"TextOutA", (FARPROC)TextOutA, (FARPROC *)&pGDITextOutA, (FARPROC)extTextOutA},
	{"GetClipBox", (FARPROC)NULL, (FARPROC *)&pGDIGetClipBox, (FARPROC)extGetClipBox},
	{"Polyline", (FARPROC)NULL, (FARPROC *)&pPolyline, (FARPROC)extPolyline},
	{"PolyBezierTo", (FARPROC)NULL, (FARPROC *)&pPolyBezierTo, (FARPROC)extPolyBezierTo},
	{"PolylineTo", (FARPROC)NULL, (FARPROC *)&pPolylineTo, (FARPROC)extPolylineTo},
	{"PolyDraw", (FARPROC)NULL, (FARPROC *)&pPolyDraw, (FARPROC)extPolyDraw},
	{"MoveToEx", (FARPROC)NULL, (FARPROC *)&pMoveToEx, (FARPROC)extMoveToEx},
	{"ArcTo", (FARPROC)NULL, (FARPROC *)&pArcTo, (FARPROC)extArcTo},
	{"LineTo", (FARPROC)NULL, (FARPROC *)&pLineTo, (FARPROC)extLineTo},
	{"SetPixel", (FARPROC)NULL, (FARPROC *)&pSetPixel, (FARPROC)extSetPixel},
	{"Ellipse", (FARPROC)NULL, (FARPROC *)&pEllipse, (FARPROC)extEllipse},
	{"Polygon", (FARPROC)NULL, (FARPROC *)&pPolygon, (FARPROC)extPolygon},
	{"Arc", (FARPROC)NULL, (FARPROC *)&pArc, (FARPROC)extArc},
	{"CreateEllipticRgn", (FARPROC)NULL, (FARPROC *)&pCreateEllipticRgn, (FARPROC)extCreateEllipticRgn},
	{"CreateEllipticRgnIndirect", (FARPROC)NULL, (FARPROC *)&pCreateEllipticRgnIndirect, (FARPROC)extCreateEllipticRgnIndirect},
	{"CreateRectRgn", (FARPROC)NULL, (FARPROC *)&pCreateRectRgn, (FARPROC)extCreateRectRgn},
	{"CreateRectRgnIndirect", (FARPROC)NULL, (FARPROC *)&pCreateRectRgnIndirect, (FARPROC)extCreateRectRgnIndirect},
	{"CreatePolygonRgn", (FARPROC)NULL, (FARPROC *)&pCreatePolygonRgn, (FARPROC)extCreatePolygonRgn},
	{"DrawTextA", (FARPROC)NULL, (FARPROC *)&pDrawText, (FARPROC)extDrawText},
	{"DrawTextExA", (FARPROC)NULL, (FARPROC *)&pDrawTextEx, (FARPROC)extDrawTextEx},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type EmulateHooks[]={
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type DDHooks[]={
	{"CreateCompatibleDC", (FARPROC)CreateCompatibleDC, (FARPROC *)&pGDICreateCompatibleDC, (FARPROC)extDDCreateCompatibleDC},
	{"DeleteDC", (FARPROC)DeleteDC, (FARPROC *)&pGDIDeleteDC, (FARPROC)extDDDeleteDC},
	{"CreateDCA", (FARPROC)CreateDCA, (FARPROC *)&pGDICreateDC, (FARPROC)extDDCreateDC},
	{"BitBlt", (FARPROC)BitBlt, (FARPROC *)&pGDIBitBlt, (FARPROC)extDDBitBlt},
	{"StretchBlt", (FARPROC)StretchBlt, (FARPROC *)&pGDIStretchBlt, (FARPROC)extDDStretchBlt},
	// {"PatBlt", (FARPROC)PatBlt, (FARPROC *)&pGDIPatBlt, (FARPROC)extDDPatBlt}, // missing one ...
	// {"MaskBlt", (FARPROC)NULL, (FARPROC *)&pMaskBlt, (FARPROC)extMaskBlt},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type GDIHooks[]={
	{"CreateCompatibleDC", (FARPROC)CreateCompatibleDC, (FARPROC *)&pGDICreateCompatibleDC, (FARPROC)extGDICreateCompatibleDC},
	{"DeleteDC", (FARPROC)DeleteDC, (FARPROC *)&pGDIDeleteDC, (FARPROC)extGDIDeleteDC},
	{"CreateDCA", (FARPROC)CreateDCA, (FARPROC *)&pGDICreateDC, (FARPROC)extGDICreateDC},
	{"BitBlt", (FARPROC)BitBlt, (FARPROC *)&pGDIBitBlt, (FARPROC)extGDIBitBlt},
	{"StretchBlt", (FARPROC)StretchBlt, (FARPROC *)&pGDIStretchBlt, (FARPROC)extGDIStretchBlt},
	{"PatBlt", (FARPROC)PatBlt, (FARPROC *)&pGDIPatBlt, (FARPROC)extGDIPatBlt},
	{"MaskBlt", (FARPROC)NULL, (FARPROC *)&pMaskBlt, (FARPROC)extMaskBlt},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type EmuHooks[]={
	{"CreateFontA", (FARPROC)CreateFont, (FARPROC *)&pGDICreateFont, (FARPROC)extCreateFont},
	{"CreateFontIndirectA", (FARPROC)CreateFontIndirectA, (FARPROC *)&pGDICreateFontIndirect, (FARPROC)extCreateFontIndirect},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type GammaHooks[]={
	{"SetDeviceGammaRamp", (FARPROC)SetDeviceGammaRamp, (FARPROC *)&pGDISetDeviceGammaRamp, (FARPROC)extSetDeviceGammaRamp},
	{"GetDeviceGammaRamp", (FARPROC)GetDeviceGammaRamp, (FARPROC *)&pGDIGetDeviceGammaRamp, (FARPROC)extGetDeviceGammaRamp},
	{0, NULL, 0, 0} // terminator
};

static HookEntry_Type FontHooks[]={
	{"CreateScalableFontResourceA", (FARPROC)NULL, (FARPROC *)&pCreateScalableFontResourceA, (FARPROC)extCreateScalableFontResourceA},
	{"CreateScalableFontResourceW", (FARPROC)NULL, (FARPROC *)&pCreateScalableFontResourceW, (FARPROC)extCreateScalableFontResourceW},
	{"AddFontResourceA", (FARPROC)NULL, (FARPROC *)&pAddFontResourceA, (FARPROC)extAddFontResourceA},
	{"AddFontResourceW", (FARPROC)NULL, (FARPROC *)&pAddFontResourceW, (FARPROC)extAddFontResourceW},
	{0, NULL, 0, 0} // terminator
};

extern HRESULT WINAPI extDirectDrawCreate(GUID FAR *, LPDIRECTDRAW FAR *, IUnknown FAR *);
extern HRESULT WINAPI extDirectDrawCreateEx(GUID FAR *, LPDIRECTDRAW FAR *, REFIID, IUnknown FAR *);

static char *libname = "gdi32.dll";

void HookGDI32Init()
{
	HookLibInit(Hooks);
	HookLibInit(RemapHooks);
	HookLibInit(DDHooks);
	HookLibInit(EmuHooks);
	HookLibInit(GammaHooks);
}

void HookGDI32(HMODULE module)
{
	HookLibrary(module, Hooks, libname);

	if(dxw.dwFlags1 & CLIENTREMAPPING)
		HookLibrary(module, RemapHooks, libname);

	if(dxw.dwFlags3 & EMULATEDC){
		HookLibrary(module, EmulateHooks, libname);
		HookLibrary(module, ScaledHooks, libname);
		HookLibrary(module, GDIHooks, libname);
	}

	if(dxw.dwFlags2 & HOOKGDI){
		HookLibrary(module, EmulateHooks, libname);
		HookLibrary(module, ScaledHooks, libname);
		HookLibrary(module, GDIHooks, libname);
	}

	if(dxw.dwFlags1 & MAPGDITOPRIMARY){
		HookLibrary(module, EmulateHooks, libname);
		HookLibrary(module, ScaledHooks, libname);
		HookLibrary(module, DDHooks, libname);
	}

	if ((dxw.dwFlags1 & EMULATESURFACE) && (dxw.dwFlags1 & HANDLEDC))
		HookLibrary(module, EmuHooks, libname);

	if(dxw.dwFlags2 & DISABLEGAMMARAMP)
		HookLibrary(module, GammaHooks, libname);

	if(dxw.dwFlags3 & FONTBYPASS) // v2.02.33 - for "Stratego" compatibility option
		HookLibrary(module, FontHooks, libname);
}

FARPROC Remap_GDI32_ProcAddress(LPCSTR proc, HMODULE hModule)
{
	FARPROC addr;

	if(addr=RemapLibrary(proc, hModule, Hooks)) return addr;

	if(dxw.dwFlags1 & CLIENTREMAPPING)
		if(addr=RemapLibrary(proc, hModule, RemapHooks)) return addr;

	if(dxw.dwFlags3 & EMULATEDC){
		if (addr=RemapLibrary(proc, hModule, EmulateHooks)) return addr;
		if (addr=RemapLibrary(proc, hModule, ScaledHooks)) return addr;
		if (addr=RemapLibrary(proc, hModule, GDIHooks)) return addr;
	}

	if(dxw.dwFlags2 & HOOKGDI){
		if (addr=RemapLibrary(proc, hModule, EmulateHooks)) return addr;
		if (addr=RemapLibrary(proc, hModule, ScaledHooks)) return addr;
		if (addr=RemapLibrary(proc, hModule, GDIHooks)) return addr;
	}

	if(dxw.dwFlags1 & MAPGDITOPRIMARY){
		if (addr=RemapLibrary(proc, hModule, EmulateHooks)) return addr;
		if (addr=RemapLibrary(proc, hModule, ScaledHooks)) return addr;
		if (addr=RemapLibrary(proc, hModule, DDHooks)) return addr;
	}

	if ((dxw.dwFlags1 & EMULATESURFACE) && (dxw.dwFlags1 & HANDLEDC))
		if(addr=RemapLibrary(proc, hModule, EmuHooks)) return addr;

	if(dxw.dwFlags2 & DISABLEGAMMARAMP)
		if(addr=RemapLibrary(proc, hModule, GammaHooks)) return addr;

	if(dxw.dwFlags3 & FONTBYPASS) // v2.02.33 - for "Stratego" compatibility option
		if(addr=RemapLibrary(proc, hModule, FontHooks)) return addr;

	return NULL;
}

//--------------------------------------------------------------------------------------------
//
// extern and common functions
//
//--------------------------------------------------------------------------------------------

extern DEVMODE *pSetDevMode;
extern DWORD PaletteEntries[256];
extern Unlock4_Type pUnlockMethod(LPDIRECTDRAWSURFACE);
extern HRESULT WINAPI sBlt(char *, LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX, BOOL);

extern GetDC_Type pGetDC;
extern ReleaseDC_Type pReleaseDC;

static COLORREF GetMatchingColor(COLORREF crColor)
{
	int iDistance, iMinDistance;
	int iColorIndex, iMinColorIndex;
	COLORREF PalColor;

	iMinDistance=0xFFFFFF;
	iMinColorIndex=0;

	for(iColorIndex=0; iColorIndex<256; iColorIndex++){
		int iDist;
		iDistance=0;

		PalColor=PaletteEntries[iColorIndex];
		switch(dxw.ActualPixelFormat.dwRGBBitCount){
		case 32:
			PalColor = ((PalColor & 0x00FF0000) >> 16) | (PalColor & 0x0000FF00) | ((PalColor & 0x000000FF) << 16);
			break;
		case 16:
			if(dxw.ActualPixelFormat.dwGBitMask==0x03E0){
				// RGB555 screen settings
				PalColor = ((PalColor & 0x7C00) >> 7) | ((PalColor & 0x03E0) << 6) | ((PalColor & 0x001F) << 19);
			}
			else {
				// RGB565 screen settings
				PalColor = ((PalColor & 0xF800) >> 8) | ((PalColor & 0x07E0) << 5) | ((PalColor & 0x001F) << 19);
			}
			break;
		}

		iDist = (crColor & 0x00FF0000) - (PalColor & 0x00FF0000);
		iDist >>= 16;
		if (iDist<0) iDist=-iDist;
		iDist *= iDist;
		iDistance += iDist;

		iDist = (crColor & 0x0000FF00) - (PalColor & 0x0000FF00);
		iDist >>= 8;
		if (iDist<0) iDist=-iDist;
		iDist *= iDist;
		iDistance += iDist;

		iDist = (crColor & 0x000000FF) - (PalColor & 0x000000FF);
		// iDist >>= 0;
		if (iDist<0) iDist=-iDist;
		iDist *= iDist;
		iDistance += iDist;

		if (iDistance < iMinDistance) {
			iMinDistance = iDistance;
			iMinColorIndex = iColorIndex;
		}

		if (iMinDistance==0) break; // got the perfect match!
	}
	OutTraceDW("GetMatchingColor: color=%x matched with palette[%d]=%x dist=%d\n", 
		crColor, iMinColorIndex, PaletteEntries[iMinColorIndex], iDistance);
	PalColor=PaletteEntries[iMinColorIndex];
	switch(dxw.ActualPixelFormat.dwRGBBitCount){
	case 32:
		crColor = ((PalColor & 0x00FF0000) >> 16) | (PalColor & 0x0000FF00) | ((PalColor & 0x000000FF) << 16); 
		break;
	case 16:
		if(dxw.ActualPixelFormat.dwGBitMask==0x03E0){
			// RGB555 screen settings
			crColor = ((PalColor & 0x7C00) >> 7) | ((PalColor & 0x03E0) << 6) | ((PalColor & 0x001F) << 19);
		}
		else {
			// RGB565 screen settings
			crColor = ((PalColor & 0xF800) >> 8) | ((PalColor & 0x07E0) << 5) | ((PalColor & 0x001F) << 19);
		}
		break;
	}
	return crColor;
}

//--------------------------------------------------------------------------------------------
//
// API hookers
//
//--------------------------------------------------------------------------------------------

int WINAPI extGetDeviceCaps(HDC hdc, int nindex)
{
	DWORD res;
	
	res = (*pGDIGetDeviceCaps)(hdc, nindex);
	if(IsTraceDDRAW){
		OutTrace("GetDeviceCaps: hdc=%x index=%x(%s)", hdc, nindex, ExplainDeviceCaps(nindex));
		switch(nindex){
			case RASTERCAPS:
				OutTrace(" res=0x%04x(%s)\n",res, ExplainRasterCaps(res)); break;
			case BITSPIXEL:
			case COLORRES:
			case VERTRES:
			case SIZEPALETTE:
			case NUMRESERVED:
				OutTrace(" res=%d\n",res); break;
			default:
				OutTrace(" res=0x%04x\n",res); break;
		}
	}


	// if you have a bypassed setting, use it first!
	if(pSetDevMode){
		switch(nindex){
		case BITSPIXEL:
		case COLORRES:
			res = pSetDevMode->dmBitsPerPel;
			OutTraceDW("GetDeviceCaps: fix(1) BITSPIXEL/COLORRES cap=%x\n",res);
			return res;
		case HORZRES:
			res = pSetDevMode->dmPelsWidth;
			OutTraceDW("GetDeviceCaps: fix(1) HORZRES cap=%d\n", res);
			return res;
		case VERTRES:
			res = pSetDevMode->dmPelsHeight;
			OutTraceDW("GetDeviceCaps: fix(1) VERTRES cap=%d\n", res);
			return res;
		}
	}

	switch(nindex){
	case VERTRES:
		res= dxw.GetScreenHeight();
		OutTraceDW("GetDeviceCaps: fix(2) VERTRES cap=%d\n", res);
		break;
	case HORZRES:
		res= dxw.GetScreenWidth();
		OutTraceDW("GetDeviceCaps: fix(2) HORZRES cap=%d\n", res);
		break;
	// WARNING: in no-emu mode, the INIT8BPP and INIT16BPP flags expose capabilities that
	// are NOT implemented and may cause later troubles!
	case RASTERCAPS:
		if(dxw.dwFlags2 & INIT8BPP) {
			res |= RC_PALETTE; // v2.02.12
			OutTraceDW("GetDeviceCaps: fix(2) RASTERCAPS setting RC_PALETTE cap=%x\n",res);
		}
		break;
	case BITSPIXEL:
	case COLORRES:
		if(dxw.dwFlags2 & (INIT8BPP|INIT16BPP)){ // v2.02.32 fix
			if(dxw.dwFlags2 & INIT8BPP) res = 8;
			if(dxw.dwFlags2 & INIT16BPP) res = 16;
			OutTraceDW("GetDeviceCaps: fix(2) BITSPIXEL/COLORRES cap=%d\n",res);
		}
		break;
	//case NUMCOLORS: // numcolors windows bug fix....
	//	if(res == -1) res=1;
	//	return res;
	}

	if(dxw.dwFlags1 & EMULATESURFACE){
		switch(nindex){
		case RASTERCAPS:
			if((dxw.VirtualPixelFormat.dwRGBBitCount==8) || (dxw.dwFlags2 & INIT8BPP)){
				res |= RC_PALETTE;
				OutTraceDW("GetDeviceCaps: fix(3) RASTERCAPS setting RC_PALETTE cap=%x(%s)\n", res, ExplainRasterCaps(res));
			}
			break;
		case BITSPIXEL:
		case COLORRES:
			int PrevRes;
			PrevRes=res;
			if(dxw.VirtualPixelFormat.dwRGBBitCount!=0) res = dxw.VirtualPixelFormat.dwRGBBitCount;
			if(dxw.dwFlags2 & INIT8BPP) res = 8;
			if(dxw.dwFlags2 & INIT16BPP) res = 16;
			if(PrevRes != res) OutTraceDW("GetDeviceCaps: fix(3) BITSPIXEL/COLORRES cap=%d\n",res);
			break;
		case SIZEPALETTE:
			res = 256;
			OutTraceDW("GetDeviceCaps: fix(3) SIZEPALETTE cap=%x\n",res);
			break;
		case NUMRESERVED:
			res = 0;
			OutTraceDW("GetDeviceCaps: fix(3) NUMRESERVED cap=%x\n",res);
			break;
		}
	}
	return res;
}

BOOL WINAPI extTextOutA(HDC hdc, int nXStart, int nYStart, LPCTSTR lpString, int cchString)
{
	BOOL ret;
	OutTraceDW("TextOut: hdc=%x xy=(%d,%d) str=(%d)\"%s\"\n", hdc, nXStart, nYStart, cchString, lpString);

	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		dxw.MapClient(&nXStart, &nYStart);
		OutTraceDW("TextOut: fixed dest=(%d,%d)\n", nXStart, nYStart);
	}

	ret=(*pGDITextOutA)(hdc, nXStart, nYStart, lpString, cchString);
	if(!ret) OutTraceE("TextOut: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extScaleWindowExtEx(HDC hdc, int Xnum, int Xdenom, int Ynum, int Ydenom, LPSIZE lpSize)
{
	OutTraceDW("ScaleWindowExtEx: hdc=%x num=(%d,%d) denom=(%d,%d) lpSize=%d\n",
		hdc, Xnum, Ynum, Xdenom, Ydenom, lpSize);

	MessageBox(0, "ScaleWindowExtEx", "to fix", MB_OK | MB_ICONEXCLAMATION);

	return (*pGDIScaleWindowExtEx)(hdc, Xnum, Xdenom, Ynum, Ydenom, lpSize);
}

BOOL WINAPI extRectangle(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect)
{
	int ret;

	OutTraceDW("Rectangle: hdc=%x xy=(%d,%d)-(%d,%d)\n", hdc, nLeftRect, nTopRect, nRightRect, nBottomRect);

	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		dxw.MapClient(&nLeftRect, &nTopRect, &nRightRect, &nBottomRect);
		OutTraceDW("Rectangle: fixed dest=(%d,%d)-(%d,%d)\n", nLeftRect, nTopRect, nRightRect, nBottomRect);
	}

	ret=(*pGDIRectangle)(hdc, nLeftRect, nTopRect, nRightRect, nBottomRect);
	if(!ret) OutTraceE("Rectangle: ERROR ret=%x\n", ret); 
	return ret;
}

int WINAPI extGDISaveDC(HDC hdc)
{
	int ret;

	ret=(*pGDISaveDC)(hdc);
	OutTraceDW("GDI.SaveDC: hdc=%x ret=%x\n", hdc, ret);
	return ret;
}

BOOL WINAPI extGDIRestoreDC(HDC hdc, int nSavedDC)
{
	BOOL ret;

	ret=(*pGDIRestoreDC)(hdc, nSavedDC);
	OutTraceDW("GDI.RestoreDC: hdc=%x nSavedDC=%x ret=%x\n", hdc, nSavedDC, ret);
	return ret;
}

/* --------------------------------------------------------------------------- */

// v2.1.75: Hooking for GDI32 CreatePalette, SelectPalette, RealizePalette: 
// maps the GDI palette to the buffered DirectDraw one. This fixes the screen 
// output for "Dementia" (a.k.a. "Armed & Delirious").

HPALETTE WINAPI extGDICreatePalette(CONST LOGPALETTE *plpal)
{
	HPALETTE ret;
	int idx;

	OutTraceDW("GDI.CreatePalette: plpal=%x version=%x NumEntries=%x\n", plpal, plpal->palVersion, plpal->palNumEntries);
	ret=(*pGDICreatePalette)(plpal);
	if(IsDebug){
		OutTraceDW("PalEntry[%x]= ", plpal->palNumEntries);
		for(idx=0; idx<plpal->palNumEntries; idx++) OutTraceDW("(%x)", plpal->palPalEntry[idx]);
		OutTraceDW("\n");
	}
	OutTraceDW("GDI.CreatePalette: hPalette=%x\n", ret);
	return ret;
}

HPALETTE hDesktopPalette=NULL;

HPALETTE WINAPI extSelectPalette(HDC hdc, HPALETTE hpal, BOOL bForceBackground)
{
	HPALETTE ret;

	ret=(*pGDISelectPalette)(hdc, hpal, bForceBackground);
	OutTraceDW("GDI.SelectPalette: hdc=%x hpal=%x ForceBackground=%x ret=%x\n", hdc, hpal, bForceBackground, ret);
	if((OBJ_DC == GetObjectType(hdc)) && (dxw.dwFlags1 & EMULATESURFACE)){
		OutTraceDW("GDI.SelectPalette: register desktop palette hpal=%x\n", hpal);
		hDesktopPalette=hpal;
	}
	return ret;
}

BOOL WINAPI extAnimatePalette(HPALETTE hpal, UINT iStartIndex, UINT cEntries, const PALETTEENTRY *ppe)
{
	// Invoked by "Pharaoh's Ascent 1.4"
	STOPPER("AnimatePalette");
	return TRUE;
}

UINT WINAPI extRealizePalette(HDC hdc)
{
	UINT ret;
	extern void mySetPalette(int, int, LPPALETTEENTRY);

	OutTraceDW("GDI.RealizePalette: hdc=%x\n", hdc);
	if((OBJ_DC == GetObjectType(hdc)) && (dxw.dwFlags1 & EMULATESURFACE)){
		PALETTEENTRY PalEntries[256];
		UINT nEntries;
		nEntries=GetPaletteEntries(hDesktopPalette, 0, 256, PalEntries);
		mySetPalette(0, nEntries, PalEntries);
		if(IsDebug) dxw.DumpPalette(nEntries, PalEntries);
		ret=DD_OK;
	}
	else
		ret=(*pGDIRealizePalette)(hdc);
	OutTraceDW("GDI.RealizePalette: hdc=%x ret=%x\n", hdc, ret);
	return ret;
}

// In emulated mode (when color depyth is 8BPP ?) it may happen that the game
// expects to get the requested system palette entries, while the 32BPP screen
// returns 0. "Mission Force Cyberstorm" is one of these. Returning the same
// value as nEntries, even though lppe is untouched, fixes the problem.

static PALETTEENTRY dp[256]={ // default palette, captured on my PC with video mode set to 8BPP
	{0x00,0x00,0x00,0x00},{0x80,0x00,0x00,0x00},{0x00,0x80,0x00,0x00},{0x80,0x80,0x00,0x00},
	{0x00,0x00,0x80,0x00},{0x80,0x00,0x80,0x00},{0x00,0x80,0x80,0x00},{0xc0,0xc0,0xc0,0x00},
	{0xa0,0xa0,0xa0,0x00},{0xf0,0xf0,0xf0,0x00},{0xc0,0xdc,0xc0,0x00},{0xa6,0xca,0xf0,0x00},
	{0x04,0x04,0x04,0x00},{0x08,0x08,0x08,0x00},{0x0c,0x0c,0x0c,0x00},{0x11,0x11,0x11,0x00},
	{0x16,0x16,0x16,0x00},{0x1c,0x1c,0x1c,0x00},{0x22,0x22,0x22,0x00},{0x29,0x29,0x29,0x00},
	{0x55,0x55,0x55,0x00},{0x4d,0x4d,0x4d,0x00},{0x42,0x42,0x42,0x00},{0x39,0x39,0x39,0x00},
	{0xff,0x7c,0x80,0x00},{0xff,0x50,0x50,0x00},{0xd6,0x00,0x93,0x00},{0xcc,0xec,0xff,0x00},
	{0xef,0xd6,0xc6,0x00},{0xe7,0xe7,0xd6,0x00},{0xad,0xa9,0x90,0x00},{0x33,0x00,0x00,0x00},
	{0x66,0x00,0x00,0x00},{0x99,0x00,0x00,0x00},{0xcc,0x00,0x00,0x00},{0x00,0x33,0x00,0x00},
	{0x33,0x33,0x00,0x00},{0x66,0x33,0x00,0x00},{0x99,0x33,0x00,0x00},{0xcc,0x33,0x00,0x00},
	{0xff,0x33,0x00,0x00},{0x00,0x66,0x00,0x00},{0x33,0x66,0x00,0x00},{0x66,0x66,0x00,0x00},
	{0x99,0x66,0x00,0x00},{0xcc,0x66,0x00,0x00},{0xff,0x66,0x00,0x00},{0x00,0x99,0x00,0x00},
	{0x33,0x99,0x00,0x00},{0x66,0x99,0x00,0x00},{0x99,0x99,0x00,0x00},{0xcc,0x99,0x00,0x00},
	{0xff,0x99,0x00,0x00},{0x00,0xcc,0x00,0x00},{0x33,0xcc,0x00,0x00},{0x66,0xcc,0x00,0x00},
	{0x99,0xcc,0x00,0x00},{0xcc,0xcc,0x00,0x00},{0xff,0xcc,0x00,0x00},{0x66,0xff,0x00,0x00},
	{0x99,0xff,0x00,0x00},{0xcc,0xff,0x00,0x00},{0x00,0x00,0x33,0x00},{0x33,0x00,0x33,0x00},
	{0x66,0x00,0x33,0x00},{0x99,0x00,0x33,0x00},{0xcc,0x00,0x33,0x00},{0xff,0x00,0x33,0x00},
	{0x00,0x33,0x33,0x00},{0x33,0x33,0x33,0x00},{0x66,0x33,0x33,0x00},{0x99,0x33,0x33,0x00},
	{0xcc,0x33,0x33,0x00},{0xff,0x33,0x33,0x00},{0x00,0x66,0x33,0x00},{0x33,0x66,0x33,0x00},
	{0x66,0x66,0x33,0x00},{0x99,0x66,0x33,0x00},{0xcc,0x66,0x33,0x00},{0xff,0x66,0x33,0x00},
	{0x00,0x99,0x33,0x00},{0x33,0x99,0x33,0x00},{0x66,0x99,0x33,0x00},{0x99,0x99,0x33,0x00},
	{0xcc,0x99,0x33,0x00},{0xff,0x99,0x33,0x00},{0x00,0xcc,0x33,0x00},{0x33,0xcc,0x33,0x00},
	{0x66,0xcc,0x33,0x00},{0x99,0xcc,0x33,0x00},{0xcc,0xcc,0x33,0x00},{0xff,0xcc,0x33,0x00},
	{0x33,0xff,0x33,0x00},{0x66,0xff,0x33,0x00},{0x99,0xff,0x33,0x00},{0xcc,0xff,0x33,0x00},
	{0xff,0xff,0x33,0x00},{0x00,0x00,0x66,0x00},{0x33,0x00,0x66,0x00},{0x66,0x00,0x66,0x00},
	{0x99,0x00,0x66,0x00},{0xcc,0x00,0x66,0x00},{0xff,0x00,0x66,0x00},{0x00,0x33,0x66,0x00},
	{0x33,0x33,0x66,0x00},{0x66,0x33,0x66,0x00},{0x99,0x33,0x66,0x00},{0xcc,0x33,0x66,0x00},
	{0xff,0x33,0x66,0x00},{0x00,0x66,0x66,0x00},{0x33,0x66,0x66,0x00},{0x66,0x66,0x66,0x00},
	{0x99,0x66,0x66,0x00},{0xcc,0x66,0x66,0x00},{0x00,0x99,0x66,0x00},{0x33,0x99,0x66,0x00},
	{0x66,0x99,0x66,0x00},{0x99,0x99,0x66,0x00},{0xcc,0x99,0x66,0x00},{0xff,0x99,0x66,0x00},
	{0x00,0xcc,0x66,0x00},{0x33,0xcc,0x66,0x00},{0x99,0xcc,0x66,0x00},{0xcc,0xcc,0x66,0x00},
	{0xff,0xcc,0x66,0x00},{0x00,0xff,0x66,0x00},{0x33,0xff,0x66,0x00},{0x99,0xff,0x66,0x00},
	{0xcc,0xff,0x66,0x00},{0xff,0x00,0xcc,0x00},{0xcc,0x00,0xff,0x00},{0x00,0x99,0x99,0x00},
	{0x99,0x33,0x99,0x00},{0x99,0x00,0x99,0x00},{0xcc,0x00,0x99,0x00},{0x00,0x00,0x99,0x00},
	{0x33,0x33,0x99,0x00},{0x66,0x00,0x99,0x00},{0xcc,0x33,0x99,0x00},{0xff,0x00,0x99,0x00},
	{0x00,0x66,0x99,0x00},{0x33,0x66,0x99,0x00},{0x66,0x33,0x99,0x00},{0x99,0x66,0x99,0x00},
	{0xcc,0x66,0x99,0x00},{0xff,0x33,0x99,0x00},{0x33,0x99,0x99,0x00},{0x66,0x99,0x99,0x00},
	{0x99,0x99,0x99,0x00},{0xcc,0x99,0x99,0x00},{0xff,0x99,0x99,0x00},{0x00,0xcc,0x99,0x00},
	{0x33,0xcc,0x99,0x00},{0x66,0xcc,0x66,0x00},{0x99,0xcc,0x99,0x00},{0xcc,0xcc,0x99,0x00},
	{0xff,0xcc,0x99,0x00},{0x00,0xff,0x99,0x00},{0x33,0xff,0x99,0x00},{0x66,0xcc,0x99,0x00},
	{0x99,0xff,0x99,0x00},{0xcc,0xff,0x99,0x00},{0xff,0xff,0x99,0x00},{0x00,0x00,0xcc,0x00},
	{0x33,0x00,0x99,0x00},{0x66,0x00,0xcc,0x00},{0x99,0x00,0xcc,0x00},{0xcc,0x00,0xcc,0x00},
	{0x00,0x33,0x99,0x00},{0x33,0x33,0xcc,0x00},{0x66,0x33,0xcc,0x00},{0x99,0x33,0xcc,0x00},
	{0xcc,0x33,0xcc,0x00},{0xff,0x33,0xcc,0x00},{0x00,0x66,0xcc,0x00},{0x33,0x66,0xcc,0x00},
	{0x66,0x66,0x99,0x00},{0x99,0x66,0xcc,0x00},{0xcc,0x66,0xcc,0x00},{0xff,0x66,0x99,0x00},
	{0x00,0x99,0xcc,0x00},{0x33,0x99,0xcc,0x00},{0x66,0x99,0xcc,0x00},{0x99,0x99,0xcc,0x00},
	{0xcc,0x99,0xcc,0x00},{0xff,0x99,0xcc,0x00},{0x00,0xcc,0xcc,0x00},{0x33,0xcc,0xcc,0x00},
	{0x66,0xcc,0xcc,0x00},{0x99,0xcc,0xcc,0x00},{0xcc,0xcc,0xcc,0x00},{0xff,0xcc,0xcc,0x00},
	{0x00,0xff,0xcc,0x00},{0x33,0xff,0xcc,0x00},{0x66,0xff,0x99,0x00},{0x99,0xff,0xcc,0x00},
	{0xcc,0xff,0xcc,0x00},{0xff,0xff,0xcc,0x00},{0x33,0x00,0xcc,0x00},{0x66,0x00,0xff,0x00},
	{0x99,0x00,0xff,0x00},{0x00,0x33,0xcc,0x00},{0x33,0x33,0xff,0x00},{0x66,0x33,0xff,0x00},
	{0x99,0x33,0xff,0x00},{0xcc,0x33,0xff,0x00},{0xff,0x33,0xff,0x00},{0x00,0x66,0xff,0x00},
	{0x33,0x66,0xff,0x00},{0x66,0x66,0xcc,0x00},{0x99,0x66,0xff,0x00},{0xcc,0x66,0xff,0x00},
	{0xff,0x66,0xcc,0x00},{0x00,0x99,0xff,0x00},{0x33,0x99,0xff,0x00},{0x66,0x99,0xff,0x00},
	{0x99,0x99,0xff,0x00},{0xcc,0x99,0xff,0x00},{0xff,0x99,0xff,0x00},{0x00,0xcc,0xff,0x00},
	{0x33,0xcc,0xff,0x00},{0x66,0xcc,0xff,0x00},{0x99,0xcc,0xff,0x00},{0xcc,0xcc,0xff,0x00},
	{0xff,0xcc,0xff,0x00},{0x33,0xff,0xff,0x00},{0x66,0xff,0xcc,0x00},{0x99,0xff,0xff,0x00},
	{0xcc,0xff,0xff,0x00},{0xff,0x66,0x66,0x00},{0x66,0xff,0x66,0x00},{0xff,0xff,0x66,0x00},
	{0x66,0x66,0xff,0x00},{0xff,0x66,0xff,0x00},{0x66,0xff,0xff,0x00},{0xa5,0x00,0x21,0x00},
	{0x5f,0x5f,0x5f,0x00},{0x77,0x77,0x77,0x00},{0x86,0x86,0x86,0x00},{0x96,0x96,0x96,0x00},
	{0xcb,0xcb,0xcb,0x00},{0xb2,0xb2,0xb2,0x00},{0xd7,0xd7,0xd7,0x00},{0xdd,0xdd,0xdd,0x00},
	{0xe3,0xe3,0xe3,0x00},{0xea,0xea,0xea,0x00},{0xff,0xfb,0xf0,0x00},{0xa0,0xa0,0xa4,0x00},
	{0x80,0x80,0x80,0x00},{0xff,0x00,0x00,0x00},{0x00,0xff,0x00,0x00},{0xff,0xff,0x00,0x00},
	{0x00,0x00,0xff,0x00},{0xff,0x00,0xff,0x00},{0x00,0xff,0xff,0x00},{0xff,0xff,0xff,0x00}
};

UINT WINAPI extGetSystemPaletteEntries(HDC hdc, UINT iStartIndex, UINT nEntries, LPPALETTEENTRY lppe)
{
	int ret;

	OutTraceDW("GetSystemPaletteEntries: hdc=%x start=%d num=%d\n", hdc, iStartIndex, nEntries);
	ret=(*pGDIGetSystemPaletteEntries)(hdc, iStartIndex, nEntries, lppe);
	OutTraceDW("GetSystemPaletteEntries: ret=%d\n", ret);
	if((ret == 0) && (dxw.dwFlags1 & EMULATESURFACE)) {
		// use static default data...
		for(UINT idx=0; idx<nEntries; idx++) lppe[idx]=dp[iStartIndex+idx]; 
		ret = nEntries;
		OutTraceDW("GetSystemPaletteEntries: FIXED ret=%d\n", ret);
	}
	if(IsDebug) dxw.DumpPalette(nEntries, &lppe[iStartIndex]);
	return ret;
}

/* -------------------------------------------------------------------- */
// directdraw supported GDI calls
/* -------------------------------------------------------------------- */

// PrimHDC: DC handle of the selected DirectDraw primary surface. NULL when invalid.
extern HDC PrimHDC;

HDC WINAPI extDDCreateCompatibleDC(HDC hdc)
{
	HDC RetHdc, SrcHdc;
	extern GetDC_Type pGetDC;

	OutTraceDW("GDI.CreateCompatibleDC: hdc=%x\n", hdc);

	if(hdc==0 && pGetDC && dxw.IsFullScreen()){
		dxw.SetPrimarySurface();
		(*pGetDC)(dxw.lpDDSPrimHDC,&SrcHdc);
		OutTraceDW("GDI.CreateCompatibleDC: duplicating screen HDC lpDDSPrimHDC=%x\n", dxw.lpDDSPrimHDC); 
		RetHdc=(*pGDICreateCompatibleDC)(SrcHdc);
		(*pReleaseDC)(dxw.lpDDSPrimHDC,SrcHdc);
	}
	else
		RetHdc=(*pGDICreateCompatibleDC)(hdc);

	if(RetHdc)
		OutTraceDW("GDI.CreateCompatibleDC: returning HDC=%x\n", RetHdc);
	else
		OutTraceE("GDI.CreateCompatibleDC ERROR: err=%d at %d\n", GetLastError(), __LINE__);

	return RetHdc;
}

BOOL WINAPI extDDDeleteDC(HDC hdc)
{
	BOOL res;

	OutTraceDW("GDI.DeleteDC: hdc=%x\n", hdc);

	res=(*pGDIDeleteDC)(hdc);
	if(!res) OutTraceE("GDI.DeleteDC: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	return res;
}

#if 1
static HDC WINAPI winDDGetDC(HWND hwnd, char *api)
{
	HDC hdc;
	HRESULT res;
	extern HRESULT WINAPI extGetDC(LPDIRECTDRAWSURFACE, HDC FAR *);

	OutTraceDW("%s: hwnd=%x\n", api, hwnd);

	dxw.ResetPrimarySurface();
	dxw.SetPrimarySurface();
	if(dxw.IsRealDesktop(hwnd)) hwnd=dxw.GethWnd();

	if(dxw.lpDDSPrimHDC){ 
		if (PrimHDC){
			OutTraceDW("%s: reusing primary hdc\n", api);
			(*pUnlockMethod(dxw.lpDDSPrimHDC))(dxw.lpDDSPrimHDC, NULL);
			hdc=PrimHDC;
		}
		else{
			OutTraceDW("%s: get hdc from PRIMARY surface lpdds=%x\n", api, dxw.lpDDSPrimHDC);
			res=extGetDC(dxw.lpDDSPrimHDC,&hdc);
			if(res) {
				OutTraceE("%s: GetDC(%x) ERROR %x(%s) at %d\n", api, dxw.lpDDSPrimHDC, res, ExplainDDError(res), __LINE__);
				if(res==DDERR_DCALREADYCREATED){
					// try recovery....
					(*pReleaseDC)(dxw.lpDDSPrimHDC,NULL);
					res=extGetDC(dxw.lpDDSPrimHDC,&hdc);
				}
				if(res)return 0;
			}
			PrimHDC=hdc;
		}
	}
	else {
		hdc=(*pGDIGetDC)(hwnd ? hwnd : dxw.GethWnd());
		OutTraceDW("%s: returning window DC handle hwnd=%x hdc=%x\n", api, hwnd, hdc);
		PrimHDC=NULL;
	}

	if(hdc)
		OutTraceDW("%s: hwnd=%x hdc=%x\n", api, hwnd, hdc);
	else
		OutTraceE("%s: ERROR err=%d at %d\n", api, GetLastError, __LINE__);
	return(hdc);
}
#else
HDC hPrimaryDC=NULL;
static HDC WINAPI winDDGetDC(HWND hwnd, char *api)
{
	HDC hdc;
	//HRESULT res;
	//extern HRESULT WINAPI extGetDC(LPDIRECTDRAWSURFACE, HDC FAR *);

	OutTraceDW("%s: hwnd=%x\n", api, hwnd);

	//dxw.ResetPrimarySurface();
	//dxw.SetPrimarySurface();
	if(dxw.IsRealDesktop(hwnd) && dxw.IsFullScreen()){
		LPDIRECTDRAWSURFACE lpPrim;
		HDC PrimDC;
		lpPrim=dxw.GetPrimarySurface();
		(*pGetDC)(lpPrim, &PrimDC);
		hdc=(*pGDICreateCompatibleDC)(PrimDC);
		(*pReleaseDC)(lpPrim, PrimDC);
		OutTraceDW("%s: returning DDRAW DC handle hwnd=%x hdc=%x\n", api, hwnd, hdc);
		hPrimaryDC=hdc;
		return hdc;
	}
	else {
		hdc=(*pGDIGetDC)(hwnd ? hwnd : dxw.GethWnd());
		OutTraceDW("%s: returning window DC handle hwnd=%x hdc=%x\n", api, hwnd, hdc);
		//PrimHDC=NULL;
	}

	if(hdc)
		OutTraceDW("%s: hwnd=%x hdc=%x\n", api, hwnd, hdc);
	else
		OutTraceE("%s: ERROR err=%d at %d\n", api, GetLastError, __LINE__);
	return(hdc);
}
#endif

HDC WINAPI extDDCreateDC(LPSTR Driver, LPSTR Device, LPSTR Output, CONST DEVMODE *InitData)
{
	HDC RetHDC;
	OutTraceDW("GDI.CreateDC: Driver=%s Device=%s Output=%s InitData=%x\n", 
		Driver?Driver:"(NULL)", Device?Device:"(NULL)", Output?Output:"(NULL)", InitData);

	if (!Driver || !strncmp(Driver,"DISPLAY",7)) {
		//HDC PrimHDC;
		LPDIRECTDRAWSURFACE lpdds;
		OutTraceDW("GDI.CreateDC: returning primary surface DC\n");
		lpdds=dxw.GetPrimarySurface();
		(*pGetDC)(lpdds, &PrimHDC);
		RetHDC=(*pGDICreateCompatibleDC)(PrimHDC);
		(*pReleaseDC)(lpdds, PrimHDC);
	}
	else{
		RetHDC=(*pGDICreateDC)(Driver, Device, Output, InitData);
	}
	if(RetHDC)
		OutTraceDW("GDI.CreateDC: returning HDC=%x\n", RetHDC);
	else
		OutTraceE("GDI.CreateDC ERROR: err=%d at %d\n", GetLastError(), __LINE__);
	return RetHDC;
}

HDC WINAPI extDDGetDC(HWND hwnd)
{
	HDC ret;
	ret=winDDGetDC(hwnd, "GDI.GetDC");
	return ret;
}

HDC WINAPI extDDGetWindowDC(HWND hwnd)
{
	HDC ret;
	ret=winDDGetDC(hwnd, "GDI.GetWindowDC");
	return ret;
}

#if 1
int WINAPI extDDReleaseDC(HWND hwnd, HDC hDC)
{
	int res;
	extern HRESULT WINAPI extReleaseDC(LPDIRECTDRAWSURFACE, HDC);

	OutTraceDW("GDI.ReleaseDC: hwnd=%x hdc=%x\n", hwnd, hDC);
	res=0;
	if ((hDC == PrimHDC) || (hwnd==0)){
		dxw.SetPrimarySurface();
		OutTraceDW("GDI.ReleaseDC: refreshing primary surface lpdds=%x\n",dxw.lpDDSPrimHDC);
		if(!dxw.lpDDSPrimHDC) return 0;
		extReleaseDC(dxw.lpDDSPrimHDC, hDC);
		PrimHDC=NULL;
		res=1; // 1 = OK
	}
	else {
		res=(*pGDIReleaseDC)(hwnd, hDC);
		if (!res) OutTraceE("GDI.ReleaseDC: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	}
	return(res);
}
#else
int WINAPI extDDReleaseDC(HWND hwnd, HDC hDC)
{
	int res;
	extern HRESULT WINAPI extReleaseDC(LPDIRECTDRAWSURFACE, HDC);

	OutTraceDW("GDI.ReleaseDC: hwnd=%x hdc=%x\n", hwnd, hDC);
	res=0;
	if (hDC == hPrimaryDC){
		OutTraceDW("GDI.ReleaseDC: refreshing primary surface lpdds=%x\n",dxw.lpDDSPrimHDC);
		//extReleaseDC(dxw.lpDDSPrimHDC, hDC);
		hPrimaryDC=NULL;
		res=1; // 1 = OK
	}
	else {
		res=(*pGDIReleaseDC)(hwnd, hDC);
		if (!res) OutTraceE("GDI.ReleaseDC: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	}
	return(res);
}
#endif

BOOL WINAPI extDDBitBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc, int nXSrc, int nYSrc, DWORD dwRop)
{
	BOOL ret;
	HRESULT res;
	extern HRESULT WINAPI extGetDC(LPDIRECTDRAWSURFACE, HDC FAR *);

	OutTraceDW("GDI.BitBlt: HDC=%x nXDest=%d nYDest=%d nWidth=%d nHeight=%d hdcSrc=%x nXSrc=%d nYSrc=%d dwRop=%x(%s)\n", 
		hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop, ExplainROP(dwRop));

	ret=1; // OK

	if(hdcDest==0) hdcDest=PrimHDC;
	if(hdcDest==0) {
		dxw.ResetPrimarySurface();
		dxw.SetPrimarySurface();
		res=extGetDC(dxw.lpDDSPrimHDC, &PrimHDC);
		hdcDest=PrimHDC;
	}

	res=(*pGDIBitBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
	if(!res) OutTraceE("GDI.BitBlt: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	res=(*pGDIBitBlt)(NULL, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
	if(!res) ret=0;
	return ret;
}

BOOL WINAPI extDDStretchBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, 
							 HDC hdcSrc, int nXSrc, int nYSrc, int nWSrc, int nHSrc, DWORD dwRop)
{
	BOOL ret;
	HRESULT res;
	RECT ClientRect;

	OutTraceDW("GDI.StretchBlt: HDC=%x nXDest=%d nYDest=%d nWidth=%d nHeight=%d hdcSrc=%x nXSrc=%d nYSrc=%d nWSrc=%x nHSrc=%x dwRop=%x\n", 
		hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, nWSrc, nHSrc, dwRop);

	if(hdcDest != hdcSrc){
		(*pGetClientRect)(dxw.GethWnd(),&ClientRect);
		ret=(*pGDIStretchBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, nWidth, nHeight, dwRop);
		if(!ret) {
			OutTraceE("GDI.StretchBlt: ERROR err=%d at %d\n", GetLastError(), __LINE__);
			return ret;
		}
	}
	dxw.SetPrimarySurface();
	OutTraceDW("GDI.StretchBlt: refreshing primary surface lpdds=%x\n",dxw.lpDDSPrimHDC);
	sBlt("GDI.StretchBlt", dxw.lpDDSPrimHDC, NULL, dxw.lpDDSPrimHDC, NULL, 0, NULL, 0);
	res=(*pUnlockMethod(dxw.lpDDSPrimHDC))(dxw.lpDDSPrimHDC, NULL);
	return ret;
}

HDC WINAPI extGDICreateDC(LPSTR Driver, LPSTR Device, LPSTR Output, CONST DEVMODE *InitData)
{
	HDC WinHDC, RetHDC;
	OutTraceDW("GDI.CreateDC: Driver=%s Device=%s Output=%s InitData=%x\n", 
		Driver?Driver:"(NULL)", Device?Device:"(NULL)", Output?Output:"(NULL)", InitData);

	if (!Driver || !strncmp(Driver,"DISPLAY",7)) {
		OutTraceDW("GDI.CreateDC: returning window surface DC\n");
		WinHDC=(*pGDIGetDC)(dxw.GethWnd());
		RetHDC=(*pGDICreateCompatibleDC)(WinHDC);
		(*pGDIReleaseDC)(dxw.GethWnd(), WinHDC);
	}
	else{
		RetHDC=(*pGDICreateDC)(Driver, Device, Output, InitData);
	}
	if(RetHDC)
		OutTraceDW("GDI.CreateDC: returning HDC=%x\n", RetHDC);
	else
		OutTraceE("GDI.CreateDC ERROR: err=%d at %d\n", GetLastError(), __LINE__);
	return RetHDC;
}

HDC WINAPI extGDICreateCompatibleDC(HDC hdc)
{
	HDC RetHdc, SrcHdc;
	extern LPDIRECTDRAWSURFACE lpDDSHDC;
	extern GetDC_Type pGetDC;
	DWORD LastError;

	OutTraceDW("GDI.CreateCompatibleDC: hdc=%x\n", hdc);
	if(hdc==0){
		SrcHdc=(*pGDIGetDC)(dxw.GethWnd());
		OutTraceDW("GDI.CreateCompatibleDC: duplicating win HDC hWnd=%x\n", dxw.GethWnd()); 
	}

	// eliminated error message for errorcode 0.
	SetLastError(0);
	RetHdc=(*pGDICreateCompatibleDC)(hdc);
	LastError=GetLastError();
	if(!LastError)
		OutTraceDW("GDI.CreateCompatibleDC: returning HDC=%x\n", RetHdc);
	else
		OutTraceE("GDI.CreateCompatibleDC ERROR: err=%d at %d\n", LastError, __LINE__);
	return RetHdc;
}

BOOL WINAPI extGDIBitBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc, int nXSrc, int nYSrc, DWORD dwRop)
{
	BOOL res;
	extern BOOL isWithinDialog;

	OutTraceDW("GDI.BitBlt: HDC=%x nXDest=%d nYDest=%d nWidth=%d nHeight=%d hdcSrc=%x nXSrc=%d nYSrc=%d dwRop=%x(%s)\n", 
		hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop, ExplainROP(dwRop));

	OutTraceB("GDI.BitBlt: DEBUG FullScreen=%x target hdctype=%x(%s) hwnd=%x\n", 
		dxw.IsFullScreen(), GetObjectType(hdcDest), ExplainDCType(GetObjectType(hdcDest)), WindowFromDC(hdcDest));

	// beware: HDC could refer to screen DC that are written directly on screen, or memory DC that will be scaled to
	// the screen surface later on, on ReleaseDC or ddraw Blit / Flip operation. Scaling of rect coordinates is 
	// needed only in the first case, and must be avoided on the second, otherwise the image would be scaled twice!

	if (OBJ_DC == GetObjectType(hdcDest)){
		//if(dxw.IsRealDesktop(WindowFromDC(hdcDest))) hdcDest=GetDC(dxw.GethWnd()); // ??????
		if (dxw.HandleFPS()) return TRUE;
		if (dxw.dwFlags3 & NOGDIBLT) return TRUE;
		if(dxw.IsFullScreen()){
			int nWDest, nHDest;
			nWDest= nWidth;
			nHDest= nHeight;
			dxw.MapClient(&nXDest, &nYDest, &nWDest, &nHDest);
			res=(*pGDIStretchBlt)(hdcDest, nXDest, nYDest, nWDest, nHDest, hdcSrc, nXSrc, nYSrc, nWidth, nHeight, dwRop);
			dxw.ShowOverlay(hdcDest);
			OutTrace("Debug: DC dest=(%d,%d) size=(%d,%d)\n", nXDest, nYDest, nWDest, nHDest);
		}
		else if(WindowFromDC(hdcDest)==NULL){
			// V2.02.31: See StretchBlt.
			int nWDest, nHDest;
			nWDest= nWidth;
			nHDest= nHeight;
			dxw.MapWindow(&nXDest, &nYDest, &nWDest, &nHDest);
			res=(*pGDIStretchBlt)(hdcDest, nXDest, nYDest, nWDest, nHDest, hdcSrc, nXSrc, nYSrc, nWidth, nHeight, dwRop);
			dxw.ShowOverlay(hdcDest);
			OutTrace("Debug: NULL dest=(%d,%d) size=(%d,%d)\n", nXDest, nYDest, nWDest, nHDest);
		}
		else{
			res=(*pGDIBitBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
			OutTrace("Debug: PROXY dest=(%d,%d) size=(%d,%d)\n", nXDest, nYDest, nWidth, nHeight);
		}
	}
	else {
		res=(*pGDIBitBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
		OutTrace("Debug: MEM dest=(%d,%d) size=(%d,%d)\n", nXDest, nYDest, nWidth, nHeight);
	}

	if(!res) OutTraceE("GDI.BitBlt: ERROR err=%d at %d\n", GetLastError(), __LINE__);

	return res;
}

BOOL WINAPI extGDIPatBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, DWORD dwRop)
{
	BOOL res;
	BOOL IsToScreen;

	OutTraceDW("GDI.PatBlt: HDC=%x nXDest=%d nYDest=%d nWidth=%d nHeight=%d dwRop=%x(%s)\n", 
		hdcDest, nXDest, nYDest, nWidth, nHeight, dwRop, ExplainROP(dwRop));

	OutTraceB("GDI.PatBlt: DEBUG FullScreen=%x target hdctype=%x(%s) hwnd=%x\n", 
		dxw.IsFullScreen(), GetObjectType(hdcDest), ExplainDCType(GetObjectType(hdcDest)), WindowFromDC(hdcDest));

	IsToScreen=FALSE;
	res=0;
	if (OBJ_DC == GetObjectType(hdcDest)){
		IsToScreen=TRUE;
		if (dxw.HandleFPS()) return TRUE;
		if (dxw.dwFlags3 & NOGDIBLT) return TRUE;
		if (dxw.IsFullScreen()){ 
			dxw.MapClient(&nXDest, &nYDest, &nWidth, &nHeight);
			dxw.ShowOverlay(hdcDest);
			res=(*pGDIPatBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, dwRop);
		}
		else if(WindowFromDC(hdcDest)==NULL){
			// V2.02.31: See StretchBlt.
			dxw.MapWindow(&nXDest, &nYDest, &nWidth, &nHeight);
			res=(*pGDIPatBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, dwRop);
		}
	}
	else {
		res=(*pGDIPatBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, dwRop);
	}
	if (IsToScreen) dxw.ShowOverlay(hdcDest);
	if(!res) OutTraceE("GDI.PatBlt: ERROR err=%d at %d\n", GetLastError(), __LINE__);

	return res;
}

BOOL WINAPI extGDIStretchBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, 
							 HDC hdcSrc, int nXSrc, int nYSrc, int nWSrc, int nHSrc, DWORD dwRop)
{
	BOOL res;
	BOOL IsToScreen;

	OutTraceDW("GDI.StretchBlt: HDC=%x nXDest=%d nYDest=%d nWidth=%d nHeight=%d hdcSrc=%x nXSrc=%d nYSrc=%d nWSrc=%d nHSrc=%d dwRop=%x(%s)\n", 
		hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, nWSrc, nHSrc, dwRop, ExplainROP(dwRop));

	OutTraceB("GDI.StretchBlt: DEBUG FullScreen=%x target hdctype=%x(%s) hwnd=%x\n", 
		dxw.IsFullScreen(), GetObjectType(hdcDest), ExplainDCType(GetObjectType(hdcDest)), WindowFromDC(hdcDest));

	IsToScreen=FALSE;
	if (OBJ_DC == GetObjectType(hdcDest)){
		if (dxw.HandleFPS()) return TRUE;
		if (dxw.dwFlags3 & NOGDIBLT) return TRUE;
		IsToScreen=TRUE;
		if(dxw.IsFullScreen()){
			dxw.MapClient(&nXDest, &nYDest, &nWidth, &nHeight);
		}
		else if(WindowFromDC(hdcDest)==NULL){
			// V2.02.31: In "Silent Hunter II" intro movie, QuickTime 5 renders the video on the PrimarySurface->GetDC device context,
			// that is a memory device type associated to NULL (desktop) window, through GDI StretchBlt api. So, you shoud compensate
			// by scaling and offsetting to main window.
			dxw.MapWindow(&nXDest, &nYDest, &nWidth, &nHeight);
		}
	}

	res=(*pGDIStretchBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, nWSrc, nHSrc, dwRop);
	if (IsToScreen) dxw.ShowOverlay(hdcDest);
	if(!res) OutTraceE("GDI.StretchBlt: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	return res;
}

BOOL WINAPI extGDIDeleteDC(HDC hdc)
{
	BOOL res;
	OutTraceDW("GDI.DeleteDC: hdc=%x\n", hdc);
	res=(*pGDIDeleteDC)(hdc);
	if(!res) OutTraceE("GDI.DeleteDC: ERROR err=%d at %d\n", GetLastError(), __LINE__);
	return res;
}

COLORREF WINAPI extSetTextColor(HDC hdc, COLORREF crColor)
{
	COLORREF res;

	if ((dxw.dwFlags1 & EMULATESURFACE) && (dxw.dwFlags1 & HANDLEDC) && (dxw.VirtualPixelFormat.dwRGBBitCount==8))
		crColor=GetMatchingColor(crColor);

	res=(*pGDISetTextColor)(hdc, crColor);
	OutTraceDW("SetTextColor: color=%x res=%x%s\n", crColor, res, (res==CLR_INVALID)?"(CLR_INVALID)":"");
	return res;
}

COLORREF WINAPI extSetBkColor(HDC hdc, COLORREF crColor)
{
	COLORREF res;

	if ((dxw.dwFlags1 & EMULATESURFACE) && (dxw.dwFlags1 & HANDLEDC) && (dxw.VirtualPixelFormat.dwRGBBitCount==8))
		crColor=GetMatchingColor(crColor);

	res=(*pGDISetBkColor)(hdc, crColor);
	OutTraceDW("SetBkColor: color=%x res=%x%s\n", crColor, res, (res==CLR_INVALID)?"(CLR_INVALID)":"");
	return res;
}

HFONT WINAPI extCreateFont(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight,
				 DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet,
				 DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality,
				 DWORD fdwPitchAndFamily, LPCTSTR lpszFace)
{
	OutTraceDW("CreateFont: h=%d w=%d face=\"%s\"\n", nHeight, nWidth, lpszFace);
	if(dxw.dwFlags1 & FIXTEXTOUT) {
		if(nHeight > 0) dxw.MapClient(&nWidth, &nHeight);
		else {
			nHeight= -nHeight;
			dxw.MapClient(&nWidth, &nHeight);
			nHeight= -nHeight;
		}
	}
	return (*pGDICreateFont)(nHeight, nWidth, nEscapement, nOrientation, fnWeight,
				 fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet,
				 fdwOutputPrecision, fdwClipPrecision, NONANTIALIASED_QUALITY,
				 fdwPitchAndFamily, lpszFace);
}

// CreateFontIndirect hook routine to avoid font aliasing that prevents reverse blitting working on palettized surfaces

HFONT WINAPI extCreateFontIndirect(const LOGFONT* lplf)
{
	LOGFONT lf;
	HFONT retHFont;
	OutTraceDW("CreateFontIndirect: h=%d w=%d face=\"%s\"\n", lplf->lfHeight, lplf->lfWidth, lplf->lfFaceName);
	memcpy((char *)&lf, (char *)lplf, sizeof(LOGFONT));
	lf.lfQuality=NONANTIALIASED_QUALITY;
	if(dxw.dwFlags1 & FIXTEXTOUT) {
		if(lf.lfHeight > 0) dxw.MapClient((int *)&lf.lfWidth, (int *)&lf.lfHeight);
		else {
			lf.lfHeight= -lf.lfHeight;
			dxw.MapClient((int *)&lf.lfWidth, (int *)&lf.lfHeight);
			lf.lfHeight= -lf.lfHeight;
		}
	}
	retHFont=((*pGDICreateFontIndirect)(&lf));
	if(retHFont)
		OutTraceDW("CreateFontIndirect: hfont=%x\n", retHFont);
	else
		OutTraceDW("CreateFontIndirect: error=%d at %d\n", GetLastError(), __LINE__);
	return retHFont;
}

BOOL WINAPI extSetDeviceGammaRamp(HDC hDC, LPVOID lpRamp)
{
	BOOL ret;
	OutTraceDW("SetDeviceGammaRamp: hdc=%x\n", hDC);
	if(dxw.dwFlags2 & DISABLEGAMMARAMP) {
		OutTraceDW("SetDeviceGammaRamp: SUPPRESSED\n");
		return TRUE;
	}
	ret=(*pGDISetDeviceGammaRamp)(hDC, lpRamp);
	if(!ret) OutTraceE("SetDeviceGammaRamp: ERROR err=%d\n", GetLastError());
	return ret;
}

BOOL WINAPI extGetDeviceGammaRamp(HDC hDC, LPVOID lpRamp)
{
	BOOL ret;
	OutTraceDW("GetDeviceGammaRamp: hdc=%x\n", hDC);
	ret=(*pGDIGetDeviceGammaRamp)(hDC, lpRamp);
	if(!ret) OutTraceE("GetDeviceGammaRamp: ERROR err=%d\n", GetLastError());
	return ret;
}

int WINAPI extGetClipBox(HDC hdc, LPRECT lprc)
{
	// v2.02.31: needed in "Imperialism II" to avoid blit clipping
	int ret;
	char *sRetCodes[4]={"ERROR", "NULLREGION", "SIMPLEREGION", "COMPLEXREGION"};
	OutTraceDW("GetClipBox: hdc=%x\n", hdc);
	ret=(*pGDIGetClipBox)(hdc, lprc);
	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc)) && (ret!=ERROR)){
		OutTraceDW("GetClipBox: scaling main win coordinates (%d,%d)-(%d,%d)\n",
			lprc->left, lprc->top, lprc->right, lprc->bottom);
		// current implementation is NOT accurate, since it always returns the whole
		// virtual desktop area as the current clipbox...!!!
		*lprc=dxw.GetScreenRect();
	}
	OutTraceDW("GetClipBox: ret=%x(%s)\n", ret, sRetCodes[ret]);
	return ret;
}

BOOL WINAPI extPolyline(HDC hdc, const POINT *lppt, int cPoints)
{
	BOOL ret;
	if(IsTraceDDRAW){
		int i;
		OutTrace("Polyline: hdc=%x cPoints=%d pt=", hdc, cPoints); 
		for(i=0; i<cPoints; i++) OutTrace("(%d,%d) ", lppt[i].x, lppt[i].y);
		OutTrace("\n");
	}
	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		int i;
		OutTrace("Polyline: fixed cPoints=%d pt=", cPoints); 
		for(i=0; i<cPoints; i++) {
			dxw.MapClient((LPPOINT)&lppt[i]);
			OutTrace("(%d,%d) ", lppt[i].x, lppt[i].y);
		}
		OutTrace("\n");
	}
	ret=(*pPolyline)(hdc, lppt, cPoints);
	if(!ret)OutTraceE("Polyline: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extLineTo(HDC hdc, int nXEnd, int nYEnd)
{
	BOOL ret;
	OutTraceDW("LineTo: hdc=%x pt=(%d,%d)\n", hdc, nXEnd, nYEnd);
	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		dxw.MapClient(&nXEnd, &nYEnd);
		OutTraceDW("LineTo: fixed pt=(%d,%d)\n", nXEnd, nYEnd);
	}
	ret=(*pLineTo)(hdc, nXEnd, nYEnd);
	if(!ret)OutTraceE("LineTo: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extArcTo(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect, int nXRadial1, int nYRadial1, int nXRadial2, int nYRadial2)
{
	BOOL ret;
	OutTraceDW("ArcTo: hdc=%x rect=(%d,%d)(%d,%d) radial=(%d,%d)(%d,%d)\n", 
		hdc, nLeftRect, nTopRect, nRightRect, nBottomRect, nXRadial1, nYRadial1, nXRadial2, nYRadial2);
	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		dxw.MapClient(&nLeftRect, &nTopRect, &nRightRect, &nBottomRect);
		dxw.MapClient(&nXRadial1, &nYRadial1, &nXRadial2, &nYRadial2);
		OutTraceDW("ArcTo: fixed rect=(%d,%d)(%d,%d) radial=(%d,%d)(%d,%d)\n", 
			nLeftRect, nTopRect, nRightRect, nBottomRect, nXRadial1, nYRadial1, nXRadial2, nYRadial2);
	}
	ret=(*pArcTo)(hdc, nLeftRect, nTopRect, nRightRect, nBottomRect, nXRadial1, nYRadial1, nXRadial2, nYRadial2);
	if(!ret)OutTraceE("ArcTo: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extMoveToEx(HDC hdc, int X, int Y, LPPOINT lpPoint)
{
	BOOL ret;
	OutTraceDW("MoveToEx: hdc=%x pt=(%d,%d)\n", hdc, X, Y);
	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		dxw.MapClient(&X, &Y);
		OutTraceDW("MoveToEx: fixed pt=(%d,%d)\n", X, Y);
	}
	ret=(*pMoveToEx)(hdc, X, Y, lpPoint);
	if(!ret)OutTraceE("MoveToEx: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extPolyDraw(HDC hdc, const POINT *lppt, const BYTE *lpbTypes, int cCount)
{
	BOOL ret;
	if(IsTraceDDRAW){
		int i;
		OutTrace("PolyDraw: hdc=%x cCount=%d pt=", hdc, cCount); 
		for(i=0; i<cCount; i++) OutTrace("(%x:%d,%d) ", lpbTypes[i], lppt[i].x, lppt[i].y);
		OutTrace("\n");
	}
	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		int i;
		OutTrace("PolyDraw: fixed cCount=%d pt=", cCount); 
		for(i=0; i<cCount; i++) {
			dxw.MapClient((LPPOINT)&lppt[i]);
			OutTrace("(%d,%d) ", lppt[i].x, lppt[i].y);
		}
		OutTrace("\n");
	}
	ret=(*pPolyDraw)(hdc, lppt, lpbTypes, cCount);
	if(!ret)OutTraceE("PolyDraw: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extPolylineTo(HDC hdc, const POINT *lppt, DWORD cCount)
{
	BOOL ret;
	if(IsTraceDDRAW){
		DWORD i;
		OutTrace("PolylineTo: hdc=%x cCount=%d pt=", hdc, cCount); 
		for(i=0; i<cCount; i++) OutTrace("(%d,%d) ", lppt[i].x, lppt[i].y);
		OutTrace("\n");
	}
	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		DWORD i;
		OutTrace("PolylineTo: fixed cCount=%d pt=", cCount); 
		for(i=0; i<cCount; i++) {
			dxw.MapClient((LPPOINT)&lppt[i]);
			OutTrace("(%d,%d) ", lppt[i].x, lppt[i].y);
		}
		OutTrace("\n");
	}
	ret=(*pPolylineTo)(hdc, lppt, cCount);
	if(!ret)OutTraceE("PolylineTo: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extPolyBezierTo(HDC hdc, const POINT *lppt, DWORD cCount)
{
	BOOL ret;
	if(IsTraceDDRAW){
		DWORD i;
		OutTrace("PolyBezierTo: hdc=%x cCount=%d pt=", hdc, cCount); 
		for(i=0; i<cCount; i++) OutTrace("(%d,%d) ", lppt[i].x, lppt[i].y);
		OutTrace("\n");
	}
	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		DWORD i;
		OutTrace("PolyBezierTo: fixed cCount=%d pt=", cCount); 
		for(i=0; i<cCount; i++) {
			dxw.MapClient((LPPOINT)&lppt[i]);
			OutTrace("(%d,%d) ", lppt[i].x, lppt[i].y);
		}
		OutTrace("\n");
	}
	ret=(*pPolyBezierTo)(hdc, lppt, cCount);
	if(!ret)OutTraceE("PolyBezierTo: ERROR ret=%x\n", ret); 
	return ret;
}

int WINAPI extStretchDIBits(HDC hdc, int XDest, int YDest, int nDestWidth, int nDestHeight, int XSrc, int YSrc, int nSrcWidth, int nSrcHeight, 
				  const VOID *lpBits, const BITMAPINFO *lpBitsInfo, UINT iUsage, DWORD dwRop)
{
	int ret;
	OutTraceDW("StretchDIBits: hdc=%x dest=(%d,%d)-(%d,%d) src=(%d,%d)-(%d,%d) rop=%x(%s)\n", 
		hdc, XDest, YDest, nDestWidth, nDestHeight, XSrc, YSrc, nSrcWidth, nSrcHeight, dwRop, ExplainROP(dwRop));

	if (dxw.IsFullScreen()){
		if(dxw.IsVirtual(hdc)){
			XDest+=dxw.VirtualOffsetX;
			YDest+=dxw.VirtualOffsetY;
			OutTraceDW("StretchDIBits: fixed dest=(%d,%d)-(%d,%d)\n", XDest, YDest, nDestWidth, nDestHeight);
		}
		else if (OBJ_DC == GetObjectType(hdc)){
			dxw.MapClient(&XDest, &YDest, &nDestWidth, &nDestHeight);
			OutTraceDW("StretchDIBits: fixed dest=(%d,%d)-(%d,%d)\n", XDest, YDest, nDestWidth, nDestHeight);
		}
	}

	ret=(*pStretchDIBits)(hdc, XDest, YDest, nDestWidth, nDestHeight, XSrc, YSrc, nSrcWidth, nSrcHeight, lpBits, lpBitsInfo, iUsage, dwRop);
	if(!ret || (ret==GDI_ERROR)) OutTraceE("StretchDIBits: ERROR ret=%x\n", ret); 
	return ret;
}

int WINAPI extSetDIBitsToDevice(HDC hdc, int XDest, int YDest, DWORD dwWidth, DWORD dwHeight, int XSrc, int YSrc, UINT uStartScan, UINT cScanLines, 
					const VOID *lpvBits, const BITMAPINFO *lpbmi, UINT fuColorUse)
{
	int ret;
	BITMAPINFOHEADER *bmi;
	OutTraceDW("SetDIBitsToDevice: hdc=%x dest=(%d,%d)-(%dx%d) src=(%d,%d) lines=(%d,%d)\n", 
		hdc, XDest, YDest, dwWidth, dwHeight, XSrc, YSrc, uStartScan, cScanLines);
	bmi=(BITMAPINFOHEADER *)&(lpbmi->bmiHeader);
	OutTraceDW("SetDIBitsToDevice: BitmapInfo dim=(%dx%d) Planes=%d BPP=%d Compression=%x SizeImage=%x\n",
		bmi->biWidth, bmi->biHeight, bmi->biPlanes, bmi->biBitCount, bmi->biCompression, bmi->biSizeImage);

	if (dxw.IsFullScreen() && dxw.IsVirtual(hdc)){
		int X, Y;
		X=XDest+dxw.VirtualOffsetX;
		Y=YDest+dxw.VirtualOffsetY;
		OutTraceDW("SetDIBitsToDevice: virtual pos=(%d,%d)+(%d+%d)=(%d,%d)\n",
			XDest, YDest, dxw.VirtualOffsetX, dxw.VirtualOffsetY, X, Y);
		ret=(*pSetDIBitsToDevice)(hdc, X, Y, dwWidth, dwHeight, XSrc, YSrc, uStartScan, cScanLines, lpvBits, lpbmi, fuColorUse);
		if(!ret || (ret==GDI_ERROR)) OutTraceE("SetDIBitsToDevice: ERROR ret=%x err=%d\n", ret, GetLastError()); 
		return ret;
	}
	//else
	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		DWORD OrigWidth, OrigHeight;
		int OrigXDest, OrigYDest;
		OrigWidth=dwWidth;
		OrigHeight=dwHeight;
		OrigXDest=XDest;
		OrigYDest=YDest;
		dxw.MapClient(&XDest, &YDest, (int *)&dwWidth, (int *)&dwHeight);
		OutTraceDW("SetDIBitsToDevice: fixed dest=(%d,%d)-(%d,%d)\n", XDest, YDest, dwWidth, dwHeight);
		HDC hTempDc;
		HBITMAP hbmPic;
		if(!(hTempDc=CreateCompatibleDC(hdc)))
			OutTraceE("CreateCompatibleDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
		// tricky part: CreateCompatibleBitmap is needed to set the dc size, but it has to be performed
		// against hdc to set for color depth, then selected (through SelectObject) against the temporary
		// dc to assign the needed size and color space to the temporary dc.
		if(!(hbmPic=CreateCompatibleBitmap(hdc, OrigWidth, OrigHeight)))
			OutTraceE("CreateCompatibleBitmap: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
		if(!SelectObject(hTempDc, hbmPic))
			OutTraceE("SelectObject: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
		if(!(*pSetDIBitsToDevice)(hTempDc, 0, 0, OrigWidth, OrigHeight, XSrc, YSrc, uStartScan, cScanLines, lpvBits, lpbmi, fuColorUse))
			OutTraceE("SetDIBitsToDevice: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
		if(!(ret=(*pGDIStretchBlt)(hdc, XDest, YDest, dwWidth, dwHeight, hTempDc, 0, 0, OrigWidth, OrigHeight, SRCCOPY)))
			OutTraceE("StretchBlt: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
		if(!(DeleteObject(hbmPic))) // v2.02.32 - avoid resource leakage
			OutTraceE("DeleteObject: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
		if(!(DeleteDC(hTempDc)))
			OutTraceE("DeleteDC: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	}
	else{
		ret=(*pSetDIBitsToDevice)(hdc, XDest, YDest, dwWidth, dwHeight, XSrc, YSrc, uStartScan, cScanLines, lpvBits, lpbmi, fuColorUse);
	}
	if(!ret || (ret==GDI_ERROR)) OutTraceE("SetDIBitsToDevice: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

//HBITMAP WINAPI extCreateCompatibleBitmap(HDC hdc, int nWidth, int nHeight)
//{
//	HBITMAP ret;
//	OutTraceDW("CreateCompatibleBitmap: hdc=%x size=(%d,%d)\n", 
//		hdc, nWidth, nHeight);
//
//	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
//		dxw.MapClient(&nWidth, &nHeight);
//		OutTraceDW("CreateCompatibleBitmap: fixed size=(%d,%d)\n", nWidth, nHeight);
//	}
//
//	ret=(*pCreateCompatibleBitmap)(hdc, nWidth, nHeight);
//	if(!ret) OutTraceE("CreateCompatibleBitmap: ERROR ret=%x err=%d\n", ret, GetLastError()); 
//	return ret;
//}

COLORREF WINAPI extSetPixel(HDC hdc, int X, int Y, COLORREF crColor)
{
	COLORREF ret;
	OutTraceDW("SetPixel: hdc=%x color=%x point=(%d,%d)\n", hdc, crColor, X, Y);

	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		dxw.MapClient(&X, &Y);
		OutTraceDW("SetPixel: fixed pos=(%d,%d)\n", X, Y);
	}

	ret=(*pSetPixel)(hdc, X, Y, crColor);
	// both 0x00000000 and 0xFFFFFFFF are legitimate colors and therefore valid return codes...
	//if(ret==GDI_ERROR) OutTraceE("SetPixel: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extEllipse(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect)
{
	int ret;
	OutTraceDW("Ellipse: hdc=%x rect=(%d,%d)-(%d,%d)\n", hdc, nLeftRect, nTopRect, nRightRect, nBottomRect);

	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		dxw.MapClient(&nLeftRect, &nTopRect, &nRightRect, &nBottomRect);
		OutTraceDW("Ellipse: fixed dest=(%d,%d)-(%d,%d)\n", nLeftRect, nTopRect, nRightRect, nBottomRect);
	}

	ret=(*pEllipse)(hdc, nLeftRect, nTopRect, nRightRect, nBottomRect);
	if(!ret) OutTraceE("Ellipse: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extPolygon(HDC hdc, const POINT *lpPoints, int cCount)
{
	BOOL ret;
	if(IsTraceDDRAW){
		int i;
		OutTrace("Polygon: hdc=%x cCount=%d pt=", hdc, cCount); 
		for(i=0; i<cCount; i++) OutTrace("(%d,%d) ", lpPoints[i].x, lpPoints[i].y);
		OutTrace("\n");
	}
	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		int i;
		OutTrace("Polygon: fixed cCount=%d pt=", cCount); 
		for(i=0; i<cCount; i++) {
			dxw.MapClient((LPPOINT)&lpPoints[i]);
			OutTrace("(%d,%d) ", lpPoints[i].x, lpPoints[i].y);
		}
		OutTrace("\n");
	}
	ret=(*pPolygon)(hdc, lpPoints, cCount);
	if(!ret)OutTraceE("Polygon: ERROR ret=%x\n", ret); 
	return ret;
}

BOOL WINAPI extArc(HDC hdc, int nLeftRect, int nTopRect, int nRightRect, int nBottomRect, int nXStartArc, int nYStartArc, int nXEndArc, int nYEndArc)
{
	int ret;
	OutTraceDW("Arc: hdc=%x rect=(%d,%d)-(%d,%d) start=(%d,%d) end=(%d,%d)\n", 
		hdc, nLeftRect, nTopRect, nRightRect, nBottomRect, nXStartArc, nYStartArc, nXEndArc, nYEndArc);

	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		dxw.MapClient(&nLeftRect, &nTopRect, &nRightRect, &nBottomRect);
		dxw.MapClient(&nXStartArc, &nYStartArc, &nXEndArc, &nYEndArc);
		OutTraceDW("Arc: fixed rect=(%d,%d)-(%d,%d) start=(%d,%d) end=(%d,%d)\n", 
			nLeftRect, nTopRect, nRightRect, nBottomRect, nXStartArc, nYStartArc, nXEndArc, nYEndArc);
	}

	ret=(*pArc)(hdc, nLeftRect, nTopRect, nRightRect, nBottomRect, nXStartArc, nYStartArc, nXEndArc, nYEndArc);
	if(!ret) OutTraceE("Arc: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

HRGN WINAPI extCreateEllipticRgn(int nLeftRect, int nTopRect, int nRightRect, int nBottomRect)
{
	HRGN ret;
	OutTraceDW("CreateEllipticRgn: rect=(%d,%d)-(%d,%d)\n", nLeftRect, nTopRect, nRightRect, nBottomRect);

	if (dxw.IsFullScreen()){
		dxw.MapClient(&nLeftRect, &nTopRect, &nRightRect, &nBottomRect);
		OutTraceDW("CreateEllipticRgn: fixed rect=(%d,%d)-(%d,%d)\n", nLeftRect, nTopRect, nRightRect, nBottomRect);
	}

	ret=(*pCreateEllipticRgn)(nLeftRect, nTopRect, nRightRect, nBottomRect);
	if(!ret) OutTraceE("CreateEllipticRgn: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

HRGN WINAPI extCreateEllipticRgnIndirect(const RECT *lprc)
{
	HRGN ret;
	OutTraceDW("CreateEllipticRgnIndirect: rect=(%d,%d)-(%d,%d)\n", lprc->left, lprc->top, lprc->right, lprc->bottom);

	if (dxw.IsFullScreen()){
		dxw.MapClient((RECT *)lprc);
		OutTraceDW("CreateEllipticRgnIndirect: fixed rect=(%d,%d)-(%d,%d)\n", lprc->left, lprc->top, lprc->right, lprc->bottom);
	}

	ret=(*pCreateEllipticRgnIndirect)(lprc);
	if(!ret) OutTraceE("CreateEllipticRgnIndirect: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

HRGN WINAPI extCreateRectRgn(int nLeftRect, int nTopRect, int nRightRect, int nBottomRect)
{
	HRGN ret;
	OutTraceDW("CreateRectRgn: rect=(%d,%d)-(%d,%d)\n", nLeftRect, nTopRect, nRightRect, nBottomRect);

	if (dxw.IsFullScreen()){
		dxw.MapClient(&nLeftRect, &nTopRect, &nRightRect, &nBottomRect);
		OutTraceDW("CreateRectRgn: fixed rect=(%d,%d)-(%d,%d)\n", nLeftRect, nTopRect, nRightRect, nBottomRect);
	}

	ret=(*pCreateRectRgn)(nLeftRect, nTopRect, nRightRect, nBottomRect);
	if(!ret) OutTraceE("CreateRectRgn: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

HRGN WINAPI extCreateRectRgnIndirect(const RECT *lprc)
{
	HRGN ret;
	OutTraceDW("CreateRectRgnIndirect: rect=(%d,%d)-(%d,%d)\n", lprc->left, lprc->top, lprc->right, lprc->bottom);

	if (dxw.IsFullScreen()){
		dxw.MapClient((RECT *)lprc);
		OutTraceDW("CreateRectRgnIndirect: fixed rect=(%d,%d)-(%d,%d)\n", lprc->left, lprc->top, lprc->right, lprc->bottom);
	}

	ret=(*pCreateRectRgnIndirect)(lprc);
	if(!ret) OutTraceE("CreateRectRgnIndirect: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

HRGN WINAPI extCreatePolygonRgn(const POINT *lpPoints, int cPoints, int fnPolyFillMode)
{
	HRGN ret;
	if(IsTraceDDRAW){
		int i;
		OutTrace("CreatePolygonRgn: PolyFillMode=%x cCount=%d pt=", fnPolyFillMode, cPoints); 
		for(i=0; i<cPoints; i++) OutTrace("(%d,%d) ", lpPoints[i].x, lpPoints[i].y);
		OutTrace("\n");
	}
	if (dxw.IsFullScreen()){
		int i;
		OutTrace("CreatePolygonRgn: fixed cCount=%d pt=", cPoints); 
		for(i=0; i<cPoints; i++) {
			dxw.MapClient((LPPOINT)&lpPoints[i]);
			OutTrace("(%d,%d) ", lpPoints[i].x, lpPoints[i].y);
		}
		OutTrace("\n");
	}
	ret=(*pCreatePolygonRgn)(lpPoints, cPoints, fnPolyFillMode);
	if(!ret)OutTraceE("CreatePolygonRgn: ERROR ret=%x\n", ret); 
	return ret;
}

int WINAPI extDrawText(HDC hdc, LPCTSTR lpchText, int nCount, LPRECT lpRect, UINT uFormat)
{
	int ret;
	OutTraceDW("DrawText: hdc=%x rect=(%d,%d)-(%d,%d) Format=%x Text=(%d)\"%s\"\n", 
		hdc, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, uFormat, nCount, lpchText);

	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		dxw.MapClient((RECT *)lpRect);
		OutTraceDW("DrawText: fixed rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}

	ret=(*pDrawText)(hdc, lpchText, nCount, lpRect, uFormat);
	if(!ret) OutTraceE("DrawText: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

int WINAPI extDrawTextEx(HDC hdc, LPTSTR lpchText, int nCount, LPRECT lpRect, UINT dwDTFormat, LPDRAWTEXTPARAMS lpDTParams)
{
	int ret;
	OutTraceDW("DrawTextEx: hdc=%x rect=(%d,%d)-(%d,%d) DTFormat=%x Text=(%d)\"%s\"\n", 
		hdc, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, dwDTFormat, nCount, lpchText);

	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		dxw.MapClient((RECT *)lpRect);
		OutTraceDW("DrawTextEx: fixed rect=(%d,%d)-(%d,%d)\n", lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	}

	ret=(*pDrawTextEx)(hdc, lpchText, nCount, lpRect, dwDTFormat, lpDTParams);
	if(!ret) OutTraceE("DrawTextEx: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extMaskBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc,
					 int nXSrc, int nYSrc, HBITMAP hbmMask, int xMask, int yMask, DWORD dwRop)
{
	BOOL ret;
	OutTraceDW("MaskBlt: hdcDest=%x pos=(%d,%d) size=(%dx%d) hdcSrc=%x pos=(%d,%d) hbmMask=%x Mask=(%d,%d) dwRop=%x\n",
		hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, hbmMask, xMask, yMask, dwRop);

	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdcDest))){
		dxw.MapClient(&nXDest, &nYDest, &nWidth, &nHeight);
		OutTraceDW("MaskBlt: fixed pos=(%d,%d) size=(%dx%d)\n", nXDest, nYDest, nWidth, nHeight);
	}

	ret=(*pMaskBlt)(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, hbmMask, xMask, yMask, dwRop);
	if(!ret) OutTraceE("MaskBlt: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extSetViewportOrgEx(HDC hdc, int X, int Y, LPPOINT lpPoint)
{
	BOOL ret;
	OutTraceDW("SetViewportOrgEx: hdc=%x pos=(%d,%d)\n", hdc, X, Y);

	if(dxw.IsVirtual(hdc)) {
		OutTraceDW("SetViewportOrgEx: virtual hdc\n");
		dxw.VirtualOffsetX = X;
		dxw.VirtualOffsetY = Y;
		return TRUE;
	}

	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		dxw.MapClient(&X, &Y);
		OutTraceDW("SetViewportOrgEx: fixed pos=(%d,%d)\n", X, Y);
	}

	ret=(*pSetViewportOrgEx)(hdc, X, Y, lpPoint);
	if(ret && lpPoint) {
		OutTraceDW("SetViewportOrgEx: previous ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
		if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
			dxw.UnmapClient(lpPoint);
			OutTraceDW("SetViewportOrgEx: fixed previous ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
		}
	}
	if(!ret) OutTraceE("SetViewportOrgEx: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extSetViewportExtEx(HDC hdc, int nXExtent, int nYExtent, LPSIZE lpSize)
{
	BOOL ret;
	OutTraceDW("SetViewportExtEx: hdc=%x ext=(%d,%d)\n", hdc, nXExtent, nYExtent);

	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		dxw.MapClient(&nXExtent, &nYExtent);
		OutTraceDW("SetViewportExtEx: fixed ext=(%d,%d)\n", nXExtent, nYExtent);
	}

	ret=(*pSetViewportExtEx)(hdc, nXExtent, nYExtent, lpSize);
	if(ret && lpSize) {
		OutTraceDW("SetViewportExtEx: previous ext=(%d,%d)\n", lpSize->cx, lpSize->cy);
		if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
			dxw.UnmapClient((LPPOINT)lpSize);
			OutTraceDW("SetViewportExtEx: fixed previous ext=(%d,%d)\n", lpSize->cx, lpSize->cy);
		}
	}

	if(!ret) OutTraceE("SetViewportExtEx: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extGetViewportOrgEx(HDC hdc, LPPOINT lpPoint)
{
	BOOL ret;
	OutTraceDW("GetViewportOrgEx: hdc=%x\n", hdc);

	if(dxw.IsVirtual(hdc)) {
		lpPoint->x = dxw.VirtualOffsetX;
		lpPoint->y = dxw.VirtualOffsetY;
		return TRUE;
	}

	ret=(*pGetViewportOrgEx)(hdc, lpPoint);
	if(ret) {
		OutTraceDW("GetViewportOrgEx: ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
		if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
			dxw.UnmapClient(lpPoint);
			OutTraceDW("GetViewportOrgEx: fixed ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
		}
	}

	if(!ret) OutTraceE("GetViewportOrgEx: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extGetWindowOrgEx(HDC hdc, LPPOINT lpPoint)
{
	BOOL ret;
	OutTraceDW("GetWindowOrgEx: hdc=%x\n", hdc);

	ret=(*pGetWindowOrgEx)(hdc, lpPoint);
	if(ret) {
		OutTraceDW("GetWindowOrgEx: ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
		if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
			dxw.UnmapClient(lpPoint);
			OutTraceDW("GetWindowOrgEx: fixed ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
		}
	}

	if(!ret) OutTraceE("GetWindowOrgEx: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extSetWindowOrgEx(HDC hdc, int X, int Y, LPPOINT lpPoint)
{
	BOOL ret;
	OutTraceDW("SetWindowOrgEx: hdc=%x pos=(%d,%d)\n", hdc, X, Y);

	if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
		dxw.MapClient(&X, &Y);
		OutTraceDW("SetWindowOrgEx: fixed pos=(%d,%d)\n", X, Y);
	}

	ret=(*pSetWindowOrgEx)(hdc, X, Y, lpPoint);
	if(ret && lpPoint) {
		OutTraceDW("SetWindowOrgEx: previous ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
		if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
			dxw.UnmapClient(lpPoint);
			OutTraceDW("SetWindowOrgEx: fixed previous ViewPort=(%d,%d)\n", lpPoint->x, lpPoint->y);
		}
	}
	if(!ret) OutTraceE("SetWindowOrgEx: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extGetCurrentPositionEx(HDC hdc, LPPOINT lpPoint)
{
	BOOL ret;
	OutTraceDW("GetCurrentPositionEx: hdc=%x\n", hdc);

	ret=(*pGetCurrentPositionEx)(hdc, lpPoint);
	if(ret) {
		OutTraceDW("GetCurrentPositionEx: pos=(%d,%d)\n", lpPoint->x, lpPoint->y);
		if (dxw.IsFullScreen() && (OBJ_DC == GetObjectType(hdc))){
			dxw.UnmapClient(lpPoint);
			OutTraceDW("GetCurrentPositionEx: fixed pos=(%d,%d)\n", lpPoint->x, lpPoint->y);
		}
	}
	if(!ret) OutTraceE("GetCurrentPositionEx: ERROR ret=%x err=%d\n", ret, GetLastError()); 
	return ret;
}

BOOL WINAPI extCreateScalableFontResourceA(DWORD fdwHidden, LPCTSTR lpszFontRes, LPCTSTR lpszFontFile, LPCTSTR lpszCurrentPath)
{
	BOOL res;
	OutTraceDW("CreateScalableFontResource: hidden=%d FontRes=\"%s\" FontFile=\"%s\" CurrentPath=\"%s\"\n",
		fdwHidden, lpszFontRes, lpszFontFile, lpszCurrentPath);
	if(dxw.dwFlags3 & FONTBYPASS) return TRUE;
	res=(*pCreateScalableFontResourceA)(fdwHidden, lpszFontRes, lpszFontFile, lpszCurrentPath);
	if(!res) OutTraceE("CreateScalableFontResource: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	return res;
}

BOOL WINAPI extCreateScalableFontResourceW(DWORD fdwHidden, LPCWSTR lpszFontRes, LPCWSTR lpszFontFile, LPCWSTR lpszCurrentPath)
{
	BOOL res;
	OutTraceDW("CreateScalableFontResource: hidden=%d FontRes=\"%ls\" FontFile=\"%ls\" CurrentPath=\"%ls\"\n",
		fdwHidden, lpszFontRes, lpszFontFile, lpszCurrentPath);
	if(dxw.dwFlags3 & FONTBYPASS) return TRUE;
	res=(*pCreateScalableFontResourceW)(fdwHidden, lpszFontRes, lpszFontFile, lpszCurrentPath);
	if(!res) OutTraceE("CreateScalableFontResource: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	return res;
}

int WINAPI extAddFontResourceA(LPCTSTR lpszFontFile)
{
	BOOL res;
	OutTraceDW("AddFontResource: FontFile=\"%s\"\n", lpszFontFile);
	if(dxw.dwFlags3 & FONTBYPASS) {
		OutTraceDW("AddFontResource: SUPPRESSED FontFile=\"%s\"\n", lpszFontFile);
		return TRUE;
	}
	res=(*pAddFontResourceA)(lpszFontFile);
	if(!res) OutTraceE("AddFontResource: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	return res;
}

int WINAPI extAddFontResourceW(LPCWSTR lpszFontFile)
{
	BOOL res;
	OutTraceDW("AddFontResource: FontFile=\"%ls\"\n", lpszFontFile);
	if(dxw.dwFlags3 & FONTBYPASS) {
		OutTraceDW("AddFontResource: SUPPRESSED FontFile=\"%ls\"\n", lpszFontFile);
		return TRUE;
	}
	res=(*pAddFontResourceW)(lpszFontFile);
	if(!res) OutTraceE("AddFontResource: ERROR err=%d at=%d\n", GetLastError(), __LINE__);
	return res;
}

UINT WINAPI extSetSystemPaletteUse(HDC hdc, UINT uUsage)
{
	//BOOL res;
	OutTraceDW("SetSystemPaletteUse: hdc=%x Usage=%x(%s)\n", hdc, uUsage, ExplainPaletteUse(uUsage));
	return SYSPAL_NOSTATIC256;
}

#if 0
int WINAPI extSetMapMode(HDC hdc, int fnMapMode)
{
	OutTraceDW("SetMapMode: hdc=%x MapMode=%d\n", hdc, fnMapMode);
	return TRUE;
}
#endif

#if 0
// to map:
// GetCurrentPositionEx
// GetViewportExtEx
// DPtoLP
// GetWindowOrgEx
// LPtoDP
// OffsetViewportOrgEx
// OffsetWindowOrgEx
// TransparentBlt
// to do: eliminate FIXTEXTOUT handling
// GetDCEx

BOOL SetTextJustification(
  _In_  HDC hdc,
  _In_  int nBreakExtra, <----
  _In_  int nBreakCount
);
#endif