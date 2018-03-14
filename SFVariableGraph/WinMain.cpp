#include "WinMain.h"

SFVG::Graph *ptrgraph;

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
	std::string titlebar = "Science Fair Project: Variable Function Grapher v";
	std::stringstream ss;
	ss << VERSION_MAJOR << '.' << VERSION_MINOR << '.' << VERSION_REVISION << '.' << BUILD_INCREMENT << " - Yang Yan";
	titlebar += ss.str ();
	HWND mwnd = CreateWindowEx (NULL, "SFVGMainWnd", titlebar.c_str (), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 0, 0, width, height, NULL, NULL, hinst, NULL);

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
	ptrgraph = &graph;
	MoveWindow (graph.GetHWND (), 0, 0, client.right * 2 / 3, client.bottom, true);
	graph.AddCLineX (SFVG::SFVE::DT1, RGB (150, 0, 0));
	graph.AddCLineX (SFVG::SFVE::Cubic1, RGB (150, 0, 150));
	graph.AddCLineY (SFVG::SFVE::DT2, RGB (0, 150, 0));
	graph.AddCLineY (SFVG::SFVE::Cubic2, RGB (0, 150, 150));

	graph.AddCLineX (SFVG::SFVE::RTLine, RGB (0, 150, 0), 0, PS_DASH);
	graph.AddCLineY (SFVG::SFVE::DTLine, RGB (150, 0, 0), 0, PS_DASH);

	//Slider panel.
	SFVG::SliderPanel sp (mwnd);
	MoveWindow (sp.GetHWND (), client.right * 2 / 3, 0, client.right - client.right * 2 / 3, client.bottom, true);
	sp.SetGraph (&graph);

	//idlepanel
	SFVG::IdleHandler::Init (&sp);
	
	MSG msg;
	while (GetMessage (&msg, NULL, 0, 0))
	{
		TranslateMessage (&msg);
		DispatchMessage (&msg);

		//if mouse message, also pass it to central control - to detect idleness
		if (msg.message == WM_MOUSEMOVE)
			SFVG::IdleHandler::ObsMouseMove ();
	}

	return 0;
}