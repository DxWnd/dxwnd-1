/* ===================================================================

u32lib.h: contains typedef definition and extern pointer declarations
for all GDI32.dll and user32.dll functions hooked by DxWnd

====================================================================*/

typedef BOOL (WINAPI *GetCursorPos_Type)(LPPOINT);
typedef BOOL (WINAPI *SetCursorPos_Type)(int, int);
typedef HCURSOR (WINAPI *SetCursor_Type)(HCURSOR);
typedef BOOL (WINAPI *ClipCursor_Type)(const RECT *);
typedef BOOL (WINAPI *TextOut_Type)(HDC, int, int, LPCTSTR, int);
typedef BOOL (WINAPI *Rectangle_Type)(HDC, int, int, int, int);
typedef COLORREF (WINAPI *SetBkColor_Type)(HDC, COLORREF);
typedef COLORREF (WINAPI *SetTextColor_Type)(HDC, COLORREF);
typedef HFONT (WINAPI *CreateFont_Type)(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCTSTR);
typedef HFONT (WINAPI *CreateFontIndirect_Type)(const LOGFONT*);
typedef HDC  (WINAPI *GDIGetDC_Type)(HWND);
typedef int  (WINAPI *GDIReleaseDC_Type)(HWND, HDC);
typedef BOOL (WINAPI *ColorCorrectPalette_Type)(HDC, HPALETTE, DWORD, DWORD);
typedef HWND (WINAPI *CreateWindowExA_Type)(DWORD, LPCTSTR, LPCTSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
typedef HWND (WINAPI *GetDesktopWindow_Type)(void);
typedef BOOL (WINAPI *ShowWindow_Type)(HWND, int);
typedef int  (WINAPI *FillRect_Type)(HDC, const RECT *, HBRUSH);
typedef int  (WINAPI *DrawFocusRect_Type)(HDC, const RECT *);
typedef int  (WINAPI *GetSystemMetrics_Type)(int);
typedef int  (WINAPI *GetDeviceCaps_Type)(HDC, int);
typedef int  (WINAPI *GDIGetDisplayMode_Type)(HDC, int);
typedef BOOL (WINAPI *ShowWindow_Type)(HWND, int);
typedef LONG (WINAPI *SetWindowLong_Type)(HWND, int, LONG);
typedef LONG (WINAPI *GetWindowLong_Type)(HWND, int);
typedef BOOL (WINAPI *SetWindowPos_Type)(HWND, HWND, int, int, int, int, UINT);
typedef HDWP (WINAPI *DeferWindowPos_Type)(HDWP, HWND, HWND, int, int, int, int, UINT);
typedef LRESULT (WINAPI *CallWindowProc_Type)(WNDPROC, HWND, UINT, WPARAM, LPARAM);
typedef BOOL (WINAPI *ScaleWindowExtEx_Type)(HDC, int, int, int, int, LPSIZE);
typedef int (WINAPI *GDIChoosePixelFormat_Type)(HDC, const PIXELFORMATDESCRIPTOR *);
typedef int (WINAPI *GDIGetPixelFormat_Type)(HDC hdc);
typedef BOOL (WINAPI *GDISetPixelFormat_Type)(HDC, int, const PIXELFORMATDESCRIPTOR *);
//typedef HRGN (WINAPI *CreateRectRgn_Type)(int, int, int, int);
typedef LRESULT (WINAPI *DefWindowProc_Type)(HWND, UINT, WPARAM, LPARAM);
typedef LONG (WINAPI *ChangeDisplaySettings_Type)(DEVMODE *, DWORD);
typedef LONG (WINAPI *ChangeDisplaySettingsEx_Type)(LPCTSTR, DEVMODE *, HWND, DWORD, LPVOID);
typedef LONG (WINAPI *EnumDisplaySettings_Type)(LPCTSTR, DWORD, LPDEVMODEA);
typedef BOOL (WINAPI *SetWindowPlacement_Type)(const WINDOWPLACEMENT*);
typedef DWORD (WINAPI *SetLayeredWindowAttributes_Type)(HWND, DWORD, BYTE, DWORD);
typedef HDC (WINAPI *CreateCompatibleDC_Type)(HDC);
typedef BOOL (WINAPI *DeleteDC_Type)(HDC);
typedef BOOL (WINAPI *BitBlt_Type)(HDC, int, int, int, int, HDC, int, int, DWORD);
typedef BOOL (WINAPI *PatBlt_Type)(HDC, int, int, int, int, DWORD);
typedef BOOL (WINAPI *StretchBlt_Type)(HDC, int, int, int, int, HDC, int, int, int, int, DWORD);
typedef HDC (WINAPI *CreateDC_Type)(LPCSTR, LPCSTR, LPCSTR, const DEVMODE *);
typedef int (WINAPI *SaveDC_Type)(HDC);
typedef BOOL (WINAPI *RestoreDC_Type)(HDC, int);
typedef HDC (WINAPI *BeginPaint_Type)(HWND, LPPAINTSTRUCT);
typedef BOOL (WINAPI *EndPaint_Type)(HWND, const PAINTSTRUCT *);

typedef HMODULE (WINAPI *LoadLibraryA_Type)(LPCTSTR);
typedef HMODULE (WINAPI *LoadLibraryExA_Type)(LPCTSTR, HANDLE, DWORD);
typedef BOOL (WINAPI *GetClipCursor_Type)(LPRECT);
typedef ATOM (WINAPI *RegisterClassExA_Type)(WNDCLASSEX *);
typedef BOOL (WINAPI *ScreenToClient_Type)(HWND, LPPOINT);
typedef BOOL (WINAPI *ClientToScreen_Type)(HWND, LPPOINT);
typedef BOOL (WINAPI *GetClientRect_Type)(HWND, LPRECT);
typedef BOOL (WINAPI *GetWindowRect_Type)(HWND, LPRECT);
typedef int  (WINAPI *MapWindowPoints_Type)(HWND, HWND, LPPOINT, UINT);

typedef BOOL (WINAPI *PeekMessage_Type)(LPMSG, HWND, UINT, UINT, UINT);
typedef BOOL (WINAPI *GetMessage_Type)(LPMSG, HWND, UINT, UINT);
typedef LRESULT (WINAPI *DispatchMessage_Type)(LPMSG);
typedef FARPROC (WINAPI *GetProcAddress_Type)(HMODULE, LPCSTR);
//typedef BOOL (WINAPI *GetUpdateRect_Type)(HWND, LPRECT, BOOL);
typedef HWND (WINAPI *CreateDialogIndirectParam_Type)(HINSTANCE, LPCDLGTEMPLATE, HWND, DLGPROC, LPARAM);
typedef HWND (WINAPI *CreateDialogParam_Type)(HINSTANCE, LPCTSTR, HWND, DLGPROC, LPARAM);
typedef BOOL (WINAPI *InvalidateRect_Type)(HWND, CONST RECT *, BOOL);
typedef BOOL (WINAPI *InvalidateRgn_Type)(HWND, HRGN, BOOL);
typedef HRESULT (STDAPICALLTYPE *CoCreateInstance_Type)(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv);

typedef HPALETTE (WINAPI *SelectPalette_Type)(HDC, HPALETTE, BOOL);
typedef UINT (WINAPI *RealizePalette_Type)(HDC);
typedef HDC (WINAPI *BeginPaint_Type)(HWND, LPPAINTSTRUCT);
typedef HPALETTE (WINAPI *GDICreatePalette_Type)(CONST LOGPALETTE *);
typedef BOOL (WINAPI *GetSystemPaletteEntries_Type)(HDC, UINT, UINT, LPPALETTEENTRY);
typedef BOOL (WINAPI *MoveWindow_Type)(HWND, int, int, int, int, BOOL);

typedef LPTOP_LEVEL_EXCEPTION_FILTER (WINAPI *SetUnhandledExceptionFilter_Type)(LPTOP_LEVEL_EXCEPTION_FILTER);

typedef BOOL (WINAPI *GetDiskFreeSpaceA_Type)(LPCSTR, LPDWORD, LPDWORD, LPDWORD, LPDWORD);
typedef BOOL (WINAPI *SetDeviceGammaRamp_Type)(HDC, LPVOID);
typedef BOOL (WINAPI *GetDeviceGammaRamp_Type)(HDC, LPVOID);
typedef LRESULT (WINAPI *SendMessage_Type)(HWND, UINT, WPARAM, LPARAM);

extern GetCursorPos_Type pGetCursorPos;
extern SetCursorPos_Type pSetCursorPos;
extern ClipCursor_Type pClipCursor;
extern TextOut_Type pTextOutA;
extern FillRect_Type pFillRect;
extern DrawFocusRect_Type pDrawFocusRect;
extern Rectangle_Type pRectangle;
extern SetTextColor_Type pSetTextColor;
extern GetSystemMetrics_Type pGetSystemMetrics;
extern CreateFont_Type pCreateFont;
extern CreateFontIndirect_Type pCreateFontIndirect;
extern SetBkColor_Type pSetBkColor;
extern GetDeviceCaps_Type pGetDeviceCaps;
extern GDIGetDisplayMode_Type pGDIGetDisplayMode;
extern ColorCorrectPalette_Type pColorCorrectPalette;
extern CreateWindowExA_Type pCreateWindowExA;
extern GetDesktopWindow_Type pGetDesktopWindow;
extern ShowWindow_Type pShowWindow;
extern SetWindowLong_Type pSetWindowLong;
extern GetWindowLong_Type pGetWindowLong;
extern SetWindowPos_Type pSetWindowPos;
extern DeferWindowPos_Type pDeferWindowPos;
extern CallWindowProc_Type pCallWindowProc;
extern ScaleWindowExtEx_Type pScaleWindowExtEx;
extern GDIChoosePixelFormat_Type pGDIChoosePixelFormat;
extern GDIGetPixelFormat_Type pGDIGetPixelFormat;
extern GDISetPixelFormat_Type pGDISetPixelFormat;
//extern CreateRectRgn_Type pCreateRectRgn;
extern DefWindowProc_Type pDefWindowProc;
extern ChangeDisplaySettings_Type pChangeDisplaySettings;
extern ChangeDisplaySettingsEx_Type pChangeDisplaySettingsEx;
extern EnumDisplaySettings_Type pEnumDisplaySettings;
extern SetWindowPlacement_Type pSetWindowPlacement;
extern CreateCompatibleDC_Type pCreateCompatibleDC;
extern DeleteDC_Type pDeleteDC;
extern BitBlt_Type pBitBlt;
extern PatBlt_Type pPatBlt;
extern StretchBlt_Type pStretchBlt;
extern CreateDC_Type pCreateDC;
extern SaveDC_Type pGDISaveDC;
extern RestoreDC_Type pGDIRestoreDC;
extern MapWindowPoints_Type pMapWindowPoints;

extern LoadLibraryA_Type pLoadLibraryA;
extern LoadLibraryExA_Type pLoadLibraryExA;
extern GetClipCursor_Type pGetClipCursor;
extern RegisterClassExA_Type pRegisterClassExA;
extern ScreenToClient_Type pScreenToClient;
extern ClientToScreen_Type pClientToScreen;
extern GetClientRect_Type pGetClientRect;
extern GetWindowRect_Type pGetWindowRect;
extern PeekMessage_Type pPeekMessage;
extern GetMessage_Type pGetMessage;
extern DispatchMessage_Type pDispatchMessage;
extern GetProcAddress_Type pGetProcAddress;
//extern GetUpdateRect_Type pGetUpdateRect;
extern CreateDialogParam_Type pCreateDialogParam;
extern CreateDialogIndirectParam_Type pCreateDialogIndirectParam;
extern BeginPaint_Type pBeginPaint;
extern EndPaint_Type pEndPaint;
extern InvalidateRect_Type pInvalidateRect;
extern CoCreateInstance_Type pCoCreateInstance;
extern GDICreatePalette_Type pGDICreatePalette;
extern SelectPalette_Type pSelectPalette;
extern RealizePalette_Type pRealizePalette;
extern GetSystemPaletteEntries_Type pGetSystemPaletteEntries;
extern MoveWindow_Type pMoveWindow;
extern SetUnhandledExceptionFilter_Type pSetUnhandledExceptionFilter;
extern GetDiskFreeSpaceA_Type pGetDiskFreeSpaceA;
extern SetDeviceGammaRamp_Type pSetDeviceGammaRamp;
extern GetDeviceGammaRamp_Type pGetDeviceGammaRamp;
extern SendMessage_Type pSendMessage;

