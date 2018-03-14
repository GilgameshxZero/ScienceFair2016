#pragma once

#include <string>
#include <sstream>

namespace SFVG
{
	class Utility
	{
		public:
			static int StrToInt (std::string str);
			static std::string IntToStr (int x);
			static std::string DoubleToStr (double x);
	};
}