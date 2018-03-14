#pragma once

#include <algorithm>
#include <Windows.h>
#include <vector>
#include "Slider.h"
#include "Graph.h"
#include "SFVE.h"
#include <sstream>
#include "Button.h"
#include <iomanip>

namespace SFVG
{
	class SliderPanel
	{
		public:
			SliderPanel (HWND par);
			HWND GetHWND ();
			void SetGraph (Graph *newgraph);

			//called by idlehandler: modify a variable by a small amount (var is 0-6)
			void IdleChVar (int var);

		private:
			HWND spwnd;
			HBRUSH bkbrush, oldbrush;
			HFONT font;
			Graph *graph;
			std::vector<Slider *> vslide;
			std::vector<Rain::Button::Button *> button;
			std::vector<HWND> labels, lowedit, highedit;
			std::vector< std::pair<double, double> > slbound;
			Rain::Button::Button bclrbifur, btogcubic, btogquad, btogarrow, btogbifur, bbifurrot;

			bool bifur_mode;
			int bifurwrt;
			std::vector<double *> wrtvar;
			std::vector<std::string> wrtname;

			static WNDPROC editproc;

			void UpdateBifurcationMode ();

			LRESULT OnMove (WPARAM wparam, LPARAM lparam);
			LRESULT OnPaint (WPARAM wparam, LPARAM lparam);
			LRESULT OnDestroy (WPARAM wparam, LPARAM lparam);
			LRESULT OnSliderUpdate (WPARAM wparam, LPARAM lparam);
			LRESULT OnEnUpdate (WPARAM wparam, LPARAM lparam);
			LRESULT OnButtonPress (WPARAM wparam, LPARAM lparam);

			LRESULT EditEnterUpdate (HWND edit);

		friend LRESULT CALLBACK SliderPanelProc (_In_ HWND hwnd, _In_ UINT umsg, _In_ WPARAM wparam, _In_ LPARAM lparam);
		friend LRESULT CALLBACK CustomEditProc (_In_ HWND hwnd, _In_ UINT umsg, _In_ WPARAM wparam, _In_ LPARAM lparam);
	};
}