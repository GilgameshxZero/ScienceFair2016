#include "Utility.h"

namespace SFVG
{
	int Utility::StrToInt (std::string str)
	{
		std::stringstream ss;
		int rtrn;

		ss << str;
		ss >> rtrn;

		return rtrn;
	}

	std::string Utility::IntToStr (int x)
	{
		std::stringstream ss;

		ss << x;
		return ss.str ();
	}

	std::string Utility::DoubleToStr (double x)
	{
		std::stringstream ss;

		ss << x;
		return ss.str ();
	}
}