#ifndef RAIN_BUTTON
#define RAIN_BUTTON

#include <Windows.h>
#include <string>
#include <WindowsX.h>
#include "Utility.h"

namespace Rain
{
	namespace Button
	{
		class Button
		{
			public:
				Button (void);
				void Initialize (HWND parent, std::string text, int cx = 0, int cy = 0, int fontsize = 12, std::string font = "Consolas"); //Creates the window for the button and draws everything on the window, but hides the window. Calls draw

				void Show (int x, int y); //Shows the window at the coordinates
				void Hide (); //Hides the window
				HWND GetHWND ();
				
				void SetText (std::string new_text);

				static void SetMessageStart (UINT start);

				static int id;
				static UINT msg_beg, //Begin code of all messages sent by this class.
					WM_BUTTONPRESS; //Shift with msg_beg to get code to send for BUTTONPRESS.

			private:
				void Draw (std::string stage); //Draws the button on the window

											   //Functions for the message handler, a friend
				bool Button::CheckMove (int x, int y);
				bool Button::CheckUp (int x, int y);
				bool Button::CheckDown (int x, int y);

				int cx, cy; //Dimensions
				int fontsize; //Size of the font
				std::string text; //Text on the button
				std::string font; //Font of the text
				std::string state; //State of the button, namely "Normal", "Hover", "Drag", or "Down"
				HWND parent, child; //Parent window of the button, window of the button

			//Friend the message handler so that it can access the Check functions
			friend LRESULT CALLBACK ButtonProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		};
	}
}

#endif