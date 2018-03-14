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

const int MAXTIME = 1e6, //max time-steps for simulation
	MAXCOL = 250; //max columns in output file

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
	ofstream out ("output.csv");
	int cpulse;
	char letter_cor[6] = {'L','R','B','D','C','X'}; //the letters each variable corresponds to

	//input data
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

	//output rdata
	out << "time";
	if (rdata.ttime <= MAXCOL)
		for (int b = 0; b < rdata.ttime; b++)
			out << ',' << b;
	else
		for (int b = 0, c = 0; c < MAXCOL; c++, b = c * rdata.ttime / MAXCOL)
			out << ',' << b;
	out << '\n';

	for (int a = 0; a < 6; a++)
	{
		out << letter_cor[a];
		if (rdata.ttime <= MAXCOL)
			for (int b = 0; b < rdata.ttime; b++)
				out << ',' << rdata.expect[b][a];
		else
			for (int b = 0, c = 0; c < MAXCOL; c++, b = c * rdata.ttime / MAXCOL)
				out << ',' << rdata.expect[b][a];
		out << '\n';
	}

	out.close ();

	return 0;
}