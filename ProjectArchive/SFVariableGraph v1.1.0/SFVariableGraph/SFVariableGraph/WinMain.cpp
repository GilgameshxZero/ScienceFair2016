#include "WinMain.h"

int CALLBACK WinMain (_In_ HINSTANCE hinst, _In_ HINSTANCE hprevinst, _In_ LPSTR cmdline, _In_ int cmdshow)
{
	WNDCLASSEX wndclass;

	wndclass.cbSize = sizeof (WNDCLASSEX);
	wndclass.style = NULL;
	wndclass.lpfnWndProc = SFVG::MainWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hinst;
	wndclass.hIcon = NULL;
	wndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "SFVGMainWnd";
	wndclass.hIconSm = NULL;

	if (!RegisterClassEx (&wndclass))
	{
		MessageBox (NULL, "RegisterClassEx Main Window failed!", "", MB_OK);
		return 0;
	}

	const int width = GetSystemMetrics (SM_CXSCREEN),
		height = GetSystemMetrics (SM_CYSCREEN);
	HWND mwnd = CreateWindowEx (NULL, "SFVGMainWnd", "SFVG - Yang Yan", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 0, 0, width, height, NULL, NULL, hinst, NULL);

	if (!mwnd)
	{
		MessageBox (NULL, "CreateWindowEx Main Window failed!", "", MB_OK);
		return 0;
	}

	ShowWindow (mwnd, cmdshow);
	ShowWindow (mwnd, SW_MAXIMIZE);

	//Create child windows (graph window).
	RECT client;
	GetClientRect (mwnd, &client);

	SFVG::Graph graph (mwnd);
	MoveWindow (graph.GetHWND (), 0, 0, client.right * 2 / 3, client.bottom, true);

	//Slider panel.
	SFVG::SliderPanel sp (mwnd);
	MoveWindow (sp.GetHWND (), client.right * 2 / 3, 0, client.right - client.right * 2 / 3, client.bottom, true);
	sp.SetGraph (&graph);
	
	MSG msg;
	while (GetMessage (&msg, NULL, 0, 0))
	{
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}

	return 0;
}