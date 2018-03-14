#pragma once

#include "InfoPanel.h"
#include "Equations.h"
#include "SFVE.h" //variables
#include <vector>
#include <fstream>
#include <Windows.h>

namespace SFVG
{
	class InfoPanel;

	class Graph
	{
		public:
			typedef double (*CLinePtr) (double);

			Graph (HWND par);
			HWND GetHWND ();

			void EditScale (double newscale);
			void EditCenterX (double newx);
			void EditCenterY (double newy);
			double GetScale ();
			double GetCenterX ();
			double GetCenterY ();

			void ClearPLine (); //Point lines.
			void AddPLine (std::vector< std::pair<double, double> > &pvec);
			int AddCLineX (CLinePtr func, COLORREF color, int thick = 2, int style = PS_SOLID);
			int AddCLineY (CLinePtr func, COLORREF color, int thick = 2, int style = PS_SOLID);
			void ToggleCLineX (int x);
			void ToggleCLineY (int x);
			void DisableAllCLine ();
			void PopCLineX ();
			void PopCLineY ();

			void DrawPLine ();
			void DrawCLine ();
			
			int AddXEq (std::string neweq, COLORREF color);
			int AddYEq (std::string neweq, COLORREF color);/*
			void EditXEq (int id, std::string neweq, COLORREF color);
			void EditYEq (int id, std::string neweq, COLORREF color);

			int AddXPoly (std::vector<double> coef, COLORREF color);
			int AddYPoly (std::vector<double> coef, COLORREF color);
			void EditXPoly (int id, std::vector<double> coef, COLORREF color);
			void EditYPoly (int id, std::vector<double> coef, COLORREF color);
			*/

			void DrawGLine (); //Draw general lines.
			void DrawPolyLine (); //Draw polynomials.

			//std::vector<bool> cusyshow, cusxshow;

			//Science Fair Specific Functions.
			void ToggleSlopeArrows ();
			void DrawSlopeArrows ();

			//Print graph data to output.csv
			void PrintCSV ();

		private:
			/*
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

			std::vector<Line> xeq, yeq;
			std::vector<Poly> xpoly, ypoly;
			std::vector<COLORREF> cuscolorx, cuscolory;
			*/

			struct PLine //Lines defined by points.
			{
				std::vector< std::pair<double, double> > pt;
			};

			struct CLine //Custom function lines.
			{
				COLORREF color;
				CLinePtr func;
				int thick;
				int style;
				bool visible;
			};

			std::vector<PLine> pline;
			std::vector<CLine> clineX;
			std::vector<CLine> clineY;

			double posx, posy, scale; //Units per pixel.
			bool slope_arrow;

			HWND graphwnd;
			HDC hwnddc, hdcmem;
			HBITMAP hbmmem, hbmold;
			HBRUSH whitebrush, oldbrush;
			HPEN tpen, oldpen;

			InfoPanel *labels;

			LRESULT OnPaint (WPARAM wparam, LPARAM lparam);
			LRESULT OnDestroy (WPARAM wparam, LPARAM lparam);
			LRESULT OnSize (WPARAM wparam, LPARAM lparam);

		friend LRESULT CALLBACK GraphWndProc (_In_ HWND hwnd, _In_ UINT umsg, _In_ WPARAM wparam, _In_ LPARAM lparam);
	};
}