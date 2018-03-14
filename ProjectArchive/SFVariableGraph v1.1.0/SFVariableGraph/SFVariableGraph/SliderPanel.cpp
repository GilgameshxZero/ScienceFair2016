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
		bkbrush = CreateSolidBrush (RGB (200, 200, 200));

		//Get client.
		RECT client;
		GetClientRect (spwnd, &client);

		//Buttons.
		std::string btext[7] = {"Bifurcate r1", "Bifurcate r-1", "Bifurcate r2", "Bifurcate r-2", "Bifurcate L_T", "Bifurcate R_T", "Bifurcate D_T"};
		Rain::Button::Button::SetMessageStart (WM_SLIDERUPDATE + 1);

		for (int a = 0;a < 7;a++)
		{
			button.push_back (new Rain::Button::Button ());
			button[a]->Initialize (spwnd, btext[a], 150, 22, 12, "Consolas");
		}

		//Labels.
		std::string ltext[3] = {"Scale", "Horozontal Shift", "Vertical Shift"};
		HDC dc = GetDC (spwnd);
		int height;
		height = -MulDiv (12, GetDeviceCaps (dc, LOGPIXELSY), 72);
		font = CreateFont (height, 0, 0, 0, 0, false, false, false, 0, 0, 0, 0, 0, "Consolas");
		for (int a = 0;a < 3;a++)
		{
			labels.push_back (HWND ());
			labels[a] = CreateWindowEx (NULL, "Static", ltext[a].c_str (), WS_CHILD | WS_BORDER | SS_CENTER, 0, 0, 150, 22, spwnd, NULL, wndclass.hInstance, NULL);
			SendMessage (labels[a], WM_SETFONT, (WPARAM)font, NULL);
			ShowWindow (labels[a], SW_SHOW);
		}

		//Edits.
		std::string lowtext[] =	{"0.001","-6","-6","0.002","0.01","0.002","0.01","0","0","0"},
			hightext[] =		{"0.025","10","10","1","1","1","1","5","5","5"};
		WNDPROC oldproc;
		static double *var_ptr[7] = {&SFVars::r1, &SFVars::rn1, &SFVars::r2, &SFVars::rn2, &SFVars::lt, &SFVars::rt, &SFVars::dt};

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
		bclrbifur.Initialize (spwnd, "Clear Bifurcations", 200, 22);
		btogcubic.Initialize (spwnd, "Toggle Cubics", 200, 22);
		btogquad.Initialize (spwnd, "Toggle Quadratics", 200, 22);

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
			button[a]->Show (client.right / 2 - 150 / 2, 70 * 3 + 40 + 70 * a - 30);
			GetClientRect (button[a]->GetHWND (), &rect);
			InvalidateRect (button[a]->GetHWND (), &rect, false);
			UpdateWindow (button[a]->GetHWND ());
		}

		for (int a = 0; a < labels.size (); a++)
		{
			MoveWindow (labels[a], client.right / 2 - 150 / 2, 40 + 70 * a - 30, 150, 22, true);
			GetClientRect (labels[a], &rect);
			InvalidateRect (labels[a], &rect, false);
			UpdateWindow (labels[a]);
			ShowWindow (labels[a], SW_SHOW);
		}

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

		bclrbifur.Show (client.right / 2 - 200 / 2, 70 * 10 + 40 - 30);
		btogcubic.Show (client.right / 2 - 200 / 2, 70 * 10 + 70 - 30);
		btogquad.Show (client.right / 2 - 200 / 2, 70 * 10 + 100 - 30);

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
		const static int PI = 3.14415926;

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
					SFVars::r1 = low + (vslide[a]->GetPos () + 1) / 2 * (high - low);
				}
				if (a == 4) //r_-1
				{
					const static double low = 0, high = 10;
					SFVars::rn1 = low + (vslide[a]->GetPos () + 1) / 2 * (high - low);
				}
				if (a == 5) //r_2
				{
					const static double low = 0, high = 10;
					SFVars::r2 = low + (vslide[a]->GetPos () + 1) / 2 * (high - low);
				}
				if (a == 6) //r_-2
				{
					const static double low = 0, high = 10;
					SFVars::rn2 = low + (vslide[a]->GetPos () + 1) / 2 * (high - low);
				}
				if (a == 7) //L_T
				{
					const static double low = 0, high = 15;
					SFVars::lt = low + (vslide[a]->GetPos () + 1) / 2 * (high - low);
				}
				if (a == 8) //R_T
				{
					const static double low = 0, high = 15;
					SFVars::rt = low + (vslide[a]->GetPos () + 1) / 2 * (high - low);
				}
				if (a == 9) //D_T
				{
					const static double low = 0, high = 15;
					SFVars::dt = low + (vslide[a]->GetPos () + 1) / 2 * (high - low);
				}*/

				double new_val = slbound[a].first + (vslide[a]->GetPos () + 1) / 2 * (slbound[a].second - slbound[a].first);
				if (a == 0)
					graph->EditScale (new_val);
				if (a == 1)
					graph->EditCenterX (new_val);
				if (a == 2)
					graph->EditCenterY (new_val);
				if (a == 3)
					SFVars::r1 = new_val;
				if (a == 4)
					SFVars::rn1 = new_val;
				if (a == 5)
					SFVars::r2 = new_val;
				if (a == 6)
					SFVars::rn2 = new_val;
				if (a == 7)
					SFVars::lt = new_val;
				if (a == 8)
					SFVars::rt = new_val;
				if (a == 9)
					SFVars::dt = new_val;

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
		static double *var_ptr[7] = {&SFVars::r1, &SFVars::rn1, &SFVars::r2, &SFVars::rn2, &SFVars::lt, &SFVars::rt, &SFVars::dt};
		double scale = graph->GetScale (), posx = graph->GetCenterX (), posy = graph->GetCenterY ();
		RECT grect;

		GetClientRect (graph->GetHWND (), &grect);
		for (int a = 0;a < button.size ();a++)
		{
			if (button[a] == (Rain::Button::Button *)wparam)
			{
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
				double val[2];
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
						curval = graph->CustomYFunction (1, c);
						if (lastval / curval < 0) //Crossed axis. Binary search for exact point.
						{
							//xroot.push_back (c - scale / 200);
							double low = c - scale, high = c, mid = low, eps = 1e-12;
							while (high - low > eps)
							{
								mid = (low + high) / 2;
								if (lastval / graph->CustomYFunction (1, mid) >= 0)
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
						val[0] = 20 * scale;
						val[1] = -1;
						eval = graph->CustomYFunction (0, xroot[c]);
						for (int d = 0; d < bifur.size (); d++)
						{
							if (abs (bifur[d].back ().first - xroot[c]) + abs (bifur[d].back ().second - eval) < val[0])
							{
								val[0] = abs (bifur[d].back ().first - xroot[c]) + abs (bifur[d].back ().second - eval);
								val[1] = d;
							}
						}

						if (val[1] != -1)
							bifur[val[1]].push_back (std::make_pair (xroot[c], eval));
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
			graph->cusxshow[1] = !graph->cusxshow[1];
			graph->cusyshow[1] = !graph->cusyshow[1];
			InvalidateRect (graph->GetHWND (), &grect, false);
			UpdateWindow (graph->GetHWND ());
		}
		if (&btogquad == (Rain::Button::Button *)wparam)
		{
			graph->cusxshow[0] = !graph->cusxshow[0];
			graph->cusyshow[0] = !graph->cusyshow[0];
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
}