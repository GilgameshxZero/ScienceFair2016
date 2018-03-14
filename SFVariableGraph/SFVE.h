#pragma once

#include <algorithm>

namespace SFVG
{
	class SFVE
	{
		public:
			static double r1, rn1, r2, rn2, lt, rt, dt;

			static double Cubic1 (double x);
			static double Cubic2 (double x);
			static double DT1 (double x);
			static double DT2 (double x);
			static double RTLine (double x);
			static double DTLine (double x);
	};
}