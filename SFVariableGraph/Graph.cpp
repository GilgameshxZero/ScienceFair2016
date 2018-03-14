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

		//graphwnd = CreateWindowEx (NULL, wndclass.lpszClassName, "", WS_CHILD | WS_CLIPCHILDREN, 0, 0, 0, 0, par, NULL, wndclass.hInstance, NULL);
		graphwnd = CreateWindowEx (NULL, wndclass.lpszClassName, "", WS_CHILD, 0, 0, 0, 0, par, NULL, wndclass.hInstance, NULL);

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

		/*cuscolory.push_back (RGB (150, 0, 0));
		cuscolory.push_back (RGB (150, 0, 150));
		cuscolorx.push_back (RGB (0, 150, 0));
		cuscolorx.push_back (RGB (0, 150, 150));

		cusxshow.push_back (true);
		cusxshow.push_back (true);
		cusyshow.push_back (true);
		cusyshow.push_back (true);*/

		labels = new InfoPanel (graphwnd);
		labels->SetGraph (this);

		slope_arrow = false;
	}

	LRESULT CALLBACK GraphWndProc (_In_ HWND hwnd, _In_ UINT umsg, _In_ WPARAM wparam, _In_ LPARAM lparam)
	{
		Graph *cur = (Graph *)GetWindowLongPtr (hwnd, GWLP_USERDATA);

		if (cur == NULL)
			return DefWindowProc (hwnd, umsg, wparam, lparam);

		switch (umsg)
		{
			case WM_PAINT:
				return cur->OnPaint (wparam, lparam);

			case WM_DESTROY:
				return cur->OnDestroy (wparam, lparam);

			case WM_SIZE:
				return cur->OnSize (wparam, lparam);

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
		MoveToEx (hdcmem, 0, static_cast<int>(client.bottom / 2.0 + posy / scale), NULL);
		LineTo (hdcmem, client.right, static_cast<int>(client.bottom / 2.0 + posy / scale));
		MoveToEx (hdcmem, static_cast<int>(client.right / 2.0 - posx / scale), 0, NULL);
		LineTo (hdcmem, static_cast<int>(client.right / 2.0 - posx / scale), client.bottom);

		DrawGLine ();
		DrawPolyLine ();
		DrawCLine (); //Custom functions before point-defined.
		DrawPLine ();

		if (slope_arrow)
			DrawSlopeArrows ();

		BitBlt (hwnddc, 0, 0, client.right, client.bottom, hdcmem, 0, 0, SRCCOPY);

		labels->UpdateInfo ();
		RECT lclient;
		GetClientRect (labels->GetHWND (), &lclient);
		InvalidateRect (labels->GetHWND (), &lclient, false);
		UpdateWindow (labels->GetHWND ());

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

	LRESULT Graph::OnSize (WPARAM wparam, LPARAM lparam)
	{
		RECT client, rect;
		GetClientRect (graphwnd, &client);

		MoveWindow (labels->GetHWND (), 0, client.bottom - 125, 400, 125, false);
		GetClientRect (labels->GetHWND (), &rect);
		InvalidateRect (labels->GetHWND (), &rect, false);
		UpdateWindow (labels->GetHWND ());

		return 0;
	}

	HWND Graph::GetHWND ()
	{
		return graphwnd;
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
		return posx;
	}

	double Graph::GetCenterY ()
	{
		return posy;
	}

	void Graph::ClearPLine ()
	{
		pline.clear ();
	}

	void Graph::AddPLine (std::vector< std::pair<double, double> > &pvec)
	{
		static PLine temp;
		temp.pt = pvec;
		pline.push_back (temp);
	}

	int Graph::AddCLineX (CLinePtr func, COLORREF color, int thick, int style)
	{
		static CLine temp;
		temp.func = func;
		temp.color = color;
		temp.thick = thick;
		temp.style = style;
		temp.visible = true;
		clineX.push_back (temp);

		return clineX.size () - 1;
	}
	int Graph::AddCLineY (CLinePtr func, COLORREF color, int thick, int style)
	{
		static CLine temp;
		temp.func = func;
		temp.color = color;
		temp.thick = thick;
		temp.style = style;
		temp.visible = true;
		clineY.push_back (temp);

		return clineX.size () - 1;
	}
	void Graph::ToggleCLineX (int x)
	{
		clineX[x].visible = !clineX[x].visible;
	}
	void Graph::ToggleCLineY (int x)
	{
		clineY[x].visible = !clineY[x].visible;
	}
	void Graph::DisableAllCLine ()
	{
		for (int a = 0;a < clineX.size ();a++)
		{
			clineX[a].visible = false;
		}
		for (int a = 0; a < clineY.size (); a++)
		{
			clineY[a].visible = false;
		}
	}
	void Graph::PopCLineX ()
	{
		clineX.pop_back ();
	}
	void Graph::PopCLineY ()
	{
		clineX.pop_back ();
	}

	void Graph::DrawPLine ()
	{
		static RECT client;
		static const int rad = 1.9;
		static double step = 1.0, eps = 1e-4, mdely = 1e-1, val, eval, econ, lastecon = 0;
		static const COLORREF plinecol = RGB (0, 0, 255);

		GetClientRect (graphwnd, &client);

		//Draw point-defined lines.
		for (int a = 0; a < pline.size (); a++)
		{
			//tpen = CreatePen (PS_SOLID, 3, RGB (0, 0, 255));
			//oldpen = (HPEN)SelectObject (hdcmem, tpen);
			for (int b = 0; b < pline[a].pt.size ()/* - 1*/; b++)
			{
				static std::pair<double, double> pt;
				pt.first = pline[a].pt[b].first / scale + client.right / 2.0 - posx / scale;
				pt.second = pline[a].pt[b].second / -scale + client.bottom / 2.0 + posy / scale;
				//uncomment if you want to draw pline as lines instead of points
				/*MoveToEx (hdcmem, static_cast<int>(pline[a].pt[b].first / scale + client.right / 2.0 - posx / scale), static_cast<int>(-pline[a].pt[b].second / scale + client.bottom / 2.0 + posy / scale), NULL);
				LineTo (hdcmem, static_cast<int>(pline[a].pt[b + 1].first / scale + client.right / 2.0 - posx / scale), static_cast<int>(-pline[a].pt[b + 1].second / scale + client.bottom / 2.0 + posy / scale));*/
				//Ellipse (hdcmem, round (pt.first - rad), round (pt.second - rad), round (pt.first + rad), round (pt.second + rad));
				SetPixel (hdcmem, pt.first - 1, pt.second - 1, plinecol);
				SetPixel (hdcmem, pt.first, pt.second - 1, plinecol);
				SetPixel (hdcmem, pt.first, pt.second, plinecol);
				SetPixel (hdcmem, pt.first, pt.second + 1, plinecol);
				SetPixel (hdcmem, pt.first + 1, pt.second + 1, plinecol);
			}
			//SelectObject (hdcmem, oldpen);
			//DeleteObject (tpen);
		}
	}
	void Graph::DrawCLine ()
	{
		static RECT client;
		static double step = 1.0, eps = 1e-4, mdely = 1e-1, val, eval, econ, lastecon = 0;
		static bool ign_last; //Ignore last point, for first point.

		GetClientRect (graphwnd, &client);

		for (int a = 0; a < clineY.size (); a++)
		{
			ign_last = true;
			if (!clineY[a].visible)
				continue;

			tpen = CreatePen (clineY[a].style, clineY[a].thick, clineY[a].color);
			oldpen = (HPEN)SelectObject (hdcmem, tpen);
			for (double b = 0; b <= client.right; b += step)
			{
				val = (double)b * scale + (posx - client.right / 2.0 * scale);
				eval = clineY[a].func (val);
				econ = -eval / scale + client.bottom / 2.0 + posy / scale;

				if (ign_last)
				{
					ign_last = false;
					continue;
				}

				if (abs (lastecon - econ) < max (client.right, client.bottom) * mdely)
				{
					MoveToEx (hdcmem, static_cast<int>(b - step), static_cast<int>(lastecon), NULL);
					LineTo (hdcmem, static_cast<int>(b), static_cast<int>(econ));
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
		for (int a = 0; a < clineX.size (); a++)
		{
			ign_last = true;
			if (!clineX[a].visible)
				continue;

			tpen = CreatePen (clineX[a].style, clineX[a].thick, clineX[a].color);
			oldpen = (HPEN)SelectObject (hdcmem, tpen);
			for (double b = 0; b <= client.bottom; b += step)
			{
				val = (double)-b * scale + (posy + client.bottom / 2.0 * scale);
				eval = clineX[a].func (val);
				econ = eval / scale + client.right / 2.0 - posx / scale;

				if (ign_last)
				{
					ign_last = false;
					continue;
				}

				if (abs (lastecon - econ) < max (client.right, client.bottom) * mdely)
				{
					MoveToEx (hdcmem, static_cast<int>(lastecon), static_cast<int>(b - step), NULL);
					LineTo (hdcmem, static_cast<int>(econ), static_cast<int>(b));
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
	}
	void Graph::DrawSlopeArrows ()
	{
		static RECT client;
		static const int div = 21; //Draw div - 1 slope arrows per row/col.
		static const double len = 15, whisrat = 5; //Length of each arrow.
		static double pixx, pixy, x, y, dx, dy, z, adjdx, adjdy;

		GetClientRect (graphwnd, &client);

		tpen = CreatePen (PS_SOLID, 1, RGB (0, 0, 0));
		oldpen = (HPEN)SelectObject (hdcmem, tpen);

		for (int a = 0; a < div - 1;a++)
		{
			for (int b = 0;b < div - 1;b++)
			{
				pixx = (1 + a) * client.right / div;
				pixy = (1 + b) * client.bottom / div;
				x = posx + (pixx - client.right / 2) * scale;
				y = posy - (pixy - client.bottom / 2) * scale;
				dx = SFVE::r1 *	(SFVE::lt - x - y) * (SFVE::rt - x) - SFVE::rn1 * x;
				dy = SFVE::r2 *	(SFVE::lt - x - y) * (SFVE::dt - y) - SFVE::rn2 * y;
				z = sqrt (dx * dx + dy * dy);
				adjdx = (dx / z) * len;
				adjdy = -(dy / z) * len; //Negative because axis different.
				MoveToEx (hdcmem, pixx - adjdx / 2, pixy - adjdy / 2, NULL);
				LineTo (hdcmem, pixx + adjdx / 2, pixy + adjdy / 2);
				//Ellipse (hdcmem, pixx + adjdx / 2 - 2, pixy + adjdy / 2 - 2, pixx + adjdx / 2 + 2, pixy + adjdy / 2 + 2);
				LineTo (hdcmem, pixx + adjdy / whisrat, pixy - adjdx / whisrat);
				MoveToEx (hdcmem, pixx + adjdx / 2, pixy + adjdy / 2, NULL);
				LineTo (hdcmem, pixx - adjdy / whisrat, pixy + adjdx / whisrat);
			}
		}

		SelectObject (hdcmem, oldpen);
		DeleteObject (tpen);
	}

	void Graph::DrawGLine ()
	{
		//double eval, econ, lastecon = 0/*, inten*/;
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
	}
	void Graph::DrawPolyLine ()
	{
		////Draw polynomials.
		//double mul;
		//double val;

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
	}

	/*
	int Graph::AddXEq (std::string neweq, COLORREF color)
	{
	xeq.push_back (Line ());
	xeq.back ().eq = neweq;
	xeq.back ().color = color;

	return static_cast<int>(xeq.size ()) - 1;
	}

	int Graph::AddYEq (std::string neweq, COLORREF color)
	{
	yeq.push_back (Line ());
	yeq.back ().eq = neweq;
	yeq.back ().color = color;

	return static_cast<int>(yeq.size ()) - 1;
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

	return static_cast<int>(xpoly.size ()) - 1;
	}

	int Graph::AddYPoly (std::vector<double> coef, COLORREF color)
	{
	ypoly.push_back (Poly ());
	ypoly.back ().coef = coef;
	ypoly.back ().color = color;

	return static_cast<int>(ypoly.size ()) - 1;
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
	*/

	void Graph::ToggleSlopeArrows ()
	{
		slope_arrow = !slope_arrow;
	}

	void Graph::PrintCSV ()
	{
		static RECT client;
		GetClientRect (graphwnd, &client);

		std::ofstream out ("output.csv");
		std::vector< std::vector<double> > buffer;

		int series = pline.size () * 2 + clineY.size () * 2 + clineX.size () * 2;
		int maxpt = max (client.right, client.bottom);

		for (int a = 0; a < pline.size (); a++)
			for (int b = 0; b < pline[a].pt.size (); b++)
				maxpt = max (maxpt, pline[a].pt.size ());

		buffer.resize (maxpt, std::vector<double>(series));

		for (int a = 0; a < pline.size (); a++)
		{
			for (int b = 0; b < pline[a].pt.size (); b++)
			{
				static std::pair<double, double> pt;
				pt.first = pline[a].pt[b].first / scale + client.right / 2.0 - posx / scale;
				pt.second = pline[a].pt[b].second / -scale + client.bottom / 2.0 + posy / scale;

				buffer[b][a*2] = pline[a].pt[b].first;
				buffer[b][a*2 + 1] = pline[a].pt[b].second;
			}
		}

		static double step = 1.0, eps = 1e-4, mdely = 1e-1, val, eval, econ, lastecon = 0;
		static bool ign_last; //Ignore last point, for first point.

		for (int a = 0; a < clineY.size (); a++)
		{
			ign_last = true;
			if (!clineY[a].visible)
				continue;

			for (double b = 0; b <= client.right; b += step)
			{
				val = (double)b * scale + (posx - client.right / 2.0 * scale);
				eval = clineY[a].func (val);
				econ = -eval / scale + client.bottom / 2.0 + posy / scale;

				if (ign_last)
				{
					ign_last = false;
					continue;
				}

				if (abs (lastecon - econ) < max (client.right, client.bottom) * mdely)
				{
					buffer[b][pline.size () * 2 + a * 2] = (b + posx / scale - client.right / 2.0) * scale;
					buffer[b][pline.size () * 2 + a * 2 + 1] = eval;
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
		}

		step = 1.0;
		for (int a = 0; a < clineX.size (); a++)
		{
			ign_last = true;
			if (!clineX[a].visible)
				continue;

			for (double b = 0; b <= client.bottom; b += step)
			{
				val = (double)-b * scale + (posy + client.bottom / 2.0 * scale);
				eval = clineX[a].func (val);
				econ = eval / scale + client.right / 2.0 - posx / scale;

				if (ign_last)
				{
					ign_last = false;
					continue;
				}

				if (abs (lastecon - econ) < max (client.right, client.bottom) * mdely)
				{
					buffer[b][pline.size () * 2 + clineY.size () * 2 + a * 2] = eval;
					buffer[b][pline.size () * 2 + clineY.size () * 2 + a * 2 + 1] = (b - posy / scale - client.bottom / 2.0) * scale * -1;
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
		}

		for (int a = 0;a < buffer.size ();a++)
		{
			for (int b = 0;b < buffer[a].size ();b++)
				out << buffer[a][b] << ',';
			out << '\n';
		}

		out.close ();
	}
}