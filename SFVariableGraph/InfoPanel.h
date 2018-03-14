#pragma once

#include "Graph.h"
#include "Utility.h"
#include "SFVE.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <Windows.h>

namespace SFVG
{
	class Graph;

	class InfoPanel
	{
		public:
			InfoPanel (HWND par);

			HWND GetHWND ();
			void UpdateInfo ();
			void SetGraph (Graph *new_graph);

		private:
			HWND ipwnd;
			HFONT font, oldfont;
			HDC hwnddc, hdcmem;
			HBITMAP hbmmem, hbmold;
			HBRUSH whitebrush, oldbrush;
			HPEN tpen, oldpen;

			Graph *graph;

			std::string label[5];

			LRESULT OnPaint (WPARAM wparam, LPARAM lparam);
			LRESULT OnDestroy (WPARAM wparam, LPARAM lparam);

		friend LRESULT CALLBACK InfoPanelProc (_In_ HWND hwnd, _In_ UINT umsg, _In_ WPARAM wparam, _In_ LPARAM lparam);
	};
}