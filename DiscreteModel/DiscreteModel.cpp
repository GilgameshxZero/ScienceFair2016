#include <iostream>
#include <utility>
#include <functional>
#include <algorithm>
#include <fstream>
#include <iomanip>
using namespace std;

const int MAXC = 1001, MAXT = 5001;
double ptab[MAXT][MAXC], expect[MAXT][3];
int C, TOTALPART, maxB;

double Power (double a, int b)
{
	if (b == 0)
		return 1;
	else if (b & 1)
	{
		double k = Power (a, b >> 1);
		return k * k * a;
	}
	else
	{
		double k = Power (a, b >> 1);
		return k * k;
	}
}

int main ()
{
	int X, Y, Z, l0, r0, b0, orig;
	int D[3][3];
	double p[2], f[3];

	cout << "x, y, z: ";
	cin >> X >> Y >> Z;
	cout << "p1, p2: ";
	cin >> p[0] >> p[1];

	cout << "L0, R0, B0 (L0, R0, B0 < " << MAXC << "): ";
	cin >> l0 >> r0 >> b0;
	orig = l0 + r0 + b0 * 2;
	C = max (l0, r0) + 2 * b0;
	maxB = min (l0, r0);

	memset (ptab, 0, sizeof (ptab));
	memset (expect, 0, sizeof (expect));
	expect[0][0] = l0;
	expect[0][1] = r0;
	expect[0][2] = b0;
	ptab[0][0] = 1;

	D[0][0] = D[0][1] = D[0][2] = 0;
	D[1][0] = D[1][1] = D[2][2] = -1;
	D[1][2] = D[2][0] = D[2][1] = 1;

	for (int a = 0; a < MAXT - 1; a++)
	{
		for (int b = 0; b <= maxB; b++)
		{
			TOTALPART = 1;
			f[1] = p[0] * Power ((double)(l0 - b) / TOTALPART, X) * Power ((double)(r0 - b) / TOTALPART, Y);
			f[2] = p[1] * Power ((double)(b) / TOTALPART, Z);

			double sum = f[1] + f[2];
			f[1] /= sum;
			f[2] /= sum;

			for (int e = 1; e < 3; e++)
			{
				if (b + D[e][2] < 0 || b + D[e][2] > maxB)
					continue;
				ptab[a + 1][b + D[e][2]] += f[e] * ptab[a][b];
			}
		}
		for (int b = 0; b <= maxB; b++)
		{
			expect[a + 1][0] += (l0 - b) * ptab[a + 1][b];
			expect[a + 1][1] += (r0 - b) * ptab[a + 1][b];
			expect[a + 1][2] += (b)* ptab[a + 1][b];
		}
	}

	ofstream out ("out.csv");
	out << fixed << setprecision (7);

	out << "t";
	for (int a = 0; a < MAXT; a++)
		out << "," << a;
	out << "\n";

	for (int b = 0; b <= maxB; b++)
	{
		out << "{" << l0 - b << "-" << r0 - b << "-" << b << "},";
		for (int a = 0; a < MAXT; a++)
		{
			out << "\"" << ptab[a][b] << "\",";
		}
		out << "\n";
	}

	for (int e = 0; e < 3; e++)
	{
		out << "exp[" << e << "]";
		for (int a = 0; a < MAXT; a++)
		{
			out << "," << expect[a][e];
		}
		out << "\n";
	}

	out.close ();
	return 0;
}