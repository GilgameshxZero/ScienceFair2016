#pragma once

#include <ctime>
#include "SliderPanel.h"
#include <Windows.h>

namespace SFVG
{
	namespace IdleHandler
	{
		extern int idlest;

		void Init (SliderPanel *newsp);
		void ObsMouseMove ();

		//called at some fps, tells sliderpanel to update a certain slider - but doesn't tell it to update it to what
		VOID CALLBACK UpdateIdler (HWND hwnd, UINT msg, UINT_PTR idevent, DWORD dtime);
	}
}