
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>

#include "trace.h"
#include "resource\resource.h"

static char g_szAppName[] = "Example1";
static char g_szAppTitle[] = "Example 1";

HBITMAP g_hBitmap;

BOOL OnCreate(HWND hWnd, CREATESTRUCT FAR* lpCreateStruct)
{
	// Load a bitmap from the resource and store the
	// handle in 'g_hBitmap' which is a global handle.
	g_hBitmap = LoadBitmap(lpCreateStruct->hInstance, MAKEINTRESOURCE(IDB_BITMAP));

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
