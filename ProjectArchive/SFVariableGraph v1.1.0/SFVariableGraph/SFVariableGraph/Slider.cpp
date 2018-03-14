#include "Slider.h"

namespace SFVG
{
	int Slider::id = 0;

	Slider::Slider (HWND par)
	{
		//Init this so that OnSize doesn't do bad things.
		hwnddc = NULL;

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

		wndclass.lpfnWndProc = SliderProc;
		std::string classname = (std::string)"SFVGSlider" + Utility::IntToStr (id++);
		wndclass.lpszClassName = classname.c_str ();

		if (!RegisterClassEx (&wndclass))
		{
			MessageBox (NULL, "RegisterClassEx Slider Window failed!", "", MB_OK);
			return;
		}

		sliderwnd = CreateWindowEx (NULL, wndclass.lpszClassName, "", WS_CHILD, 0, 0, 0, 0, par, NULL, wndclass.hInstance, NULL);

		if (!sliderwnd)
		{
			MessageBox (NULL, "CreateWindowEx Slider Window failed!", "", MB_OK);
			return;
		}

		//Set userdata to point to this object.
		SetWindowLongPtr (sliderwnd, GWLP_USERDATA, (LONG_PTR)this);

		ShowWindow (sliderwnd, SW_SHOW);

		//Set constants.
		bkgdcolor = RGB (0, 0, 0);
		slcolor = RGB (150, 150, 150);
		pos = 0;
		dragging = false;

		//Create memory DC.
		RECT client;
		GetClientRect (sliderwnd, &client);
		hwnddc = GetDC (sliderwnd);
		hdcmem = CreateCompatibleDC (hwnddc);

		//Bitmap should be big enough to encompass all resizes.
		const int width = GetSystemMetrics (SM_CXSCREEN),
			height = GetSystemMetrics (SM_CYSCREEN);
		hbmmem = CreateCompatibleBitmap (hwnddc, width, height);

		hbmold = (HBITMAP)SelectObject (hdcmem, hbmmem);

		bkbrush = CreateSolidBrush (bkgdcolor);
		slbrush = CreateSolidBrush (slcolor);
		bkpen = CreatePen (PS_SOLID, 0, bkgdcolor);
		slpen = CreatePen (PS_SOLID, 0, slcolor);

		oldbrush = (HBRUSH)SelectObject (hdcmem, bkbrush);
		oldpen = (HPEN)SelectObject (hdcmem, bkpen);

		//PostMessage (par, WM_SLIDERUPDATE, 0, (LPARAM)this);
	}

	LRESULT CALLBACK SliderProc (HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
	{
		Slider *cur = (Slider *)(GetWindowLongPtr (hwnd, GWLP_USERDATA));
		switch (umsg)
		{
			case WM_PAINT:
				return cur->OnPaint (wparam, lparam);

			case WM_MOUSEMOVE:
				return cur->OnMouseMove (wparam, lparam);

			case WM_LBUTTONDOWN:
				return cur->OnLButtonDown (wparam, lparam);

			case WM_LBUTTONUP:
				return cur->OnLButtonUp (wparam, lparam);

			case WM_DESTROY:
				return cur->OnDestroy (wparam, lparam);

			case WM_SIZE:
				return cur->OnSize (wparam, lparam);

			default:
				break;
		}

		return DefWindowProc (hwnd, umsg, wparam, lparam);
	}

	LRESULT Slider::OnPaint (WPARAM wparam, LPARAM lparam)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint (sliderwnd, &ps);

		RECT client;
		GetClientRect (sliderwnd, &client);

		SelectObject (hdcmem, bkpen);
		SelectObject (hdcmem, bkbrush);

		FillRect (hdcmem, &client, bkbrush);

		//Draw slider rectange.
		RECT box;
		box.top = 0;
		box.left = (double)(1.0 + pos) / 2 * (client.right - client.bottom);
		box.right = box.left + client.bottom;
		box.bottom = client.bottom;

		SelectObject (hdcmem, slpen);
		SelectObject (hdcmem, slbrush);

		FillRect (hdcmem, &box, slbrush);

		BitBlt (hwnddc, 0, 0, client.right, client.bottom, hdcmem, 0, 0, SRCCOPY);
		//SetDCBrushColor (hwnddc, RGB (150, 0, 0));
		//Rectangle (hdc, 0, 0, client.right / 2, client.bottom / 2);
		//FillRect (hdc, &client, bkbrush);

		EndPaint (sliderwnd, &ps);

		return 0;
	}

	LRESULT Slider::OnMouseMove (WPARAM wparam, LPARAM lparam)
	{
		if (dragging)
		{
			RECT client;
			GetClientRect (sliderwnd, &client);

			int xPos = GET_X_LPARAM (lparam);
			double diff = (double)(xPos - origx) / (client.right - client.bottom) * 2;
			pos = origpos + diff;

			if (pos < -1)
				pos = -1;
			if (pos > 1)
				pos = 1;

			InvalidateRect (sliderwnd, &client, false);
			UpdateWindow (sliderwnd);
			
			//Send udpate to parent, lparam = this for idenfication.
			HWND par = (HWND)GetWindowLongPtr (sliderwnd, GWLP_HWNDPARENT);
			SendMessage (par, WM_SLIDERUPDATE, 0, (LPARAM)(this));
		}

		return 0;
	}

	LRESULT Slider::OnLButtonDown (WPARAM wparam, LPARAM lparam)
	{
		RECT client;
		GetClientRect (sliderwnd, &client);

		int xPos = GET_X_LPARAM (lparam),
			yPos = GET_Y_LPARAM (lparam),
			boxleft = (1 + pos) / 2 * (client.right - client.bottom),
			boxright = boxleft + client.bottom;

		if (xPos >= boxleft && xPos <= boxright)
		{
			SetCapture (sliderwnd);
			dragging = true;
			origx = xPos;
			origpos = pos;
		}

		return 0;
	}

	LRESULT Slider::OnLButtonUp (WPARAM wparam, LPARAM lparam)
	{
		if (dragging)
		{
			ReleaseCapture ();
			dragging = false;
		}

		return 0;
	}

	LRESULT Slider::OnDestroy (WPARAM wparam, LPARAM lparam)
	{
		SelectObject (hdcmem, oldbrush);
		SelectObject (hdcmem, oldpen);

		DeleteObject (bkbrush);
		DeleteObject (slbrush);
		DeleteObject (bkpen);
		DeleteObject (slpen);

		SelectObject (hdcmem, hbmold);

		DeleteObject (hbmmem);
		DeleteDC (hdcmem);
		ReleaseDC (sliderwnd, hwnddc);

		return 0;
	}

	LRESULT Slider::OnSize (WPARAM wparam, LPARAM lparam)
	{
		return 0;
	}

	HWND Slider::GetHWND ()
	{
		return sliderwnd;
	}

	double Slider::GetPos ()
	{
		return pos;
	}

	void Slider::UpdatePos (double new_pos)
	{
		pos = new_pos;

		if (pos < -1)
			pos = -1;
		if (pos > 1)
			pos = 1;

		HWND par = (HWND)GetWindowLongPtr (sliderwnd, GWLP_HWNDPARENT);
		PostMessage (par, WM_SLIDERUPDATE, 0, (LPARAM)this);

		RECT client;
		GetClientRect (sliderwnd, &client);
		InvalidateRect (sliderwnd, &client, false);
		UpdateWindow (sliderwnd);
	}
}