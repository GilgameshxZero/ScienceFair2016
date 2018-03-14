#include "IdleHandler.h"

namespace SFVG
{
	namespace IdleHandler
	{
		int idlest, tlastchange, curvar, lasttime;
		const int IDLETHRESH = 5000,
			CHANGETHRESH = 10000,
			TOTALVAR = 7, 
			FPS = 5, 
			SPF = static_cast<int> (1000.0 / FPS);
		const bool IDLEON = false;
		SliderPanel *sp;

		void Init (SliderPanel *newsp)
		{
			sp = newsp;
			curvar = 0;
			tlastchange = CHANGETHRESH + 1;
			idlest = lasttime = GetTickCount ();
			srand (idlest);
			SetTimer (NULL, 1, USER_TIMER_MINIMUM, UpdateIdler);
		}

		void ObsMouseMove ()
		{
			idlest = GetTickCount ();
		}

		VOID CALLBACK UpdateIdler (HWND hwnd, UINT msg, UINT_PTR idevent, DWORD dtime)
		{
			if (!IDLEON)
				return;

			int idletime = dtime - idlest;

			if (idletime >= IDLETHRESH) //we are in idle mode, send messages
			{
				//choose a variable to change
				if (tlastchange >= CHANGETHRESH)
				{
					tlastchange -= CHANGETHRESH;
					curvar = rand () % TOTALVAR;
				}

				//send message to sliderpanel
				sp->IdleChVar (curvar);
			}

			//set new timer
			tlastchange += dtime - lasttime;
			lasttime = dtime;
			SetTimer (NULL, 1, SPF, UpdateIdler);
		}
	}
}