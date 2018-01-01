
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>

#include "trace.h"

static char g_szAppName[] = "Example4";
static char g_szAppTitle[] = "Example 4";

#define	DIB_DEPTH   32
#define	DIB_WIDTH   320
#define	DIB_HEIGHT  240

BYTE* g_pBits = NULL;
LPBITMAPINFO g_lpBmi = NULL;

LPBITMAPINFO CreateDIB(int cx, int cy, int iBpp, BYTE* &pBits)
{
	LPBITMAPINFO lpBmi;
	int iBmiSize;
	int iSurfaceSize;

	// Calculate the size of the bitmap info header.
	switch(iBpp) {
	case 8 :		// 8 bpp
		iBmiSize = sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 256;
		iSurfaceSize = cx * cy * sizeof(BYTE);
		break;
	
	case 15 :	// 15/16 bpp
	case 16 :
		iBmiSize = sizeof(BITMAPINFO) + sizeof(DWORD) * 4;
		iSurfaceSize = cx * cy * sizeof(WORD);
		break;

	case 24 :	// 24 bpp
		iBmiSize = sizeof(BITMAPINFO);
		iSurfaceSize = cx * cy * (sizeof(BYTE) * 3);
		break;

	case 32 :	// 32 bpp
		iBmiSize = sizeof(BITMAPINFO) + sizeof(DWORD) * 4;
		iSurfaceSize = cx * cy * sizeof(DWORD);
		break;
	}

	// Allocate memory for the bitmap info header.
	if((lpBmi = (LPBITMAPINFO)malloc(iBmiSize)) == NULL){
		TRACE("Error allocating BitmapInfo!\n");
		return NULL;
	}

	ZeroMemory(lpBmi, iBmiSize);

	// Allocate memory for the DIB surface.
	if((pBits = (BYTE*)malloc(iSurfaceSize)) == NULL) {
		TRACE("Error allocating memory for bitmap bits\n");
		return NULL;
	}

	ZeroMemory(pBits, iSurfaceSize);

	// Initialize bitmap info header
	lpBmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpBmi->bmiHeader.biWidth = cx;
	lpBmi->bmiHeader.biHeight = -(signed)cy;		// <-- NEGATIVE MEANS TOP DOWN!!!
	lpBmi->bmiHeader.biPlanes = 1;
	lpBmi->bmiHeader.biSizeImage = 0;
	lpBmi->bmiHeader.biXPelsPerMeter = 0;
	lpBmi->bmiHeader.biYPelsPerMeter = 0;
	lpBmi->bmiHeader.biClrUsed = 0;
	lpBmi->bmiHeader.biClrImportant = 0;
	lpBmi->bmiHeader.biCompression = BI_RGB;

	// After initializing the bitmap info header we need to store some
	// more information depending on the bpp of the bitmap.
	switch(iBpp) {
	case 8:
		{
			// For the 8bpp DIB we will create a simple grayscale palette.
			for(int i = 0; i < 256; i++) {
				lpBmi->bmiColors[i].rgbRed			= (BYTE)i;
				lpBmi->bmiColors[i].rgbGreen		= (BYTE)i;
				lpBmi->bmiColors[i].rgbBlue		= (BYTE)i;
				lpBmi->bmiColors[i].rgbReserved	= (BYTE)0;
			}

			//-- Set the bpp for this DIB to 8bpp.
			lpBmi->bmiHeader.biBitCount = 8;
		}
		break;
	
	case 15:
		{
			// This is where we will tell the DIB what bits represent what
			// data. This may look confusing at first but the representation
			// of the RGB data can be different on different devices. For
			// example you can have for Hicolor a 565 format. Meaning 5 bits
			// for red, 6 bits for green and 5 bits for blue or better stated
			// like RGB. But, the pixel data can also be the other way around,
			// for example BGR meaning, 5 bits for blue, 6 bits for green and
			// 5 bits for red. This piece of information will tell the bitmap
			// info header how the pixel data will be stored. In this case in
			// RGB format in 555 because this is a 15bpp DIB so the highest
			// bit (bit 15) will not be used.
			DWORD *pBmi = (DWORD*)lpBmi->bmiColors;

			pBmi[0] = 0x00007C00;	// Red mask
			pBmi[1] = 0x000003E0;	// Green mask
			pBmi[2] = 0x0000001F;	// Blue mask
			pBmi[3] = 0x00000000;	// Not used

			// 15bpp DIB also use 16 bits to store a pixel.
			lpBmi->bmiHeader.biBitCount = 16;
			lpBmi->bmiHeader.biCompression |= BI_BITFIELDS;
		}
		break;

	case 16:
		{
			// Take a look at the remarks written by 15bpp. For this format
			// it's the same thing, except in this case the mask's will be
			// different because our format will be 565 (RGB).
			DWORD *pBmi = (DWORD*)lpBmi->bmiColors;

			pBmi[0] = 0x0000F800;	// Red mask
			pBmi[1] = 0x000007E0;	// Green mask
			pBmi[2] = 0x0000001F;	// Blue mask
			pBmi[3] = 0x00000000;	// Not used

			lpBmi->bmiHeader.biBitCount = 16;
			lpBmi->bmiHeader.biCompression |= BI_BITFIELDS;
		}
		break;

	case 24:
		{
			// This is a 1:1 situation. There is no need to set any extra
			// information.
			lpBmi->bmiHeader.biBitCount = 24;
		}
		break;

	case 32:
		{
			// This may speak for it's self. In this case where using 32bpp.
			// The format will be ARGB. the Alpha (A) portion of the format
			// will not be used. The other mask's tell us where the bytes
			// for the R, G and B data will be stored in the DWORD.
			DWORD *pBmi = (DWORD*)lpBmi->bmiColors;

			pBmi[0] = 0x00FF0000;	// Red mask
			pBmi[1] = 0x0000FF00;	// Green mask
			pBmi[2] = 0x000000FF;	// Blue mask
			pBmi[3] = 0x00000000;	// Not used (Alpha?)

			lpBmi->bmiHeader.biBitCount = 32;
			lpBmi->bmiHeader.biCompression |= BI_BITFIELDS;
		}
		break;
	}

	return lpBmi;
}

void PutPixel(int x, int y, BYTE r, BYTE g, BYTE b, LPBITMAPINFO lpBmi, void* pBits)
{
	int iOffset = lpBmi->bmiHeader.biWidth * y + x;

	switch(lpBmi->bmiHeader.biBitCount) {
	case 8:
		{
			// Cast void* to a BYTE* and write pixel to surface
			BYTE* p = (BYTE*)pBits;
			p[iOffset] = (BYTE)r;
		}
		break;

	case 15:
		{
			// Cast void* to a WORD* and write pixel to surface
			WORD* p = (WORD*)pBits;
			p[iOffset] = (WORD)(((r & 0xF8) << 7) | ((g & 0xF8) << 2) | b >> 3);
		}
		break;

	case 16:
		{
			// Cast void* to a WORD* and write pixel to surface
			WORD* p = (WORD*)pBits;
			p[iOffset] = (WORD)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | b >> 3);
		}
		break;

	case 24:
		{
			// Cast void* to a BYTE* and write pixel to surface
			BYTE* p = (BYTE*)pBits;
			p[iOffset * 3 + 0] = r;
			p[iOffset * 3 + 1] = g;
			p[iOffset * 3 + 2] = b;
		}
		break;

	case 32:
		{
			// Cast void* to a DWORD* and write pixel to surface
			DWORD* p = (DWORD*)pBits;
			p[iOffset] = (DWORD)((r << 16) | (g << 8) | b);
		}
		break;
	}
}

void Render(HWND hWnd, LPBITMAPINFO lpBmi, void* pBits)
{
	// Get a random x- and y-coordinate and plot pixel
	int x = rand() % lpBmi->bmiHeader.biWidth;
	int y = rand() % lpBmi->bmiHeader.biHeight;

	PutPixel(x, y, rand() % 256, rand() % 256, rand() % 256, lpBmi, pBits);

	// Make sure OnPaint gets called.
	InvalidateRect(hWnd, NULL, FALSE);
}

BOOL OnCreate(HWND hWnd, CREATESTRUCT FAR* lpCreateStruct)
{
	// Create a new DIB
	if((g_lpBmi = CreateDIB(DIB_WIDTH, DIB_HEIGHT, DIB_DEPTH, g_pBits)) == NULL) {
		return FALSE;
	}

	return TRUE;
}

void OnDestroy(HWND hWnd)
{
	if(g_pBits) {
		free(g_pBits);
	}

	if(g_lpBmi) {
		free(g_lpBmi);
	}

	PostQuitMessage(0);
}

void OnPaint(HWND hWnd)
{
	static PAINTSTRUCT ps;
	static HDC hDC;

	hDC = BeginPaint(hWnd, &ps);

	// Use StretchDIBits to display the DIB in the window.
	RECT rc;
	GetClientRect(hWnd, &rc);
	StretchDIBits(hDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top, 0, 0, DIB_WIDTH, DIB_HEIGHT, (BYTE*)g_pBits, g_lpBmi, DIB_RGB_COLORS, SRCCOPY);

	EndPaint(hWnd, &ps);
}

BOOL OnEraseBkgnd(HWND hWnd, HDC hdc)
{
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg) {
		HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(hWnd, WM_PAINT, OnPaint);
		HANDLE_MSG(hWnd, WM_ERASEBKGND, OnEraseBkgnd);
	}

	return DefWindowProc(hWnd, iMsg, wParam, lParam);	
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	MSG msg;
	HWND hWnd;
	WNDCLASSEX wc;

	wc.cbSize = sizeof(wc);
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_szAppName;
	wc.hIconSm = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(
		NULL,
		g_szAppName,
		g_szAppTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);

	while(1) {
		if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			if(!GetMessage(&msg, NULL, 0, 0))
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		if(TRUE) {
			Render(hWnd, g_lpBmi, g_pBits);
		}
		else {
			WaitMessage();
		}
	}

	return msg.wParam;
}
