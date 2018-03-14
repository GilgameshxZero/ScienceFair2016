#include "Button.h"

namespace Rain
{
	namespace Button
	{
		int Button::id = 0;
		UINT Button::msg_beg = WM_APP;
		UINT Button::WM_BUTTONPRESS = 0;

		Button::Button ()
		{
			state = "Normal";
			child = NULL;
			parent = NULL;
			cx = cy = 0;
			text = "NULL";
			font = "Consolas";
			fontsize = 12;
		}
		void Button::Initialize (HWND parent, std::string text, int cx, int cy, int fontsize, std::string font)
		{
			HINSTANCE hInstance = (HINSTANCE)(GetWindowLongPtr (parent, GWLP_HINSTANCE));
			WNDCLASSEX wc;
			std::string ButtonClassName = (std::string)"RainButton" + SFVG::Utility::IntToStr (id++),
				ButtonTitleText = ButtonClassName;

			//For calculating the length and height of the button (window) (cx & cy)
			HDC hDC;
			HFONT font_temp;
			int height;

			//Create the window, initialize GWLP_USERDATA, check for errors, register the window class, and hide the window
			{
				//Create the windowclass for the button
				wc.cbSize = sizeof (WNDCLASSEX);
				wc.style = 0;
				wc.lpfnWndProc = ButtonProc;
				wc.cbClsExtra = 0;
				wc.cbWndExtra = 0;
				wc.hInstance = hInstance;
				wc.hIcon = static_cast<HICON>(LoadImage (NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
				wc.hCursor = LoadCursor (NULL, IDC_ARROW);
				wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
				wc.lpszMenuName = NULL;
				wc.lpszClassName = ButtonClassName.c_str ();
				wc.hIconSm = static_cast<HICON>(LoadImage (NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));

				//If could not register window class
				if (!RegisterClassEx (&wc))
				{
					MessageBox (NULL, (ButtonClassName + " registration failed! Error code: " + SFVG::Utility::IntToStr (static_cast<int>(GetLastError ()))).c_str (), "Error!", MB_OK | MB_ICONEXCLAMATION);
					PostQuitMessage (0);
					return;
				}

				//Create window
				this->child = CreateWindowEx (WS_EX_NOPARENTNOTIFY, ButtonClassName.c_str (), ButtonTitleText.c_str (), WS_CHILD, 0, 0, cx, cy, parent, NULL, hInstance, NULL);

				//Check for error in window creation
				if (this->child == NULL)
				{
					MessageBox (NULL, (ButtonClassName + " creation failed! Error code: " + SFVG::Utility::IntToStr (static_cast<int>(GetLastError ()))).c_str (), "Error!", MB_OK | MB_ICONEXCLAMATION);
					PostQuitMessage (0);
					return;
				}

				//Set GWLP_USERDATA as a pointer to this so that the Procedure can access this
				SetWindowLongPtr (child, GWLP_USERDATA, (LONG_PTR)(this));

				//Hide the window
				ShowWindow (this->child, SW_HIDE);
			}

			//Initialize a temporary text HDC and check for a 0 cx || cy to reinit height and length
			{
				hDC = GetDC (child);

				//Initialize the words DC to print the button text
				height = -MulDiv (this->fontsize, GetDeviceCaps (hDC, LOGPIXELSY), 72);
				font_temp = CreateFont (height, 0, 0, 0, 0, false, false, false, 0, 0, 0, 0, 0, font.c_str ());

				//If font creation failed
				if (!font_temp)
				{
					MessageBox (NULL, ("Button font creation failed! Error code: " + SFVG::Utility::IntToStr (static_cast<int>(GetLastError ()))).c_str (), "Error", MB_OK | MB_ICONEXCLAMATION);
					PostQuitMessage (0);
					return;
				}

				//Select font with HDC
				SelectObject (hDC, font_temp);

				//If the user did not specify a length and height for the button, calculate the length and width by adding 10 to that of the words
				if (cx == 0 || cy == 0)
				{
					SIZE tsize;

					GetTextExtentPoint32 (hDC, text.c_str (), static_cast<int>(text.length ()), &tsize);
					cx = tsize.cx + 10;
					cy = tsize.cy + 10;
				}
			}

			//Copy all parameters and properties to local storage
			this->parent = parent;
			this->cx = cx; //Width and height of button
			this->cy = cy;
			this->fontsize = fontsize;
			this->font = font;
			this->text = text;

			//Draw the button with "Normal"
			Draw ("Normal");

			//Delete and free objects and DC
			DeleteDC (hDC);
			DeleteObject (font_temp);

			return;
		}

		void Button::Show (int x, int y)
		{
			state = "Normal";

			MoveWindow (child, x, y, cx, cy, false);
			ShowWindow (child, SW_SHOW);
			Draw (state);
		}
		void Button::Hide ()
		{
			state = "Hidden";
			ShowWindow (child, SW_HIDE);
		}
		HWND Button::GetHWND ()
		{
			return child;
		}

		void Button::SetText (std::string new_text)
		{
			text = new_text;
		}

		void Button::SetMessageStart (UINT start)
		{
			msg_beg = start;
		}

		void Button::Draw (std::string state)
		{
			HBRUSH bk_brush = CreateSolidBrush (RGB (230, 230, 230));
			HPEN txt_pen = CreatePen (PS_SOLID, 1, RGB (0, 0, 0)), bk_pen = CreatePen (PS_SOLID, 1, RGB (230, 230, 230));
			HFONT font_temp;
			SIZE tsize;
			int radius = 3, height;

			//Off-screen DC
			RECT cRect;
			HDC hDCMem, hDC = GetDC (child);
			HBITMAP hBMMem, hBMOld;

			//Set up an off-screen DC
			{
				GetClientRect (child, &cRect);

				hDCMem = CreateCompatibleDC (hDC);
				hBMMem = CreateCompatibleBitmap (hDC, cRect.right - cRect.left, cRect.bottom - cRect.top);
				hBMOld = (HBITMAP)SelectObject (hDCMem, hBMMem);

				FillRect (hDCMem, &cRect, CreateSolidBrush (RGB (255, 255, 255)));
			}

			//Create the brushes and pens based on the state
			if (state == "Hover")
			{
				bk_brush = CreateSolidBrush (RGB (200, 200, 200));
				bk_pen = CreatePen (PS_SOLID, 1, RGB (200, 200, 200));
			}
			else if (state == "Drag")
			{
				bk_brush = CreateSolidBrush (RGB (255, 150, 150));
				bk_pen = CreatePen (PS_SOLID, 1, RGB (255, 150, 150));
			}
			else if (state == "Down")
			{
				bk_brush = CreateSolidBrush (RGB (150, 150, 255));
				bk_pen = CreatePen (PS_SOLID, 1, RGB (150, 150, 255));
			}

			//Set default for button HDC
			SelectObject (hDCMem, bk_brush);
			SelectObject (hDCMem, txt_pen);
			/* If you want rounded rectangle button.
			//Draw a rounded rectangle with borders inside
			Ellipse (hDCMem, 0, 0, 2 * radius, 2 * radius);
			Ellipse (hDCMem, cx - 2 * radius, 0, cx, 2 * radius);
			Ellipse (hDCMem, 0, cy - 2 * radius, 2 * radius, cy);
			Ellipse (hDCMem, cx - 2 * radius, cy - 2 * radius, cx, cy);

			Rectangle (hDCMem, radius, 0, cx - radius, cy);
			Rectangle (hDCMem, 0, radius, cx, cy - radius);

			//Clear the inside borders
			SelectObject (hDCMem, bk_pen);
			Rectangle (hDCMem, radius, 1, cx - radius, cy - 1);
			Rectangle (hDCMem, 1, radius, cx - 1, cy - radius);
			*/
			Rectangle (hDCMem, 0, 0, cx, cy);

			//Create font for HDC `words`
			{
				//Initialize the words DC to print the button text
				height = -MulDiv (fontsize, GetDeviceCaps (hDCMem, LOGPIXELSY), 72);
				font_temp = CreateFont (height, 0, 0, 0, 0, false, false, false, 0, 0, 0, 0, 0, font.c_str ());

				//If font creation failed
				if (!font_temp)
				{
					MessageBox (NULL, ("Button font creation failed! Error code: " + SFVG::Utility::IntToStr (static_cast<int>(GetLastError ()))).c_str (), "Error", MB_OK | MB_ICONEXCLAMATION);
					PostQuitMessage (0);
					return;
				}

				//Select objects with HDC
				SetBkColor (hDCMem, RGB (255, 255, 255));
				SetTextColor (hDCMem, RGB (0, 0, 0));
				SelectObject (hDCMem, font_temp);
			}

			//Get size of text
			GetTextExtentPoint32 (hDCMem, text.c_str (), static_cast<int>(text.length ()), &tsize);
			SetBkMode (hDCMem, TRANSPARENT);

			//Print text at coordinates
			TextOut (hDCMem, (cx - tsize.cx) / 2, (cy - tsize.cy) / 2, text.c_str (), static_cast<int>(text.length ()));

			//Bitblt the off-screen DC onto button
			BitBlt (hDC, cRect.left, cRect.top, cRect.right - cRect.left, cRect.bottom - cRect.top, hDCMem, 0, 0, SRCCOPY);

			//Free the DC and the objects
			DeleteObject (bk_brush);
			DeleteObject (bk_pen);
			DeleteObject (txt_pen);
			DeleteObject (font_temp);
			SelectObject (hDCMem, hBMOld);
			DeleteObject (hBMMem);
			DeleteDC (hDCMem);
		}

		bool Button::CheckMove (int x, int y)
		{
			if (state == "Normal")
			{
				if (x >= 0 && x <= cx && y >= 0 && y <= cy)
				{
					state = "Hover";
					Draw (state);

					if (this->child != GetCapture ())
						SetCapture (this->child);

					return true;
				}
				else if (this->child == GetCapture ())
					ReleaseCapture ();
			}
			else if (state == "Hover")
			{
				if (!(x >= 0 && x <= cx && y >= 0 && y <= cy))
				{
					state = "Normal";
					Draw (state);

					if (this->child == GetCapture ())
						ReleaseCapture ();

					return true;
				}
				else if (this->child != GetCapture ())
					SetCapture (this->child);
			}
			else if (state == "Drag")
			{
				if (x >= 0 && x <= cx && y >= 0 && y <= cy)
				{
					state = "Down";
					Draw (state);

					if (this->child != GetCapture ())
						SetCapture (this->child);

					return true;
				}
			}
			else if (state == "Down")
			{
				if (!(x >= 0 && x <= cx && y >= 0 && y <= cy))
				{
					state = "Drag";
					Draw (state);
					return true;
				}
			}

			return false;
		}
		bool Button::CheckUp (int x, int y)
		{
			if (this->child == GetCapture ())
				ReleaseCapture ();

			if (state == "Down")
			{
				if (x >= 0 && x <= cx && y >= 0 && y <= cy)
				{
					state = "Normal";
					Draw (state);
					return true;
				}
			}

			state = "Normal";
			Draw (state);

			return false;
		}
		bool Button::CheckDown (int x, int y)
		{
			if (this->child != GetCapture ())
				SetCapture (this->child);

			if (x >= 0 && x <= cx && y >= 0 && y <= cy)
			{
				state = "Down";
				Draw (state);
				return true;
			}
			else if (this->child == GetCapture ())
				ReleaseCapture ();

			return true;
		}

		LRESULT CALLBACK ButtonProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			Button *WndClass = (Button *)(GetWindowLongPtr (hWnd, GWLP_USERDATA));

			switch (msg)
			{
				case WM_MOUSEMOVE:
					WndClass->CheckMove (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
					break;

				case WM_LBUTTONDOWN:
				{
					WndClass->CheckDown (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
					break;
				}

				case WM_LBUTTONUP:
					if (WndClass->CheckUp (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam)))
						SendMessage (WndClass->parent, Button::msg_beg + Button::WM_BUTTONPRESS, (WPARAM)(WndClass), 0);
					break;

				case WM_ERASEBKGND:
					return (LRESULT)1; // Say we handled it for the draw function

				default:
					return DefWindowProc (hWnd, msg, wParam, lParam);
					break;
			}

			return 0;
		}
	}
}