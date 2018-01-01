
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>

#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "trace.h"
#include "resource\resource.h"

static char g_szAppName[] = "Example2";
static char g_szAppTitle[] = "Example 2";

HBITMAP g_hBitmap;

void SaveBitmap(HBITMAP hBitmap, LPCTSTR lpszFilename)
{
	RGBQUAD rgbPalette[256];
	DIBSECTION ds;

	// Get the DIB section information of the bitmap.
	GetObject(hBitmap, sizeof(DIBSECTION), &ds);

	// Select the bitmap into a DC so we can retrieve
	// it's palette and calculate the number of colors
	// used.
	HDC hDC = CreateCompatibleDC(NULL);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDC, (HBITMAP)hBitmap);
	int iUsedColors = GetDIBColorTable(hDC, 0, ds.dsBmih.biClrUsed, &rgbPalette[0]);
	SelectObject(hDC, hOldBitmap);
	DeleteDC(hDC);

	// Create a bitmap file header and load it up with
	// the right information about our bitmap.
	BITMAPFILEHEADER bh;
	bh.bfSize = sizeof(BITMAPFILEHEADER);
	bh.bfType = ((WORD) ('M' << 8) | 'B');
	bh.bfOffBits = (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * iUsedColors));
	bh.bfReserved1 = 0;
	bh.bfReserved2 = 0;

	// Write the bitmap data to disk.
	int hFile = _open(lpszFilename, _O_CREAT | _O_BINARY | _O_WRONLY);
	_write(hFile, &bh, sizeof(BITMAPFILEHEADER));
	_write(hFile, &ds.dsBmih, sizeof(BITMAPINFOHEADER));
	_write(hFile, &rgbPalette, sizeof(RGBQUAD) * iUsedColors);
	_write(hFile, ds.dsBm.bmBits, ds.dsBm.bmWidth * ds.dsBm.bmHeight * (ds.dsBm.bmBitsPixel / 8));
	_close(hFile);
}

BOOL OnCreate(HWND hWnd, CREATESTRUCT FAR* lpCreateStruct)
{
	// Load a bitmap from the resource and store the
	// handle in 'g_hBitmap' which is a global handle.
	g_hBitmap = (HBITMAP)LoadImage(lpCreateStruct->hInstance, MAKEINTRESOURCE(IDB_BITMAP), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
 
	// If 'g_hBitmap' equals NULL then the bitmap failed
	// to load. Return FALSE to quit the program.
	if(!g_hBitmap) {
		return FALSE;
	}

	return TRUE;
}

void OnDestroy(HWND hWnd)
{
	// If the handle is valid then destroy the bitmap.
	if(g_hBitmap) {
		DeleteObject(g_hBitmap);
	}

	PostQuitMessage(0);
}

void OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hDC, hBitmapDC;
	HBITMAP hOldBitmap;
	BITMAP bm;

	hDC = BeginPaint(hWnd, &ps);

	// Create a DC for the bitmap and select it
	// into the created DC.
	hBitmapDC = CreateCompatibleDC(hDC);
	hOldBitmap = (HBITMAP)SelectObject(hBitmapDC, (HBITMAP)g_hBitmap);

	// Display the bitmap into window
	GetObject((HBITMAP)g_hBitmap, sizeof(BITMAP), &bm);	
	BitBlt(hDC, 0, 0, bm.bmWidth, bm.bmHeight, hBitmapDC, 0, 0, SRCCOPY);

	// Delete the temporary bitmap DC
	SelectObject(hBitmapDC, hOldBitmap);
	DeleteDC(hBitmapDC);

	EndPaint(hWnd, &ps);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg) {
		HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(hWnd, WM_PAINT, OnPaint);
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

	while(GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}
