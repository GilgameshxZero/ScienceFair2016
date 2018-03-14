#include "InfoPanel.h"

namespace SFVG
{
	InfoPanel::InfoPanel (HWND par)
	{
		WNDCLASSEX wndclass;

		wndclass.cbSize = sizeof (WNDCLASSEX);
		wndclass.style = CS_HREDRAW | CS_VREDRAW;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = GetModuleHandle (NULL);
		wndclass.hIcon = NULL;
		wndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
		wndclass.hbrBackground = NULL;
		wndclass.lpszMenuName = NULL;
		wndclass.hIconSm = NULL;

		wndclass.lpfnWndProc = InfoPanelProc;
		std::string classname = "SFVGInfoPanel";
		wndclass.lpszClassName = classname.c_str ();

		if (!RegisterClassEx (&wndclass))
		{
			MessageBox (NULL, "RegisterClassEx InfoPanel Window failed!", "", MB_OK);
			return;
		}

		ipwnd = CreateWindowEx (WS_EX_TRANSPARENT, wndclass.lpszClassName, "", WS_CHILD, 0, 0, 0, 0, par, NULL, wndclass.hInstance, NULL);
		//ipwnd = CreateWindowEx (NULL, wndclass.lpszClassName, "", WS_CHILD, 0, 0, 0, 0, par, NULL, wndclass.hInstance, NULL);

		if (!ipwnd)
		{
			MessageBox (NULL, "CreateWindowEx InfoPanel Window failed!", "", MB_OK);
			return;
		}

		ShowWindow (ipwnd, SW_SHOW);
		SetLayeredWindowAttributes (ipwnd, RGB (255, 255, 255), 0, LWA_COLORKEY);
		SetWindowLongPtr (ipwnd, GWLP_USERDATA, (LONG_PTR)this);
		
		//Create memory DC.
		RECT client;
		GetClientRect (ipwnd, &client);
		hwnddc = GetDC (ipwnd);
		hdcmem = CreateCompatibleDC (hwnddc);

		//Bitmap should be big enough to encompass all resizes.
		const int width = GetSystemMetrics (SM_CXSCREEN),
			height = GetSystemMetrics (SM_CYSCREEN);
		hbmmem = CreateCompatibleBitmap (hwnddc, width, height);

		hbmold = (HBITMAP)SelectObject (hdcmem, hbmmem);

		whitebrush = CreateSolidBrush (RGB (255, 255, 255));
		oldbrush = (HBRUSH)SelectObject (hdcmem, whitebrush);

		int fheight;
		fheight = -MulDiv (12, GetDeviceCaps (hwnddc, LOGPIXELSY), 72);
		font = CreateFont (fheight, 0, 0, 0, 0, false, false, false, 0, 0, 0, 0, 0, "Consolas");
		oldfont = (HFONT)SelectObject (hdcmem, font);

		SetDCPenColor (hdcmem, RGB (0, 0, 0));
	}

	HWND InfoPanel::GetHWND ()
	{
		return ipwnd;
	}
	void InfoPanel::UpdateInfo ()
	{
		static const int precision = 2;
		std::stringstream ss;
		ss << std::fixed << std::setprecision (precision) << "C=" << SFVE::rt << "-B-(" << SFVE::rn2 << "*B)/(" << SFVE::r2 << "*(" << SFVE::dt << "-B))";
		label[0] = ss.str ();
		ss = std::stringstream ();
		ss << std::fixed << std::setprecision (precision) << "C=" << (-SFVE::r2*SFVE::r2*SFVE::rn1 + SFVE::r2*SFVE::r1*SFVE::rn2) << "*B^3+" << (2 * SFVE::r2*SFVE::r2*SFVE::rn1*SFVE::dt + SFVE::r2*SFVE::r2*SFVE::rn1*SFVE::rt - SFVE::r2*SFVE::r1*SFVE::SFVE::rn2*SFVE::rt - SFVE::r2*SFVE::r1*SFVE::SFVE::rn2*SFVE::dt + SFVE::r2*SFVE::r1*SFVE::SFVE::rn2*SFVE::lt + SFVE::rn2*SFVE::r2*SFVE::rn1 - SFVE::rn2*SFVE::r1*SFVE::SFVE::rn2) << "*B^2+" << (-2 * SFVE::r2*SFVE::r2*SFVE::rn1*SFVE::rt*SFVE::dt - SFVE::r2*SFVE::r2*SFVE::rn1*SFVE::dt*SFVE::dt + SFVE::r2*SFVE::r1*SFVE::SFVE::rn2*SFVE::rt*SFVE::dt - SFVE::r2*SFVE::r1*SFVE::SFVE::rn2*SFVE::dt*SFVE::lt - SFVE::rn2*SFVE::r2*SFVE::rn1*SFVE::dt) << "*B+" << SFVE::r2*SFVE::r2*SFVE::rn1*SFVE::rt*SFVE::dt*SFVE::dt;
		label[1] = ss.str ();
		ss = std::stringstream ();
		ss << std::fixed << std::setprecision (precision) << "B=" << SFVE::rt << "-C-(" << SFVE::rn1 << "*C)/(" << SFVE::r1 << "*(" << SFVE::lt << "-C))";
		label[2] = ss.str ();
		ss = std::stringstream ();
		ss << std::fixed << std::setprecision (precision) << "B=" << (-SFVE::r1*SFVE::r1*SFVE::rn2 + SFVE::r1*SFVE::r2*SFVE::rn1) << "*C^3+" << (2 * SFVE::r1*SFVE::r1*SFVE::rn2*SFVE::lt + SFVE::r1*SFVE::r1*SFVE::rn2*SFVE::rt - SFVE::r1*SFVE::r2*SFVE::SFVE::rn1*SFVE::rt - SFVE::r1*SFVE::r2*SFVE::SFVE::rn1*SFVE::lt + SFVE::r1*SFVE::r2*SFVE::SFVE::rn1*SFVE::dt + SFVE::rn1*SFVE::r1*SFVE::rn2 - SFVE::rn1*SFVE::r2*SFVE::SFVE::rn1) << "*C^2+" << (-2 * SFVE::r1*SFVE::r1*SFVE::rn2*SFVE::rt*SFVE::lt - SFVE::r1*SFVE::r1*SFVE::rn2*SFVE::lt*SFVE::lt + SFVE::r1*SFVE::r2*SFVE::SFVE::rn1*SFVE::rt*SFVE::lt - SFVE::r1*SFVE::r2*SFVE::SFVE::rn1*SFVE::lt*SFVE::dt - SFVE::rn1*SFVE::r1*SFVE::rn2*SFVE::lt) << "*C+" << SFVE::r1*SFVE::r1*SFVE::rn2*SFVE::rt*SFVE::lt*SFVE::lt;
		label[3] = ss.str ();
		ss = std::stringstream ();

		double posx = graph->GetCenterX (), posy = graph->GetCenterY (), scale = graph->GetScale ();
		static RECT client;
		GetClientRect (graph->GetHWND (), &client);
		ss << std::fixed << std::setprecision (max (2, precision - 2)) << "View: ((" << posx - (double)(client.right / 2) * scale << ", " << posx + (double)(client.right / 2) * scale << "), (" << posy - (double)(client.bottom / 2) * scale << ", " << posy + (double)(client.bottom / 2) * scale << "))";
		label[4] = ss.str ();
	}
	void InfoPanel::SetGraph (Graph *new_graph)
	{
		graph = new_graph;
	}

	LRESULT CALLBACK InfoPanelProc (_In_ HWND hwnd, _In_ UINT umsg, _In_ WPARAM wparam, _In_ LPARAM lparam)
	{
		InfoPanel *cur = (InfoPanel *)GetWindowLongPtr (hwnd, GWLP_USERDATA);

		switch (umsg)
		{
			case WM_PAINT:
				return cur->OnPaint (wparam, lparam);

			case WM_DESTROY:
				return cur->OnDestroy (wparam, lparam);

			default:
				break;
		}

		return DefWindowProc (hwnd, umsg, wparam, lparam);
	}

	LRESULT InfoPanel::OnPaint (WPARAM wparam, LPARAM lparam)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint (ipwnd, &ps);

		RECT client;
		GetClientRect (ipwnd, &client);

		FillRect (hdcmem, &client, whitebrush);
		static COLORREF textcolor[5] = {RGB (150, 0, 0), RGB (150, 0, 150), RGB (0, 150, 0), RGB (0, 150, 150), RGB (0, 0, 0)};

		for (int a = 0;a < 5;a++)
		{
			SetTextColor (hdcmem, textcolor[a]);
			TextOut (hdcmem, 0, a * 25, label[a].c_str (), static_cast<int>(label[a].length ()));
		}

		//BitBlt (hwnddc, 0, 0, client.right, client.bottom, hdcmem, 0, 0, SRCCOPY);
		TransparentBlt (hwnddc, 0, 0, client.right, client.bottom, hdcmem, 0, 0, client.right, client.bottom, RGB (255, 255, 255));

		EndPaint (ipwnd, &ps);

		return 0;
	}

	LRESULT InfoPanel::OnDestroy (WPARAM wparam, LPARAM lparam)
	{
		SelectObject (hdcmem, oldbrush);

		DeleteObject (whitebrush);

		SelectObject (hdcmem, hbmold);

		DeleteObject (hbmmem);
		DeleteDC (hdcmem);
		ReleaseDC (ipwnd, hwnddc);

		return 0;
	}
}