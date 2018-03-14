#pragma once

#include "Utility.h"
#include <string>
#include <Windowsx.h>
#include <Windows.h>

namespace SFVG
{

#define WM_SLIDERUPDATE WM_APP

	class Slider
	{
		public:
			Slider (HWND par);
			HWND GetHWND ();
			double GetPos ();
			void UpdatePos (double new_pos);

			static int id;

		private:
			HWND sliderwnd;
			COLORREF bkgdcolor, slcolor;
			HDC hwnddc, hdcmem;
			HBITMAP hbmmem, hbmold;
			HPEN oldpen, bkpen, slpen;
			HBRUSH oldbrush, bkbrush, slbrush;
			double pos, origpos;
			bool dragging;
			int origx;

			LRESULT OnPaint (WPARAM wparam, LPARAM lparam);
			LRESULT OnMouseMove (WPARAM wparam, LPARAM lparam);
			LRESULT OnLButtonDown (WPARAM wparam, LPARAM lparam);
			LRESULT OnLButtonUp (WPARAM wparam, LPARAM lparam);
			LRESULT OnDestroy (WPARAM wparam, LPARAM lparam);
			LRESULT OnSize (WPARAM wparam, LPARAM lparam);

		friend LRESULT CALLBACK SliderProc (HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
	};
}