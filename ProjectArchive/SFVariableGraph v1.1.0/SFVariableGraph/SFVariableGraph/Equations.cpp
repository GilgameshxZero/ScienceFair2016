#include "Equations.h"

namespace SFVG
{
	double Equations::EvalEq (std::string eq, char var, double val)
	{
		//return 0.1 * val * val;

		//Replace variables.
		std::string valcon;
		std::stringstream ss;
		ss << std::fixed << std::setprecision (15) << val;
		valcon = ss.str ();

		for (int a = 0;a < eq.size ();a++)
		{
			if (eq[a] != var)
				continue;

			eq.erase (a, 1);
			eq.insert (a, valcon);
		}

		//if (val == 

		return EvalParenExpr (eq);

		/*std::string ts;
		std::vector<double> expr; //stack of value of current expression
		std::vector<int> prevop; //previous operator, -1 is none
		char op[] = {'+', '-', '*', '/', '(', ')'};
		int t;
		double td;

		expr.push_back (0);
		prevop.push_back (0);

		for (int a = 0;a < eq.size ();a++)
		{
			if ((eq[a] >= '0' && eq[a] <= '9') || eq[a] == '.')
			{
				ts += eq[a];
			}
			else
			{
				std::stringstream ss;
				ss << ts;
				ss >> td;
				ts = "";

				if (eq[a] == op[0])
				{
					if (prevop.back () == 0)
					{
						expr.back () += td;
						prevop.back () = 0;
					}
					if (prevop.back () == 1)
					{
						expr.back () -= td;
						prevop.back () = 0;
					}
					else
					{
						if (prevop.back () == 2)
						{
							expr.back () *= td;
							prevop.back () = 0;
						}
						if (prevop.back () == 3)
						{
							expr.back () /= td;
							prevop.back () = 0;
						}
					}
				}
				else if (eq[a] == op[1])
				{
					if (prevop.back () == 0)
					{
						expr.back () += td;
						prevop.back () = 1;
					}
					if (prevop.back () == 1)
					{
						expr.back () -= td;
						prevop.back () = 1;
					}
					else
					{
						if (prevop.back () == 2)
						{
							expr.back () *= td;
							prevop.back () = 1;
						}
						if (prevop.back () == 3)
						{
							expr.back () /= td;
							prevop.back () = 1;
						}
					}
				}
				else if (eq[a] == op[2])
				{

					if (prevop.back () == 0)
					{
						expr.back () += td;
						prevop.back () = 1;
					}
					if (prevop.back () == 1)
					{
						expr.back () -= td;
						prevop.back () = 1;
					}
					else
					{
						if (prevop.back () == 2)
						{
							expr.back () *= td;
							prevop.back () = 1;
						}
						if (prevop.back () == 3)
						{
							expr.back () /= td;
							prevop.back () = 1;
						}
					}
				}
				else if (eq[a] == op[3])
				{
				}
				else if (eq[a] == op[4])
				{
				}
				else if (eq[a] == op[5])
				{
				}
				else if (eq[a] == var)
				{
					t = prevop.back ();
					if (t == 0)
						expr.back () += val;
					else if (t == 1)
						expr.back () -= val;
					else if (t == 2)
						expr.back () *= val;
					else if (t == 3)
						expr.back () /= val;
				}
			}
		}*/
	}

	double Equations::EvalParenExpr (std::string expr)
	{
		expr = "0" + expr;

		//Break into parenthesis.
		std::vector<std::string> estack;
		estack.push_back ("");

		for (int a = 0;a < expr.size ();a++)
		{
			if (expr[a] == '(')
			{
				estack.push_back ("");
			}
			else if (expr[a] == ')')
			{
				std::stringstream ss;
				double td = EvalExpr (estack.back ());
				ss << std::fixed << std::setprecision (15) << td;
				estack.pop_back ();
				estack.back () += ss.str ();
			}
			else
			{
				estack.back () += expr[a];
			}
		}

		return EvalExpr (estack[0]);
	}

	double Equations::EvalExpr (std::string expr)
	{
		std::stringstream ss;
		std::string ts;

		ss << std::fixed << std::setprecision (15);

		//Simplify multiplications.
		for (int a = 0;a < expr.size ();a++)
		{
			if (expr[a] == '*')
			{
				//Expand left and right until operator.
				int b, c;
				for (b = a - 1;b >= 0;b--)
				{
					if (expr[b] == '+' || expr[b] == '-' || expr[b] == '*' || expr[b] == '/')
						break;
				}
				for (c = a + 2; c < expr.size (); c++) //Skip next char, might be like 5*-45.
				{
					if (expr[c] == '+' || expr[c] == '-' || expr[c] == '*' || expr[c] == '/')
						break;
				}

				//Parse left and right.
				double d1, d2;
				for (int d = b + 1;d < a;d++)
					ss << expr[d];
				ss >> d1;
				ss.str (std::string ());
				ss.clear ();

				for (int d = a + 1;d < c;d++)
					ss << expr[d];
				ss >> d2;
				ss.str (std::string ());
				ss.clear ();

				ss << d1 * d2;
				ss >> ts;
				ss.str (std::string ());
				ss.clear ();

				//Finish and sub.
				expr.erase (b + 1, c - (b + 1));
				a = b + 1;
				expr.insert (b + 1, ts);
			}
		}

		//Simplify divisions.
		for (int a = 0; a < expr.size (); a++)
		{
			if (expr[a] == '/')
			{
				int b, c;
				for (b = a - 1; b >= 0; b--)
					if (expr[b] == '+' || expr[b] == '-' || expr[b] == '*' || expr[b] == '/')
						break;
				for (c = a + 2; c < expr.size (); c++)
					if (expr[c] == '+' || expr[c] == '-' || expr[c] == '*' || expr[c] == '/')
						break;

				double d1, d2;
				for (int d = b + 1; d < a; d++)
					ss << expr[d];
				ss >> d1;
				ss.str (std::string ());
				ss.clear ();

				for (int d = a + 1; d < c; d++)
					ss << expr[d];
				ss >> d2;
				ss.str (std::string ());
				ss.clear ();

				if (d2 == 0)
					d2 = 1e-9;
				ss << d1 / d2;
				ss >> ts;
				ss.str (std::string ());
				ss.clear ();

				//Finish and sub.
				expr.erase (b + 1, c - (b + 1));
				a = b + 1;
				expr.insert (b + 1, ts);
			}
		}

		//Turn +- to - and -- to +.
		for (int a = 0;a < expr.size () - 1;a++)
		{
			if (expr[a] == '+' && expr[a + 1] == '-')
			{
				expr.erase (a, 1);
			}
			else if (expr[a] == '-' && expr[a + 1] == '-')
			{
				expr.erase (a, 1);
				expr[a] = '+';
			}
		}

		//Simplify additions.
		for (int a = 0; a < expr.size (); a++)
		{
			if (expr[a] == '+')
			{
				int b, c;
				for (b = a - 1; b >= 0; b--)
					if (expr[b] == '+' || expr[b] == '-' || expr[b] == '*' || expr[b] == '/')
						break;
				for (c = a + 2; c < expr.size (); c++)
					if (expr[c] == '+' || expr[c] == '-' || expr[c] == '*' || expr[c] == '/')
						break;

				double d1, d2;
				for (int d = b + 1; d < a; d++)
					ss << expr[d];
				ss >> d1;
				ss.str (std::string ());
				ss.clear ();

				for (int d = a + 1; d < c; d++)
					ss << expr[d];
				ss >> d2;
				ss.str (std::string ());
				ss.clear ();

				ss << d1 + d2;
				ss >> ts;
				ss.str (std::string ());
				ss.clear ();

				//Finish and sub.
				expr.erase (b + 1, c - (b + 1));
				a = b + 1;
				expr.insert (b + 1, ts);
			}
		}

		//Simplify subtractions.
		for (int a = 1; a < expr.size (); a++)
		{
			if (expr[a] == '-')
			{
				int b, c;
				for (b = a - 1; b >= 0; b--)
					if (expr[b] == '+' || expr[b] == '-' || expr[b] == '*' || expr[b] == '/')
						break;
				for (c = a + 2; c < expr.size (); c++)
					if (expr[c] == '+' || expr[c] == '-' || expr[c] == '*' || expr[c] == '/')
						break;

				double d1, d2;
				for (int d = b + 1; d < a; d++)
					ss << expr[d];
				ss >> d1;
				ss.str (std::string ());
				ss.clear ();

				for (int d = a + 1; d < c; d++)
					ss << expr[d];
				ss >> d2;
				ss.str (std::string ());
				ss.clear ();

				ss << d1 - d2;
				ss >> ts;
				ss.str (std::string ());
				ss.clear ();

				//Finish and sub.
				expr.erase (b + 1, c - (b + 1));
				a = b + 1;
				expr.insert (b + 1, ts);
			}
		}

		//Simplify negatives.
		double mul = 1;
		/*for (int a = 0; a < expr.size (); a++)
		{
			if (expr[a] == '-')
			{
				mul *= -1;
			}
			else
				ss << expr[a];
		}*/

		double ans;
		ss << expr;
		ss >> ans;

		return mul * ans;
	}
}