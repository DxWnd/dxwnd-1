#define _CRT_SECURE_NO_WARNINGS
#define INITGUID
//#define FULLHEXDUMP

#include <windows.h>
#include <ddraw.h>
#include "dxwnd.h"
#include "dxhook.h"
#include "ddrawi.h"
#include "dxwcore.hpp"
#include "stdio.h" 
#include "hddraw.h"
#include "dxhelper.h"
#include "syslibs.h"

extern GetDC_Type pGetDCMethod();
extern ReleaseDC_Type pReleaseDCMethod();
extern Unlock1_Type pUnlockMethod(int);

void BlitToWindow(HWND w, LPDIRECTDRAWSURFACE s)
{
	HDC shdc, thdc;
	RECT client;
	HRESULT res;
	BOOL ret;

	if(!s) return; // for surface color fill 
	res=(*pGetDCMethod())(s, &shdc);
	if(res) {
		OutTrace("ddraw GetDC error lpdds=%x res=%x(%s)\n", s, res, ExplainDDError(res));
		return;
	}
	thdc=(*pGDIGetDC)(w);
	if(!thdc) {
		OutTrace("GDI GetDC error=%d\n", GetLastError());
		return;
	}
	client = dxw.MapClientRect(NULL);
	if(dxw.dwFlags5 & CENTERTOWIN){
		int x, y;
		x = (client.left + client.right - dxw.GetScreenWidth()) >> 1; // right-shift 1 bit means divide by 2!
		y = (client.top + client.bottom - dxw.GetScreenHeight()) >> 1;
		ret=(*pGDIBitBlt)(thdc, x, y, dxw.GetScreenWidth(), dxw.GetScreenHeight(), shdc, 0, 0, SRCCOPY);
		if(!ret) OutTrace("BitBlt error=%d\n", GetLastError());
	}
	else{
		if(dxw.dwFlags5 & BILINEARFILTER) {
			ret=SetStretchBltMode(thdc, HALFTONE);
			if((!ret) || (ret==ERROR_INVALID_PARAMETER)) OutTrace("GDI SetStretchBltMode error=%d\n", GetLastError());
		}
		ret=(*pGDIStretchBlt)(thdc, 
			client.left, client.top, client.right-client.left, client.bottom-client.top, 
			shdc, 0, 0, dxw.GetScreenWidth(), dxw.GetScreenHeight(), SRCCOPY);
		if(!ret) OutTrace("GDI StretchBlt error=%d\n", GetLastError());
	}
	dxw.ShowOverlay(thdc);
	res=(*pReleaseDCMethod())(s, shdc);
	if(res) OutTrace("ddraw ReleaseDC error lpdds=%x res=%x(%s)\n", s, res, ExplainDDError(res));
	ret=(*pGDIReleaseDC)(w, thdc);
	if(!ret) OutTrace("GDI ReleaseDC error=%d\n", GetLastError());
}