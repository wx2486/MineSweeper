// MineSweeperInterface.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MineSweeper In XP.h"

// my include
#include "uxtheme.h"

#define MAX_LOADSTRING 100

// my definetions
#define BUTTON_TITLE "¿ªÊ¼É¨À×"
#define BUTTON_TITLE_T "Í£Ö¹É¨À×"
#define STATIC_TITLE "Welcome to use this little tool."
#define MY_WINDOW_WIDTH 270
#define MY_WINDOW_HEIGHT 200
#define BUTTON_X 100
#define BUTTON_Y 100
#define BUTTON_CX 70
#define BUTTON_CY 20
#define EDIT_WINDOW_X 20
#define EDIT_WINDOW_Y 10
#define EDIT_WINDOW_CX 230
#define EDIT_WINDOW_CY 70
#define REALTIME_DISTANCE 20

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int, HWND &);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	HWND myWnd, buttonWnd, staticWnd;

	// my code

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MINESWEEPERINXP, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow, myWnd))
	{
		return FALSE;
	}

	buttonWnd = CreateWindow(_T("BUTTON"), _T(BUTTON_TITLE), WS_CHILD,
		BUTTON_X, BUTTON_Y, BUTTON_CX, BUTTON_CY, myWnd, NULL, hInstance, NULL);

	if (!buttonWnd)
		return false;

	staticWnd = CreateWindow(_T("Static"), _T(STATIC_TITLE), WS_CHILD,
		EDIT_WINDOW_X, EDIT_WINDOW_Y, EDIT_WINDOW_CX, EDIT_WINDOW_CY, myWnd, NULL, hInstance, NULL);

	if (!staticWnd)
		return false;
	
   ShowWindow(buttonWnd, nCmdShow);
   UpdateWindow(buttonWnd);

   ShowWindow(staticWnd, nCmdShow);
   UpdateWindow(staticWnd);


	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MINESWEEPERINXP));

	MineSweeper myMS(staticWnd);
	bool buttonDown = false;

	SetTimer(myWnd, 1, REALTIME_DISTANCE, NULL);

	// Main message loop (my):
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			if (msg.hwnd==buttonWnd)
			{
				switch(msg.message)
				{
				case WM_LBUTTONDOWN:
					buttonDown = true;
					break;
				case WM_MOUSELEAVE:
					buttonDown = false;
					break;
				case WM_LBUTTONUP:
					if (buttonDown)
					{
						buttonDown = false;
						if (myMS.Running())
							myMS.End();
						else
							myMS.Begin();
					}
					break;
				default:
					break;
				}
			}
			
			if (msg.message==WM_TIMER)
			{
				myMS.RealTime();

				if (myMS.changed)
				{
					myMS.changed = false;
					if (myMS.Running())
						SetWindowText(buttonWnd, _T(BUTTON_TITLE_T));
					else
						SetWindowText(buttonWnd, _T(BUTTON_TITLE));
				}
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINESWEEPERINXP));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MINESWEEPERINXP);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, HWND &hWnd)
{
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW & (~(WS_SIZEBOX | WS_MAXIMIZEBOX)),
      CW_USEDEFAULT, 0, MY_WINDOW_WIDTH, MY_WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

