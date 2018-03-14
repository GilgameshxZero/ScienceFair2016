#pragma once

#include "Equations.h"
#include "SFVars.h" //variables
#include <vector>
#include <Windows.h>

namespace SFVG
{
	class Graph
	{
		public:
			Graph (HWND par);
			HWND GetHWND ();
			
			int AddXEq (std::string neweq, COLORREF color);
			int AddYEq (std::string neweq, COLORREF color);
			void EditXEq (int id, std::string neweq, COLORREF color);
			void EditYEq (int id, std::string neweq, COLORREF color);

			int AddXPoly (std::vector<double> coef, COLORREF color);
			int AddYPoly (std::vector<double> coef, COLORREF color);
			void EditXPoly (int id, std::vector<double> coef, COLORREF color);
			void EditYPoly (int id, std::vector<double> coef, COLORREF color);

			void EditScale (double newscale);
			void EditCenterX (double newx);
			void EditCenterY (double newy);
			double GetScale ();
			double GetCenterX ();
			double GetCenterY ();

			void ClearPLine ();
			void AddPLine (std::vector< std::pair<double, double> > &pvec);

			double CustomXFunction (int id, double val); //put any non-poly functions X= here.
			double CustomYFunction (int id, double val); //put any non-poly functions Y= here.

			std::vector<bool> cusyshow, cusxshow;

		private:
			//General line.
			struct Line
			{
				std::string eq;
				COLORREF color;
			};

			//Polynomial.
			struct Poly
			{
				std::vector<double> coef;
				COLORREF color;
			};

			HWND graphwnd;
			std::vector<Line> xeq, yeq;
			std::vector<Poly> xpoly, ypoly;
			std::vector< std::vector< std::pair<double, double> > > pline;
			double posx, posy, scale;

			HDC hwnddc, hdcmem;
			HBITMAP hbmmem, hbmold;
			HBRUSH whitebrush, oldbrush;
			HPEN tpen, oldpen;

			LRESULT OnPaint (WPARAM wparam, LPARAM lparam);
			LRESULT OnDestroy (WPARAM wparam, LPARAM lparam);

			std::vector<COLORREF> cuscolorx, cuscolory;

		friend LRESULT CALLBACK GraphWndProc (_In_ HWND hwnd, _In_ UINT umsg, _In_ WPARAM wparam, _In_ LPARAM lparam);
	};
}