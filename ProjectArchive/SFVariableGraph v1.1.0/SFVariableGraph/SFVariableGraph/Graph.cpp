#include "Graph.h"

namespace SFVG
{
	Graph::Graph (HWND par)
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

		wndclass.lpfnWndProc = GraphWndProc;
		wndclass.lpszClassName = "SFVGGraphWnd";

		if (!RegisterClassEx (&wndclass))
		{
			MessageBox (NULL, "RegisterClassEx Slider Panel Window failed!", "", MB_OK);
			return;
		}

		graphwnd = CreateWindowEx (NULL, wndclass.lpszClassName, "", WS_CHILD | WS_CLIPCHILDREN, 0, 0, 0, 0, par, NULL, wndclass.hInstance, NULL);

		if (!graphwnd)
		{
			MessageBox (NULL, "CreateWindowEx Slider Window failed!", "", MB_OK);
			int k = GetLastError ();
			return;
		}

		ShowWindow (graphwnd, SW_SHOW);

		//Set userdata to point to this object.
		SetWindowLongPtr (graphwnd, GWLP_USERDATA, (LONG_PTR)this);

		//Variables.
		posx = 0; //(X, Y) of the center of window.
		posy = 0;
		scale = 1; //Units per pixel.

		//Create memory DC.
		RECT client;
		GetClientRect (graphwnd, &client);
		hwnddc = GetDC (graphwnd);
		hdcmem = CreateCompatibleDC (hwnddc);

		//Bitmap should be big enough to encompass all resizes.
		const int width = GetSystemMetrics (SM_CXSCREEN),
			height = GetSystemMetrics (SM_CYSCREEN);
		hbmmem = CreateCompatibleBitmap (hwnddc, width, height);

		hbmold = (HBITMAP)SelectObject (hdcmem, hbmmem);

		whitebrush = CreateSolidBrush (RGB (255, 255, 255));
		oldbrush = (HBRUSH)SelectObject (hdcmem, whitebrush);

		//AddYEq ("x", RGB (255, 0, 0));
		//AddYEq ("x*x", RGB (0, 255, 0));
		//AddXEq ("y*y+3", RGB (0, 0, 255));

		/*std::vector<double> coef (4);
		coef[0] = 0;
		coef[1] = 0.5;
		coef[2] = 3;
		coef[3] = 0;
		AddYPoly (coef, RGB (100, 0, 100));*/

		cuscolory.push_back (RGB (150, 0, 0));
		cuscolory.push_back (RGB (150, 0, 150));
		cuscolorx.push_back (RGB (0, 150, 0));
		cuscolorx.push_back (RGB (0, 150, 150));

		cusxshow.push_back (true);
		cusxshow.push_back (true);
		cusyshow.push_back (true);
		cusyshow.push_back (true);
	}

	LRESULT CALLBACK GraphWndProc (_In_ HWND hwnd, _In_ UINT umsg, _In_ WPARAM wparam, _In_ LPARAM lparam)
	{
		Graph *cur = (Graph *)GetWindowLongPtr (hwnd, GWLP_USERDATA);
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

	LRESULT Graph::OnPaint (WPARAM wparam, LPARAM lparam)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint (graphwnd, &ps);

		RECT client;
		GetClientRect (graphwnd, &client);

		FillRect (hdcmem, &client, whitebrush);

		//Draw axis.
		MoveToEx (hdcmem, 0, client.bottom / 2.0 + posy / scale, NULL);
		LineTo (hdcmem, client.right, client.bottom / 2.0 + posy / scale);
		MoveToEx (hdcmem, client.right / 2.0 - posx / scale, 0, NULL);
		LineTo (hdcmem, client.right / 2.0 - posx / scale, client.bottom);

		//Draw general equations.
		double eval, econ, lastecon = 0/*, inten*/;
		////COLORREF tcolor;
		//for (int a = 0;a < yeq.size ();a++)
		//{
		//	tpen = CreatePen (PS_SOLID, 1, yeq[a].color);
		//	oldpen = (HPEN)SelectObject (hdcmem, tpen);
		//	for (int b = 0;b <= client.right;b++)
		//	{
		//		eval = Equations::EvalEq (yeq[a].eq, 'x', (double)b * scale + (posx - client.right / 2.0 * scale));
		//		econ = -eval / scale + client.bottom / 2.0 + posy / scale;

		//		//SetPixel (hdcmem, b, econ, yeq[a].color);
		//		if (b != 0)
		//		{
		//			MoveToEx (hdcmem, b - 1, lastecon, NULL);
		//			LineTo (hdcmem, b, econ);
		//		}

		//		//Draw in different intensities for the rectangle  (b - 1, lastecon) to (b, econ).
		//		/*if (b != 0)
		//		{
		//			if (econ < lastecon)
		//			{
		//				for (int c = lastecon;c != econ;c--)
		//				{
		//					inten = (double)(lastecon - c) / (lastecon - econ);
		//					tcolor = GetPixel (hdcmem, b - 1, c);
		//					tcolor = RGB (max (GetRValue (tcolor), inten * GetRValue (yeq[a].color)), max (GetGValue (tcolor), inten * GetGValue (yeq[a].color)), max (GetBValue (tcolor), inten * GetBValue (yeq[a].color)));
		//					SetPixel (hdcmem, b - 1, c, tcolor);
		//				}
		//			}
		//			else if (econ > lastecon)
		//			{
		//				for (int c = lastecon; c != econ; c++)
		//				{
		//					inten = (double)(c - lastecon) / (lastecon - econ);
		//					tcolor = GetPixel (hdcmem, b - 1, c);
		//					tcolor = RGB (max (GetRValue (tcolor), inten * GetRValue (yeq[a].color)), max (GetGValue (tcolor), inten * GetGValue (yeq[a].color)), max (GetBValue (tcolor), inten * GetBValue (yeq[a].color)));
		//					SetPixel (hdcmem, b - 1, c, tcolor);
		//				}
		//			}
		//		}*/

		//		lastecon = econ;
		//	}
		//	SelectObject (hdcmem, oldpen);
		//	DeleteObject (tpen);
		//}

		//for (int a = 0; a < xeq.size (); a++)
		//{
		//	tpen = CreatePen (PS_SOLID, 1, xeq[a].color);
		//	oldpen = (HPEN)SelectObject (hdcmem, tpen);
		//	for (int b = 0; b <= client.right; b++)
		//	{
		//		eval = Equations::EvalEq (xeq[a].eq, 'y', (double)-b * scale + (posy + client.bottom / 2.0 * scale));
		//		econ = eval / scale + client.right / 2.0 - posx / scale;

		//		if (b != 0)
		//		{
		//			MoveToEx (hdcmem, lastecon, b - 1, NULL);
		//			LineTo (hdcmem, econ, b);
		//		}

		//		lastecon = econ;
		//	}
		//	SelectObject (hdcmem, oldpen);
		//	DeleteObject (tpen);
		//}

		////Draw polynomials.
		double mul, val;

		//for (int a = 0; a < ypoly.size (); a++)
		//{
		//	tpen = CreatePen (PS_SOLID, 1, ypoly[a].color);
		//	oldpen = (HPEN)SelectObject (hdcmem, tpen);
		//	for (int b = 0; b <= client.right; b++)
		//	{
		//		mul = 1;
		//		eval = 0;
		//		val = (double)b * scale + (posx - client.right / 2.0 * scale);
		//		for (int c = 0; c < ypoly[a].coef.size (); c++)
		//		{
		//			eval += mul * ypoly[a].coef[c];
		//			mul *= val;
		//		}
		//		econ = -eval / scale + client.bottom / 2.0 + posy / scale;

		//		if (b != 0)
		//		{
		//			MoveToEx (hdcmem, b - 1, lastecon, NULL);
		//			LineTo (hdcmem, b, econ);
		//		}

		//		lastecon = econ;
		//	}
		//	SelectObject (hdcmem, oldpen);
		//	DeleteObject (tpen);
		//}

		//for (int a = 0; a < xpoly.size (); a++)
		//{
		//	tpen = CreatePen (PS_SOLID, 1, xpoly[a].color);
		//	oldpen = (HPEN)SelectObject (hdcmem, tpen);
		//	for (int b = 0; b <= client.right; b++)
		//	{
		//		mul = 1;
		//		eval = 0;
		//		val = (double)-b * scale + (posy + client.bottom / 2.0 * scale);
		//		for (int c = 0;c < xpoly[a].coef.size ();c++)
		//		{
		//			eval += mul * xpoly[a].coef[c];
		//			mul *= val;
		//		}
		//		econ = eval / scale + client.right / 2.0 - posx / scale;

		//		if (b != 0)
		//		{
		//			MoveToEx (hdcmem, lastecon, b - 1, NULL);
		//			LineTo (hdcmem, econ, b);
		//		}

		//		lastecon = econ;
		//	}
		//	SelectObject (hdcmem, oldpen);
		//	DeleteObject (tpen);
		//}
		//

		//Draw custom functions.
		static double step = 1.0, eps = 1e-4, mdely = 1e-1;
		bool ign_last = true;
		for (int a = 0;a < cuscolory.size ();a++)
		{
			ign_last = true;
			if (cusyshow[a] == false)
				continue;

			tpen = CreatePen (PS_SOLID, 3, cuscolory[a]);
			oldpen = (HPEN)SelectObject (hdcmem, tpen);
			for (double b = 0; b <= client.right; b += step)
			{
				val = (double)b * scale + (posx - client.right / 2.0 * scale);
				eval = CustomYFunction (a, val);
				econ = -eval / scale + client.bottom / 2.0 + posy / scale;

				if (ign_last)
				{
					ign_last = false;
					continue;
				}

				if (abs (lastecon - econ) < max (client.right, client.bottom) * mdely)
				{
					MoveToEx (hdcmem, b - step, lastecon, NULL);
					LineTo (hdcmem, b, econ);
					step = min (1.0, step * 2);
					lastecon = econ;
					ign_last = false;
				}
				else if (abs (lastecon - econ) >= max (client.right, client.bottom) * mdely)
				{
					if (step >= eps * 2)
					{
						b -= step;
						ign_last = false;
					}
					else
					{
						lastecon = econ;
						ign_last = true;
					}
					step = max (eps, step / 2);
				}
			}
			SelectObject (hdcmem, oldpen);
			DeleteObject (tpen);
		}
		
		step = 1.0;
		for (int a = 0; a < cuscolorx.size (); a++)
		{
			ign_last = true;
			if (cusxshow[a] == false)
				continue;

			tpen = CreatePen (PS_SOLID, 3, cuscolorx[a]);
			oldpen = (HPEN)SelectObject (hdcmem, tpen);
			for (double b = 0; b <= client.right; b += step)
			{
				val = (double)-b * scale + (posy + client.bottom / 2.0 * scale);
				eval = CustomXFunction (a, val);
				econ = eval / scale + client.right / 2.0 - posx / scale;

				if (ign_last)
				{
					ign_last = false;
					continue;
				}

				if (abs (lastecon - econ) < max (client.right, client.bottom) * mdely)
				{
					MoveToEx (hdcmem, lastecon, b - step, NULL);
					LineTo (hdcmem, econ, b);
					step = min (1.0, step * 2);
					lastecon = econ;
					ign_last = false;
				}
				else if (abs (lastecon - econ) >= max (client.right, client.bottom) * mdely)
				{
					if (step >= eps * 2)
					{
						b -= step;
						ign_last = false;
					}
					else
					{
						lastecon = econ;
						ign_last = true;
					}
					step = max (eps, step / 2);
				}
			}
			SelectObject (hdcmem, oldpen);
			DeleteObject (tpen);
		}
		
		//Draw point-defined lines.
		for (int a = 0;a < pline.size ();a++)
		{
			tpen = CreatePen (PS_SOLID, 5, RGB (0, 0, 150));
			oldpen = (HPEN)SelectObject (hdcmem, tpen);
			for (int b = 0; b < pline[a].size () - 1; b++)
			{
				MoveToEx (hdcmem, pline[a][b].first / scale + client.right / 2.0 - posx / scale, -pline[a][b].second / scale + client.bottom / 2.0 + posy / scale, NULL);
				LineTo (hdcmem, pline[a][b + 1].first / scale + client.right / 2.0 - posx / scale, -pline[a][b + 1].second / scale + client.bottom / 2.0 + posy / scale);
			}
			SelectObject (hdcmem, oldpen);
			DeleteObject (tpen);
		}

		BitBlt (hwnddc, 0, 0, client.right, client.bottom, hdcmem, 0, 0, SRCCOPY);

		EndPaint (graphwnd, &ps);

		return 0;
	}

	LRESULT Graph::OnDestroy (WPARAM wparam, LPARAM lparam)
	{
		SelectObject (hdcmem, oldbrush);
		
		DeleteObject (whitebrush);

		SelectObject (hdcmem, hbmold);

		DeleteObject (hbmmem);
		DeleteDC (hdcmem);
		ReleaseDC (graphwnd, hwnddc);

		return 0;
	}

	HWND Graph::GetHWND ()
	{
		return graphwnd;
	}

	int Graph::AddXEq (std::string neweq, COLORREF color)
	{
		xeq.push_back (Line ());
		xeq.back ().eq = neweq;
		xeq.back ().color = color;

		return xeq.size () - 1;
	}

	int Graph::AddYEq (std::string neweq, COLORREF color)
	{
		yeq.push_back (Line ());
		yeq.back ().eq = neweq;
		yeq.back ().color = color;

		return yeq.size () - 1;
	}

	void Graph::EditXEq (int id, std::string neweq, COLORREF color)
	{
		xeq[id].eq = neweq;
		xeq[id].color = color;
	}

	void Graph::EditYEq (int id, std::string neweq, COLORREF color)
	{
		yeq[id].eq = neweq;
		yeq[id].color = color;
	}

	int Graph::AddXPoly (std::vector<double> coef, COLORREF color)
	{
		xpoly.push_back (Poly ());
		xpoly.back ().coef = coef;
		xpoly.back ().color = color;

		return xpoly.size () - 1;
	}

	int Graph::AddYPoly (std::vector<double> coef, COLORREF color)
	{
		ypoly.push_back (Poly ());
		ypoly.back ().coef = coef;
		ypoly.back ().color = color;

		return ypoly.size () - 1;
	}

	void Graph::EditXPoly (int id, std::vector<double> coef, COLORREF color)
	{
		xpoly[id].coef = coef;
		xpoly[id].color = color;
	}

	void Graph::EditYPoly (int id, std::vector<double> coef, COLORREF color)
	{
		ypoly[id].coef = coef;
		ypoly[id].color = color;
	}

	void Graph::EditScale (double newscale)
	{
		scale = newscale;
	}

	void Graph::EditCenterX (double newx)
	{
		posx = newx;
	}

	void Graph::EditCenterY (double newy)
	{
		posy = newy;
	}

	double Graph::GetScale ()
	{
		return scale;
	}

	double Graph::GetCenterX ()
	{
		return posy;
	}

	double Graph::GetCenterY ()
	{
		return posy;
	}

	double Graph::CustomXFunction (int id, double val)
	{
		if (id == 0)
		{
			if (SFVars::r2 * (SFVars::dt - val) == 0)
				return SFVars::lt - val - (SFVars::rn2 * val) / 1e-9;
			return SFVars::lt - val - (SFVars::rn2 * val) / (SFVars::r2 * (SFVars::dt - val));
		}
		if (id == 1)
		{
			return (-SFVars::r2*SFVars::r2*SFVars::rn1 + SFVars::r2*SFVars::r1*SFVars::rn2)*val*val*val + (2 * SFVars::r2*SFVars::r2*SFVars::rn1*SFVars::dt + SFVars::r2*SFVars::r2*SFVars::rn1*SFVars::lt - SFVars::r2*SFVars::r1*SFVars::SFVars::rn2*SFVars::lt - SFVars::r2*SFVars::r1*SFVars::SFVars::rn2*SFVars::dt + SFVars::r2*SFVars::r1*SFVars::SFVars::rn2*SFVars::rt + SFVars::rn2*SFVars::r2*SFVars::rn1 - SFVars::rn2*SFVars::r1*SFVars::SFVars::rn2)*val*val + (-2 * SFVars::r2*SFVars::r2*SFVars::rn1*SFVars::lt*SFVars::dt - SFVars::r2*SFVars::r2*SFVars::rn1*SFVars::dt*SFVars::dt + SFVars::r2*SFVars::r1*SFVars::SFVars::rn2*SFVars::lt*SFVars::dt - SFVars::r2*SFVars::r1*SFVars::SFVars::rn2*SFVars::dt*SFVars::rt - SFVars::rn2*SFVars::r2*SFVars::rn1*SFVars::dt)*val + SFVars::r2*SFVars::r2*SFVars::rn1*SFVars::lt*SFVars::dt*SFVars::dt;
		}
	}

	double Graph::CustomYFunction (int id, double val)
	{
		if (id == 0)
		{
			if (SFVars::SFVars::r1 * (SFVars::SFVars::rt - val) == 0)
				return SFVars::SFVars::lt - val - (SFVars::SFVars::rn1 * val) / 1e-9;
			return SFVars::SFVars::lt - val - (SFVars::SFVars::rn1 * val) / (SFVars::SFVars::r1 * (SFVars::SFVars::rt - val));
		}
		if (id == 1)
		{
			return (-SFVars::r1*SFVars::r1*SFVars::rn2 + SFVars::r1*SFVars::r2*SFVars::rn1)*val*val*val + (2 * SFVars::r1*SFVars::r1*SFVars::rn2*SFVars::rt + SFVars::r1*SFVars::r1*SFVars::rn2*SFVars::lt - SFVars::r1*SFVars::r2*SFVars::SFVars::rn1*SFVars::lt - SFVars::r1*SFVars::r2*SFVars::SFVars::rn1*SFVars::rt + SFVars::r1*SFVars::r2*SFVars::SFVars::rn1*SFVars::dt + SFVars::rn1*SFVars::r1*SFVars::rn2 - SFVars::rn1*SFVars::r2*SFVars::SFVars::rn1)*val*val + (-2 * SFVars::r1*SFVars::r1*SFVars::rn2*SFVars::lt*SFVars::rt - SFVars::r1*SFVars::r1*SFVars::rn2*SFVars::rt*SFVars::rt + SFVars::r1*SFVars::r2*SFVars::SFVars::rn1*SFVars::lt*SFVars::rt - SFVars::r1*SFVars::r2*SFVars::SFVars::rn1*SFVars::rt*SFVars::dt - SFVars::rn1*SFVars::r1*SFVars::rn2*SFVars::rt)*val + SFVars::r1*SFVars::r1*SFVars::rn2*SFVars::lt*SFVars::rt*SFVars::rt;
		}
	}

	void Graph::ClearPLine ()
	{
		pline.clear ();
	}

	void Graph::AddPLine (std::vector< std::pair<double, double> > &pvec)
	{
		pline.push_back (std::vector< std::pair<double, double> > (pvec));
	}
}