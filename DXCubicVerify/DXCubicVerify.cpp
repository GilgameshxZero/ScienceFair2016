#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

/*
Equations
L+R<->B
L+D<->C
L+X->X
*/

const int MAXTIME = 1e6; //max time-steps in sim.

struct PulseData
{
	int var, //the variable to pulse
		time[2]; //time to start and end pulse
	double add; //the amount to pulse each time-step
};

struct InitData
{
	vector<PulseData> pulse;
	double prob[5], //binding affinities
		init[6]; //initial values of vars: {L, R, B, D, C, X}
};

struct ResultData
{
	int ttime; //total time-steps
	double expect[MAXTIME][6]; //expected value of each variable at each time-step
};

ResultData rdata;

//check if two doubles are close to each other (equal)
bool CloseTo (double a, double b, double EPS = 1e-5)
{
	if (abs (a - b) < EPS)
		return true;
	return false;
}

//output cubic coefs based on initial params
vector<double> VerifyEQ (InitData idata)
{
	vector<double> rtrn;
	double LT, RT, DT, A, B;

	for (int a = 0; a < idata.pulse.size (); a++)
		idata.init[idata.pulse[a].var] += idata.pulse[a].add;

	if (idata.prob[4] != 0 && idata.init[5] != 0)
		LT = idata.init[2] + idata.init[4];
	else
		LT = idata.init[0] + idata.init[2] + idata.init[4];

	DT = idata.init[3] + idata.init[4];
	RT = idata.init[1] + idata.init[2];
	A = idata.prob[0] * idata.prob[3];
	B = idata.prob[1] * idata.prob[2];

	//output the three coefficients of the cubic
	rtrn.push_back ((B - A) * idata.prob[0]);
	rtrn.push_back ((idata.prob[0] * B * DT - idata.prob[0] * B * LT - idata.prob[0] * B * RT - idata.prob[1] * B + idata.prob[0] * A * LT + idata.prob[1] * A + 2 * idata.prob[0] * A * RT));
	rtrn.push_back ((-2 * idata.prob[0] * A * LT * RT + idata.prob[0] * B * LT * RT - idata.prob[0] * A * RT * RT - idata.prob[0] * B * DT * RT - idata.prob[1] * A * RT));
	rtrn.push_back (idata.prob[0] * A * LT * RT * RT);

	return rtrn;
}

void Simulate (InitData &idata)
{
	//delats and orders define equations
	static const int delta[5][6] =
	{{-1,-1,1,0,0,0},
	{1,1,-1,0,0,0},
	{-1,0,0,-1,1,0},
	{1,0,0,1,-1,0},
	{-1,0,0,0,0,0}};
	static const bool order[5][6] =
	{{1,1,0,0,0,0},
	{0,0,1,0,0,0},
	{1,0,0,1,0,0},
	{0,0,0,0,1,0},
	{1,0,0,0,0,1}};
	static double change;
	static bool flag;

	rdata.ttime = -1;
	memcpy (rdata.expect[0], idata.init, sizeof (idata.init));

	for (int a = 1; a < MAXTIME; a++)
	{
		memcpy (rdata.expect[a], rdata.expect[a - 1], sizeof (idata.init));
		for (int b = 0; b < 5; b++)
		{
			//modify next time-step for this var (b)
			change = idata.prob[b];
			for (int c = 0; c < 6; c++)
				if (order[b][c] == true)
					change *= rdata.expect[a - 1][c];
			for (int c = 0; c < 6; c++)
				rdata.expect[a][c] += delta[b][c] * change;
		}

		//simulate pulses
		for (int c = 0; c < idata.pulse.size (); c++)
		{
			if (a >= idata.pulse[c].time[0] && a < idata.pulse[c].time[1])
				//make sure pulse doesn't put var under 0
				rdata.expect[a][idata.pulse[c].var] = max ((double)0, rdata.expect[a][idata.pulse[c].var] + idata.pulse[c].add);
		}

		//check if equilibrium reached
		flag = true;
		for (int b = 0; b < 6; b++)
		{
			if (!CloseTo (rdata.expect[a - 1][b], rdata.expect[a][b]))
			{
				flag = false;
				break;
			}
		}

		//exit if eq.
		if (flag)
		{
			rdata.ttime = a + 1;
			break;
		}
	}
}

int main ()
{
	InitData idata;
	ifstream in ("input.txt");
	int cpulse;

	//input
	for (int a = 0; a < 6; a++)
		in >> idata.init[a];
	for (int a = 0; a < 5; a++)
		in >> idata.prob[a];
	in >> cpulse;
	idata.pulse.resize (cpulse);
	for (int a = 0; a < cpulse; a++)
		in >> idata.pulse[a].var >> idata.pulse[a].time[0] >> idata.pulse[a].time[1] >> idata.pulse[a].add;
	in.close ();

	Simulate (idata);

	//output B at equilibrium and coefs of cubic
	vector<double> verout = VerifyEQ (idata);
	cout << fixed << setprecision (15) << "B at EQ: " << rdata.expect[rdata.ttime - 1][2] << '\n';
	for (int a = 0; a < verout.size (); a++)
		cout << "Verify Data: " << verout[a] << '\n';

	cin.get ();

	return 0;
}