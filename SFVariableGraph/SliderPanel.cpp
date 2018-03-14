#include "SliderPanel.h"

namespace SFVG
{
	WNDPROC SliderPanel::editproc = NULL;

	SliderPanel::SliderPanel (HWND par)
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

		wndclass.lpfnWndProc = SliderPanelProc;
		wndclass.lpszClassName = "SFVGSliderPanelWnd";

		if (!RegisterClassEx (&wndclass))
		{
			MessageBox (NULL, "RegisterClassEx Slider Panel Window failed!", "", MB_OK);
			return;
		}

		spwnd = CreateWindowEx (NULL, wndclass.lpszClassName, "", WS_CHILD | WS_CLIPCHILDREN, 0, 0, 0, 0, par, NULL, wndclass.hInstance, NULL);

		if (!spwnd)
		{
			MessageBox (NULL, "CreateWindowEx Slider Panel Window failed!", "", MB_OK);
			int k = GetLastError ();
			return;
		}

		ShowWindow (spwnd, SW_SHOW);

		//Set userdata to point to this object.
		SetWindowLongPtr (spwnd, GWLP_USERDATA, (LONG_PTR)this);

		//Variables.
		bkbrush = CreateSolidBrush (RGB (0, 100, 0));

		//Get client.
		RECT client;
		GetClientRect (spwnd, &client);

		//Buttons.
		std::string btext[10] = {"Scale", "Horozontal Shift", "Vertical Shift", "Trace Eq.: r1", "Trace Eq.: r-1", "Trace Eq.: r2", "Trace Eq.: r-2", "Trace Eq.: L_T", "Trace Eq.: R_T", "Trace Eq.: D_T"};
		Rain::Button::Button::SetMessageStart (WM_SLIDERUPDATE + 1);

		for (int a = 0;a < 10;a++)
		{
			button.push_back (new Rain::Button::Button ());
			button[a]->Initialize (spwnd, btext[a], 150, 22, 12, "Consolas");
		}

		//Font.
		HDC dc = GetDC (spwnd);
		int height;
		height = -MulDiv (12, GetDeviceCaps (dc, LOGPIXELSY), 72);
		font = CreateFont (height, 0, 0, 0, 0, false, false, false, 0, 0, 0, 0, 0, "Consolas");

		//Labels.
		/*std::string ltext[3] = {"Scale", "Horozontal Shift", "Vertical Shift"};
		for (int a = 0;a < 3;a++)
		{
			labels.push_back (HWND ());
			labels[a] = CreateWindowEx (NULL, "Static", ltext[a].c_str (), WS_CHILD | WS_BORDER | SS_CENTER, 0, 0, 150, 22, spwnd, NULL, wndclass.hInstance, NULL);
			SendMessage (labels[a], WM_SETFONT, (WPARAM)font, NULL);
			ShowWindow (labels[a], SW_SHOW);
		}*/

		//Edits.
		std::string lowtext[] =	{"0.001","-6","-6","0.002","0.01","0.002","0.01","0","0","0"},
			hightext[] =		{"0.025","10","10","1","1","1","1","5","5","5"};
		WNDPROC oldproc;
		static double *var_ptr[7] = {&SFVE::r1, &SFVE::rn1, &SFVE::r2, &SFVE::rn2, &SFVE::lt, &SFVE::rt, &SFVE::dt};

		std::stringstream ss;
		for (int a = 0;a < 10;a++)
		{
			slbound.push_back (std::pair<double, double> ());

			ss << lowtext[a];
			ss >> slbound[a].first;
			ss.str (std::string ());
			ss.clear ();

			ss << hightext[a];
			ss >> slbound[a].second;
			ss.str (std::string ());
			ss.clear ();

			if (a >= 3)
				*var_ptr[a - 3] = (slbound[a].second + slbound[a].first) / 2;
		}

		for (int a = 0;a < 10;a++)
		{
			lowedit.push_back (HWND ());
			lowedit[a] = CreateWindowEx (NULL, "Edit", lowtext[a].c_str (), WS_CHILD | ES_CENTER | ES_LEFT | ES_AUTOHSCROLL, 0, 0, 50, 22, spwnd, NULL, wndclass.hInstance, NULL);
			SendMessage (lowedit[a], WM_SETFONT, (WPARAM)font, NULL);
			ShowWindow (lowedit[a], SW_SHOW);
			highedit.push_back (HWND ());
			highedit[a] = CreateWindowEx (NULL, "Edit", hightext[a].c_str (), WS_CHILD | ES_CENTER | ES_LEFT | ES_AUTOHSCROLL, 0, 0, 50, 22, spwnd, NULL, wndclass.hInstance, NULL);
			SendMessage (highedit[a], WM_SETFONT, (WPARAM)font, NULL);
			ShowWindow (highedit[a], SW_SHOW);

			//Subclass the window so we can filter keystrokes.
			oldproc = reinterpret_cast<WNDPROC>(SetWindowLongPtr (lowedit[a], GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(CustomEditProc)));
			if (editproc == NULL)
				editproc = oldproc;
			reinterpret_cast<WNDPROC>(SetWindowLongPtr (highedit[a], GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(CustomEditProc)));
		}

		//Sliders.
		for (int a = 0; a < 10; a++)
		{
			vslide.push_back (new Slider (spwnd));
			//InvalidateRect (vslide[a]->GetHWND (), &client, false);
			//UpdateWindow (vslide[a]->GetHWND ());
		}

		//Special buttons.
		bclrbifur.Initialize (spwnd, "Clear Eq. Trace", 200, 22);
		btogcubic.Initialize (spwnd, "Toggle Cubics", 200, 22);
		btogquad.Initialize (spwnd, "Toggle Quadratics", 200, 22);
		btogarrow.Initialize (spwnd, "Toggle Arrows", 200, 22);
		btogbifur.Initialize (spwnd, "Toggle Bifurcation", 200, 22);
		bbifurrot.Initialize (spwnd, "W.R.T. r1", 200, 22);

		//Bifurcation mode.
		bifur_mode = false;
		bifurwrt = 0;
		wrtvar.push_back (&SFVE::r1);
		wrtvar.push_back (&SFVE::rn1);
		wrtvar.push_back (&SFVE::r2);
		wrtvar.push_back (&SFVE::rn2);
		wrtvar.push_back (&SFVE::lt);
		wrtvar.push_back (&SFVE::rt);
		wrtvar.push_back (&SFVE::dt);
		wrtname.push_back ("r1");
		wrtname.push_back ("r-1");
		wrtname.push_back ("r2");
		wrtname.push_back ("r-2");
		wrtname.push_back ("L_T");
		wrtname.push_back ("R_T");
		wrtname.push_back ("D_T");

		ReleaseDC (spwnd, dc);
	}

	HWND SliderPanel::GetHWND ()
	{
		return spwnd;
	}

	void SliderPanel::SetGraph (Graph *newgraph)
	{
		graph = newgraph;
		graph->EditScale ((slbound[0].second + slbound[0].first) / 2);
		graph->EditCenterX ((slbound[1].second + slbound[1].first) / 2);
		graph->EditCenterY ((slbound[2].second + slbound[2].first) / 2);

		/*//Upon setting the graph, add equations to it.
		std::vector<double> coef (4, 1);
		graph->AddYPoly (coef, RGB (100, 100, 0));*/
	}

	void SliderPanel::UpdateBifurcationMode ()
	{
		double scale = graph->GetScale (), posx = graph->GetCenterX (), posy = graph->GetCenterY ();
		RECT grect;

		GetClientRect (graph->GetHWND (), &grect);

		double orig_val;
		orig_val = *wrtvar[bifurwrt];

		double lastval, curval;//, eval;
		//double val0;
		//int val1;
		//std::vector<double> xroot, yroot;
		std::vector< std::vector< std::pair<double, double> > > bifur;
		//double origdt, origrt;

		//origdt = SFVE::dt;
		//origrt = SFVE::rt;

		//uncomment to use x cubic to find roots and connect points
		////Use even steps on X axis.
		//for (double b = posx - grect.right / 2 * scale; b <= posx + grect.right / 2 * scale; b += scale * 3)
		//{
		//	*wrtvar[bifurwrt] = b;

		//	//Search for roots at this value.
		//	lastval = 0;
		//	for (double c = posy - grect.bottom / 2 * scale; c <= posy + grect.bottom / 2 * scale; c += scale)
		//	{
		//		curval = SFVE::Cubic1 (c);
		//		if (lastval / curval < 0) //Crossed axis. Binary search for exact point.
		//		{
		//			//xroot.push_back (c - scale / 200);
		//			double low = c - scale, high = c, mid = low, eps = 1e-12;
		//			while (high - low > eps)
		//			{
		//				mid = (low + high) / 2;
		//				if (lastval / SFVE::Cubic1 (mid) >= 0)
		//					low = mid;
		//				else
		//					high = mid;
		//			}
		//			xroot.push_back (mid);
		//		}
		//		lastval = curval;
		//	}

		//	//For each xroot, add the corresponding point to closest bifur line, if it exists.
		//	for (int c = 0; c < xroot.size (); c++)
		//	{
		//		//ONLY TRACK BIOLOGICAL ROOT.
		//		//if (!(xroot[c] >= 0 && xroot[c] <= min (SFVE::dt, SFVE::lt) && b >= 0 && b <= min (SFVE::rt, SFVE::lt)))
		//			//continue;

		//		val0 = 50 * scale;
		//		//val0 = 1e9;
		//		val1 = -1;
		//		for (int d = 0; d < bifur.size (); d++)
		//		{
		//			if (abs (bifur[d].back ().second - xroot[c]) + abs (bifur[d].back ().first - b) < val0)
		//			{
		//				val0 = abs (bifur[d].back ().second - xroot[c]) + abs (bifur[d].back ().first - b);
		//				val1 = d;
		//				//break;
		//			}
		//		}

		//		if (val1 != -1)
		//			bifur[val1].push_back (std::make_pair (b, xroot[c]));
		//		else
		//			bifur.push_back (std::vector< std::pair<double, double> > (1, std::make_pair (b, xroot[c])));
		//	}
		//	xroot.clear ();
		//}

		//*wrtvar[bifurwrt] = orig_val;
		//graph->ClearPLine ();
		//for (int b = 0; b < bifur.size (); b++)
		//	graph->AddPLine (bifur[b]);

		//NEW VERSION:
		//calculate points using cubic for B
		//calculate additional points by varying B, and finding value of variable that makes cubic = 0
		bifur.push_back (std::vector< std::pair<double, double> > ());
		const static int precision = 1; //the lower the better
		//for (double b = posx - grect.right / 2 * scale; b <= posx + grect.right / 2 * scale; b += scale)
		//{
		//	*wrtvar[bifurwrt] = b;

		//	lastval = 0;
		//	for (double step = scale * precision, c = posy - grect.bottom / 2 * scale; c <= posy + grect.bottom / 2 * scale; c += step)
		//	{
		//		curval = SFVE::Cubic2 (c);
		//		if (lastval / curval < 0) //Crossed axis. Binary search for exact point.
		//		{
		//			static double low, high, mid, eps =  0.5 * scale;
		//			low = c - step;
		//			high = c;
		//			while (high - low > eps)
		//			{
		//				mid = (low + high) / 2;
		//				if (lastval / SFVE::Cubic2 (mid) >= 0)
		//					low = mid;
		//				else
		//					high = mid;
		//			}
		//			bifur[0].push_back (std::make_pair (b, mid));
		//		}
		//		lastval = curval;
		//	}
		//}
		//for (double b = posy + grect.bottom / 2 * scale;b >= posy - grect.bottom / 2 * scale;b -= scale)
		//{
		//	lastval = 0;
		//	for (double step = scale * precision, c = posx - grect.right / 2 * scale; c <= posx + grect.right / 2 * scale; c += step)
		//	{
		//		*wrtvar[bifurwrt] = c;
		//		curval = SFVE::Cubic2 (b);
		//		if (lastval / curval < 0) //Crossed axis. Binary search for exact point.
		//		{
		//			static double low, high, mid, eps = 0.5 * scale;
		//			low = c - step;
		//			high = c;
		//			while (high - low > eps)
		//			{
		//				mid = (low + high) / 2;
		//				*wrtvar[bifurwrt] = mid;
		//				if (lastval / SFVE::Cubic2 (b) >= 0)
		//					low = mid;
		//				else
		//					high = mid;
		//			}
		//			bifur[0].push_back (std::make_pair (mid, b));
		//		}
		//		lastval = curval;
		//	}
		//}
		for (double b = posx - grect.right / 2 * scale; b <= posx + grect.right / 2 * scale; b += scale)
		{
			*wrtvar[bifurwrt] = b;

			lastval = 0;
			for (double step = scale * precision, c = posy - grect.bottom / 2 * scale; c <= posy + grect.bottom / 2 * scale; c += step)
			{
				curval = SFVE::Cubic2 (c);
				if (lastval / curval < 0) //Crossed axis, assume middle of last step and this one
					bifur[0].push_back (std::make_pair (b, c - step / 2));
				lastval = curval;
			}
		}
		for (double b = posy + grect.bottom / 2 * scale;b >= posy - grect.bottom / 2 * scale;b -= scale)
		{
			lastval = 0;
			for (double step = scale * precision, c = posx - grect.right / 2 * scale; c <= posx + grect.right / 2 * scale; c += step)
			{
				*wrtvar[bifurwrt] = c;
				curval = SFVE::Cubic2 (b);
				if (lastval / curval < 0) //Crossed axis, assume middle of last step and this one
					bifur[0].push_back (std::make_pair (c - step / 2, b));
				lastval = curval;
			}
		}

		*wrtvar[bifurwrt] = orig_val;
		graph->ClearPLine ();
		graph->AddPLine (bifur[0]);
	}

	LRESULT CALLBACK SliderPanelProc (_In_ HWND hwnd, _In_ UINT umsg, _In_ WPARAM wparam, _In_ LPARAM lparam)
	{
		SliderPanel *cur = (SliderPanel *)GetWindowLongPtr (hwnd, GWLP_USERDATA);

		if (cur == NULL)
			return DefWindowProc (hwnd, umsg, wparam, lparam);

		switch (umsg)
		{
			case WM_MOVE:
				return cur->OnMove (wparam, lparam);

			case WM_PAINT:
				return cur->OnPaint (wparam, lparam);

			case WM_DESTROY:
				return cur->OnDestroy (wparam, lparam);

			case WM_SLIDERUPDATE:
				return cur->OnSliderUpdate (wparam, lparam);

			case WM_COMMAND:
			{
				if (HIWORD (wparam) == EN_CHANGE)
					return cur->OnEnUpdate (wparam, lparam);
				break;
			}
				
			default:
				break;
		}

		if (umsg == (Rain::Button::Button::msg_beg + Rain::Button::Button::WM_BUTTONPRESS))
			return cur->OnButtonPress (wparam, lparam);

		return DefWindowProc (hwnd, umsg, wparam, lparam);
	}

	LRESULT SliderPanel::OnMove (WPARAM wparam, LPARAM lparam)
	{
		RECT client, rect;
		GetClientRect (spwnd, &client);

		for (int a = 0;a < vslide.size ();a++)
		{
			MoveWindow (vslide[a]->GetHWND (), 100, 40 + 70 * a, client.right - 200, 10, true);
			GetClientRect (vslide[a]->GetHWND (), &rect);
			InvalidateRect (vslide[a]->GetHWND (), &rect, false);
			UpdateWindow (vslide[a]->GetHWND ());
		}

		for (int a = 0; a < button.size (); a++)
		{
			button[a]->Show (client.right / 2 - 150 / 2, 40 + 70 * a - 30);
			GetClientRect (button[a]->GetHWND (), &rect);
			InvalidateRect (button[a]->GetHWND (), &rect, false);
			UpdateWindow (button[a]->GetHWND ());
		}
		/*
		for (int a = 0; a < labels.size (); a++)
		{
			MoveWindow (labels[a], client.right / 2 - 150 / 2, 40 + 70 * a - 30, 150, 22, true);
			GetClientRect (labels[a], &rect);
			InvalidateRect (labels[a], &rect, false);
			UpdateWindow (labels[a]);
			ShowWindow (labels[a], SW_SHOW);
		}*/

		for (int a = 0; a < lowedit.size (); a++)
		{
			MoveWindow (lowedit[a], 30, 64 + 70 * a - 30, 50, 22, true);
			GetClientRect (lowedit[a], &rect);
			InvalidateRect (lowedit[a], &rect, false);
			UpdateWindow (lowedit[a]);
			MoveWindow (highedit[a], client.right - 50 - 30, 64 + 70 * a - 30, 50, 22, true);
			GetClientRect (highedit[a], &rect);
			InvalidateRect (highedit[a], &rect, false);
			UpdateWindow (highedit[a]);
		}

		bclrbifur.Show ((client.right - 200 * 2) / 3, 70 * 10 + 40 - 30);
		btogcubic.Show ((client.right - 200 * 2) / 3, 70 * 10 + 70 - 30);
		btogquad.Show ((client.right - 200 * 2) / 3, 70 * 10 + 100 - 30);
		btogarrow.Show ((client.right - 200 * 2) * 2 / 3 + 200, 70 * 10 + 40 - 30);
		btogbifur.Show ((client.right - 200 * 2) * 2 / 3 + 200, 70 * 10 + 70 - 30);

		if (bifur_mode)
			bbifurrot.Show ((client.right - 200 * 2) * 2 / 3 + 200, 70 * 10 + 100 - 30);
		else
			bbifurrot.Hide ();

		return 0;
	}

	LRESULT SliderPanel::OnPaint (WPARAM wparam, LPARAM lparam)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint (spwnd, &ps);

		RECT client;
		GetClientRect (spwnd, &client);

		FillRect (hdc, &client, bkbrush);

		EndPaint (spwnd, &ps);

		return 0;
	}

	LRESULT SliderPanel::OnDestroy (WPARAM wparam, LPARAM lparam)
	{
		DeleteObject (bkbrush);
		DeleteObject (font);

		return 0;
	}

	LRESULT SliderPanel::OnSliderUpdate (WPARAM wparam, LPARAM lparam)
	{
		const static double PI = 3.14415926;

		for (int a = 0;a < vslide.size ();a++)
		{
			if ((Slider *)lparam == vslide[a])
			{
				//std::stringstream ss;

				//ss << std::fixed << std::setprecision (15) << vslide[a]->GetPos () << "*x";

				/*std::vector<double> coef (4);
				coef[0] = 0;
				coef[1] = 0.5;
				coef[2] = 3;
				coef[3] = vslide[a]->GetPos ();

				graph->EditYPoly (0, coef, RGB (255, 0, 0));

				RECT client;
				GetClientRect (graph->GetHWND (), &client);
				InvalidateRect (graph->GetHWND (), &client, false);
				UpdateWindow (graph->GetHWND ());*/

				/*if (a == 0) //scale
				{
					double pos = vslide[a]->GetPos ();
					const static double mul = 0.01;

					graph->EditScale (tan ((pos + 1.01) / 2 * PI / 2) * mul);
				}
				if (a == 1) //xpos
				{
					double pos = vslide[a]->GetPos ();
					const static double mul = 1;

					if (pos >= 0)
						graph->EditCenterX ((1 - 1 / (1.00001 - pos)) * mul);
					if (pos < 0)
						graph->EditCenterX ((-1 + 1 / (1.00001 + pos)) * mul);
				}
				if (a == 2) //ypos
				{
					double pos = vslide[a]->GetPos ();
					const static double mul = 1;

					if (pos >= 0)
						graph->EditCenterY ((1 - 1 / (1.00001 - pos)) * mul);
					if (pos < 0)
						graph->EditCenterY ((-1 + 1 / (1.00001 + pos)) * mul);
				}
				if (a == 3) //r_1
				{
					const static double low = 0, high = 10;
					SFVE::r1 = low + (vslide[a]->GetPos () + 1) / 2 * (high - low);
				}
				if (a == 4) //r_-1
				{
					const static double low = 0, high = 10;
					SFVE::rn1 = low + (vslide[a]->GetPos () + 1) / 2 * (high - low);
				}
				if (a == 5) //r_2
				{
					const static double low = 0, high = 10;
					SFVE::r2 = low + (vslide[a]->GetPos () + 1) / 2 * (high - low);
				}
				if (a == 6) //r_-2
				{
					const static double low = 0, high = 10;
					SFVE::rn2 = low + (vslide[a]->GetPos () + 1) / 2 * (high - low);
				}
				if (a == 7) //L_T
				{
					const static double low = 0, high = 15;
					SFVE::lt = low + (vslide[a]->GetPos () + 1) / 2 * (high - low);
				}
				if (a == 8) //R_T
				{
					const static double low = 0, high = 15;
					SFVE::rt = low + (vslide[a]->GetPos () + 1) / 2 * (high - low);
				}
				if (a == 9) //D_T
				{
					const static double low = 0, high = 15;
					SFVE::dt = low + (vslide[a]->GetPos () + 1) / 2 * (high - low);
				}*/

				double new_val = slbound[a].first + (vslide[a]->GetPos () + 1) / 2 * (slbound[a].second - slbound[a].first);
				if (a == 0)
					graph->EditScale (new_val);
				if (a == 1)
					graph->EditCenterX (new_val);
				if (a == 2)
					graph->EditCenterY (new_val);
				if (a == 3)
					SFVE::r1 = new_val;
				if (a == 4)
					SFVE::rn1 = new_val;
				if (a == 5)
					SFVE::r2 = new_val;
				if (a == 6)
					SFVE::rn2 = new_val;
				if (a == 7)
					SFVE::lt = new_val;
				if (a == 8)
					SFVE::rt = new_val;
				if (a == 9)
					SFVE::dt = new_val;

				//If variable changed, changed corresponding button label.
				if (a >= 3 && a <= 9)
					;

				//If we are in bifurcation mode, update the bifurcation diagram as well.
				if (bifur_mode)
					UpdateBifurcationMode ();

				RECT client;
				GetClientRect (graph->GetHWND (), &client);
				InvalidateRect (graph->GetHWND (), &client, false);
				UpdateWindow (graph->GetHWND ());
			}
		}

		return 0;
	}

	LRESULT SliderPanel::OnEnUpdate (WPARAM wparam, LPARAM lparam)
	{
		return 0;
	}

	LRESULT SliderPanel::OnButtonPress (WPARAM wparam, LPARAM lparam)
	{
		static double *var_ptr[7] = {&SFVE::r1, &SFVE::rn1, &SFVE::r2, &SFVE::rn2, &SFVE::lt, &SFVE::rt, &SFVE::dt};
		double scale = graph->GetScale (), posx = graph->GetCenterX (), posy = graph->GetCenterY ();
		RECT grect;

		GetClientRect (graph->GetHWND (), &grect);
		for (int a = 0;a < button.size ();a++)
		{
			if (button[a] == (Rain::Button::Button *)wparam)
			{
				if (a >= 3)
				{
					a -= 3; //Change for bifurcation buttons.

					double orig_val;
					orig_val = *var_ptr[a];
					/*
					//Range that variable through its bounds, and keep track of intersections between the graph's custom functions. On each step, connect the closest intersections together.
					//Instead of tracking intersections, track roots of the cubics we have mathematically deduced.
					//Build vectors of x and y coords of roots of two cubics, over change in var.
					std::vector< std::vector<double> > xroot, yroot;
					double lastval = 0, curval;
					*var_ptr[a] = slbound[a + 3].first;
					for (double c = posx - grect.right / 2 * scale;c <= posx + grect.right / 2 * scale;c += scale)
					{
						curval = graph->CustomYFunction (1, c);
						if (lastval / curval < 0) //Crossed axis.
							xroot.push_back (std::vector<double>(1, c - scale / 2));
						lastval = curval;
					}

					for (double c = posy - grect.bottom / 2 * scale; c <= posy + grect.bottom / 2 * scale; c += scale)
					{
						curval = graph->CustomXFunction (1, c);
						if (lastval / curval < 0) //Crossed axis.
							yroot.push_back (std::vector<double> (1, c - scale / 2));
						lastval = curval;
					}

					static const int steps = 100;
					for (double b = slbound[a + 3].first + (slbound[a + 3].second - slbound[a + 3].first) / steps;b <= slbound[a + 3].second;b += (slbound[a + 3].second - slbound[a + 3].first) / steps)
					{
						*var_ptr[a] = b;

						//Find the new roots, and match them with closest sequence. If none, create new sequence.
					}

					//Combine
					*/
					/*
					//Find intersections while ranging variable.
					static const int steps = 1000;
					static const double eps = 3;
					double val[2];
					std::vector< std::vector< std::pair<double, double> > > bifur, tbifur;
					std::vector<int> lastsize;
					std::pair<double, double> tp;
					bool flag;
					for (double b = slbound[a + 3].first; b <= slbound[a + 3].second; b += (slbound[a + 3].second - slbound[a + 3].first) / steps)
					{
						*var_ptr[a] = b;

						for (double c = posx - grect.right / 2 * scale; c <= posx + grect.right / 2 * scale; c += scale / 2)
						{
							val[0] = graph->CustomYFunction (0, c);
							val[1] = graph->CustomXFunction (0, val[0]);

							if (abs (c - val[1]) < eps * scale)
							{
								//Intersection found for this b at tp. Insert it into closest tbifur sequence.
								tp.first = (c + val[1]) / 2;
								tp.second = val[0];
								flag = false;
								for (int d = 0;d < tbifur.size ();d++)
								{
									if (abs (tbifur[d].back ().first - tp.first) < scale)
									{
										tbifur[d].push_back (tp);
										flag = true;
										break;
									}
								}

								if (!flag) //Create new tbifur sequence.
									tbifur.push_back (std::vector< std::pair<double, double> > (1, tp));
							}
						}

						//Everytime through above loop, reduce to one point addition at most to each tbifur seq.
						for (int c = 0;c < tbifur.size ();c++)
						{
							tp.first = tp.second = 0;
							flag = false;
							for (int d = 0; d < tbifur[c].size (); d++)
							{
								tp.first += tbifur[c][d].first;
								tp.second += tbifur[c][d].second;
								flag = true;
							}
							tp.first /= (tbifur[c].size ());
							tp.second /= (tbifur[c].size ());
							while (tbifur[c].size () != 0)
								tbifur[c].pop_back ();
							if (flag)
								tbifur[c].push_back (tp);
						}

						//For each tbifur point, add to closest bifur seq.
						for (int c = 0;c < tbifur.size ();c++)
						{
							val[0] = 20 * scale;
							val[1] = -1;
							for (int d = 0;d < bifur.size ();d++)
							{
								if (abs (bifur[d].back ().first - tbifur[c][0].first) < val[0])
								{
									val[0] = abs (bifur[d].back ().first - tbifur[c][0].first);
									val[1] = d;
								}
							}

							if (val[1] != -1)
								bifur[val[1]].push_back (tbifur[c][0]);
							else
								bifur.push_back (std::vector< std::pair<double, double> > (1, tbifur[c][0]));
						}

						tbifur.clear ();
					}
					*/

					//Use cubic to find xroots, then use xroots to build bifur seq.
					static const int steps = 1000;
					double lastval, curval, eval;
					double val0;
					int val1;
					std::vector<double> xroot;
					std::vector< std::vector< std::pair<double, double> > > bifur;
					int scalek = 1;
					//Use more steps at small values, they cause bigger changes.
					for (double b = slbound[a + 3].first; b <= slbound[a + 3].second; b += (slbound[a + 3].second - slbound[a + 3].first) * scalek / ((steps + 1) * steps / 2), scalek++)
					{
						*var_ptr[a] = b;
						lastval = 0;
						for (double c = posx - grect.right / 2 * scale; c <= posx + grect.right / 2 * scale; c += scale)
						{
							curval = SFVE::Cubic2 (c);
							if (lastval / curval < 0) //Crossed axis. Binary search for exact point.
							{
								//xroot.push_back (c - scale / 200);
								double low = c - scale, high = c, mid = low, eps = 1e-12;
								while (high - low > eps)
								{
									mid = (low + high) / 2;
									if (lastval / SFVE::Cubic2 (mid) >= 0)
										low = mid;
									else
										high = mid;
								}
								xroot.push_back (mid);
							}
							lastval = curval;
						}

						//For each xroot, add the corresponding point to closest bifur, if it exists.
						for (int c = 0; c < xroot.size (); c++)
						{
							val0 = 20 * scale;
							val1 = -1;
							eval = SFVE::DT2 (xroot[c]);
							for (int d = 0; d < bifur.size (); d++)
							{
								if (abs (bifur[d].back ().first - xroot[c]) + abs (bifur[d].back ().second - eval) < val0)
								{
									val0 = abs (bifur[d].back ().first - xroot[c]) + abs (bifur[d].back ().second - eval);
									val1 = d;
								}
							}

							if (val1 != -1)
								bifur[val1].push_back (std::make_pair (xroot[c], eval));
							else
								bifur.push_back (std::vector< std::pair<double, double> > (1, std::make_pair (xroot[c], eval)));
						}
						xroot.clear ();
					}

					*var_ptr[a] = orig_val;
					graph->ClearPLine ();
					for (int b = 0;b < bifur.size ();b++)
						graph->AddPLine (bifur[b]);
					InvalidateRect (graph->GetHWND (), &grect, false);
					UpdateWindow (graph->GetHWND ());

					a += 3; //Change back.
				}
				else //a <= 2, scale and shift buttons
				{
					if (a == 0) //Scale
					{
						graph->EditCenterX (SFVE::RTLine (0) / 2);
						graph->EditCenterY (SFVE::DTLine (0) / 2);
						if (SFVE::RTLine (0) / grect.right > SFVE::DTLine (0) / grect.bottom)
							graph->EditScale (SFVE::RTLine (0) / grect.right);
						else
							graph->EditScale (SFVE::DTLine (0) / grect.bottom);
						InvalidateRect (graph->GetHWND (), &grect, false);
						UpdateWindow (graph->GetHWND ());
					}
					else if (a == 1)
					{
						graph->EditCenterX (SFVE::RTLine (0) / 2);
						graph->EditScale (SFVE::RTLine (0) / grect.right);
						InvalidateRect (graph->GetHWND (), &grect, false);
						UpdateWindow (graph->GetHWND ());
					}
					else if (a == 2)
					{
						graph->EditCenterY (SFVE::DTLine (0) / 2);
						graph->EditScale (SFVE::DTLine (0) / grect.bottom);
						InvalidateRect (graph->GetHWND (), &grect, false);
						UpdateWindow (graph->GetHWND ());
					}
				}
			}
		}

		//Specials.
		if (&bclrbifur == (Rain::Button::Button *)wparam)
		{
			graph->ClearPLine ();
			InvalidateRect (graph->GetHWND (), &grect, false);
			UpdateWindow (graph->GetHWND ());
		}
		if (&btogcubic == (Rain::Button::Button *)wparam)
		{
			graph->ToggleCLineX (1);
			graph->ToggleCLineY (1);
			InvalidateRect (graph->GetHWND (), &grect, false);
			UpdateWindow (graph->GetHWND ());
		}
		if (&btogquad == (Rain::Button::Button *)wparam)
		{
			graph->ToggleCLineX (0);
			graph->ToggleCLineY (0);
			InvalidateRect (graph->GetHWND (), &grect, false);
			UpdateWindow (graph->GetHWND ());
		}
		if (&btogarrow == (Rain::Button::Button *)wparam)
		{
			graph->ToggleSlopeArrows ();
			InvalidateRect (graph->GetHWND (), &grect, false);
			UpdateWindow (graph->GetHWND ());
		}
		if (&btogbifur == (Rain::Button::Button *)wparam)
		{
			bifur_mode = !bifur_mode;
			graph->ClearPLine ();
			graph->DisableAllCLine ();

			if (!bifur_mode)
			{
				graph->DisableAllCLine ();
				graph->ToggleCLineX (0);
				graph->ToggleCLineY (0);
				graph->ToggleCLineX (1);
				graph->ToggleCLineY (1);
				graph->ToggleCLineX (2);
				graph->ToggleCLineY (2);
				
				bbifurrot.Hide ();
			}
			else
			{
				//graph->ToggleCLineX (2);
				//graph->ToggleCLineY (2);

				static RECT client;
				GetClientRect (spwnd, &client);
				bbifurrot.Show ((client.right - 200 * 2) * 2 / 3 + 200, 70 * 10 + 100 - 30);
				UpdateBifurcationMode ();
			}

			InvalidateRect (graph->GetHWND (), &grect, false);
			UpdateWindow (graph->GetHWND ());
		}
		if (&bbifurrot == (Rain::Button::Button *)wparam)
		{
			static RECT brect;
			GetClientRect (bbifurrot.GetHWND (), &brect);

			bifurwrt = (bifurwrt + 1 ) % 7;
			bbifurrot.SetText ("W.R.T. " + wrtname[bifurwrt]);

			InvalidateRect (bbifurrot.GetHWND (), &brect, false);
			UpdateWindow (bbifurrot.GetHWND ());

			UpdateBifurcationMode ();
			InvalidateRect (graph->GetHWND (), &grect, false);
			UpdateWindow (graph->GetHWND ());
		}

		return 0;
	}

	LRESULT SliderPanel::EditEnterUpdate (HWND edit)
	{
		char buffer[20];
		GetWindowText (edit, buffer, 20);
		std::stringstream ss;
		ss << buffer;
		double new_val;
		ss >> new_val;
		double orig_val;

		for (int a = 0; a < lowedit.size (); a++)
		{
			if (lowedit[a] == edit)
			{
				orig_val = slbound[a].first + (vslide[a]->GetPos () + 1) / 2 * (slbound[a].second - slbound[a].first);
				if (new_val != slbound[a].second)
					slbound[a].first = new_val;
				vslide[a]->UpdatePos ((orig_val - slbound[a].first) / (slbound[a].second - slbound[a].first) * 2 - 1);
			}
		}
		for (int a = 0; a < highedit.size (); a++)
		{
			if (highedit[a] == edit)
			{
				orig_val = slbound[a].first + (vslide[a]->GetPos () + 1) / 2 * (slbound[a].second - slbound[a].first);
				if (new_val != slbound[a].first)
					slbound[a].second = new_val;
				vslide[a]->UpdatePos ((orig_val - slbound[a].first) / (slbound[a].second - slbound[a].first) * 2 - 1);
			}
		}
		return 0;
	}

	LRESULT CALLBACK CustomEditProc (_In_ HWND hwnd, _In_ UINT umsg, _In_ WPARAM wparam, _In_ LPARAM lparam)
	{
		if (umsg == WM_CHAR)
		{
			// Make sure we only allow specific characters
			if (!((wparam >= '0' && wparam <= '9')
				|| wparam == '.'
				|| wparam == '-'
				|| wparam == VK_RETURN
				|| wparam == VK_DELETE
				|| wparam == VK_BACK))
			{
				return 0;
			}

			if (wparam == VK_RETURN) //Call udpate function here.
			{
				HWND par = (HWND)GetWindowLongPtr (hwnd, GWLP_HWNDPARENT);
				SliderPanel *cur = (SliderPanel *)GetWindowLongPtr (par, GWLP_USERDATA);
				cur->EditEnterUpdate (hwnd);
			}
		}

		return CallWindowProc (SliderPanel::editproc, hwnd, umsg, wparam, lparam);
	}

	void SliderPanel::IdleChVar (int var)
	{
		static const double step = 0.004;
		static const double EPS = step * 1.1;
		static int dir = 1;
		double pos;

		pos = vslide[var + 3]->GetPos ();
		if (pos < EPS - 1 || pos > 1.0 - EPS)
			dir *= -1;

		vslide[var + 3]->UpdatePos (pos + dir * step);
	}
}