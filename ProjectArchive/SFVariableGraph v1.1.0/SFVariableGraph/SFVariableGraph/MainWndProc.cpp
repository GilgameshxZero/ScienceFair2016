#include "MainWndProc.h"

namespace SFVG
{
	LRESULT CALLBACK MainWndProc (_In_ HWND hwnd, _In_ UINT umsg, _In_ WPARAM wparam, _In_ LPARAM lparam)
	{
		switch (umsg)
		{
			case WM_CLOSE:
				PostQuitMessage (0);
				break;

			default:
				break;
		}

		return DefWindowProc (hwnd, umsg, wparam, lparam);
	}
}