#pragma once

#include "Utility.h"
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>

namespace SFVG
{
	class Equations
	{
		public:
			//Basic operations/no parenthesis.
			static double EvalEq (std::string eq, char var, double val);

			static double EvalParenExpr (std::string expr);
			static double EvalExpr (std::string expr);
	};
}