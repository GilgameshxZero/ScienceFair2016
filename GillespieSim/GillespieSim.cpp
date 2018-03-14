#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <cstdlib>

using namespace std;

/*
Equations
L+R<->B
L+D<->C
L+X->X
*/

const double MAXTIME = 25, //end time for simulation
OUTROW = 5000; //rows of data in output for each simulation
const bool sptime = true; //turn off intelligent cutting?

struct PulseData
{
	int var, //the variable to pulse
		time[2]; //time to start and end pulse
	double add; //the amount to pulse each time-step
};

struct InitData
{
	vector<PulseData> pulse;
	vector<double> prob, //binding affinities (r1, r-1, r2, r-2, r3)
		init; //initial values of vars: {L, R, B, D, C, X}
};

struct ResultData
{
	vector<double> truetime; //time of a reaction finish
	vector< vector<double> > val; //values of parameters at some time (truetime)
};

struct OutputData
{
	double truetime;
	vector<double> val[2]; //values of both simulations here
};

ResultData gilldata, dmpdata;
vector<OutputData> outdata;

//check if two doubles are close to each other (equal)
bool CloseTo (double a, double b, double EPS = 1e-5)
{
	if (abs (a - b) < EPS)
		return true;
	return false;
}

//returns probability of reaction 'reac' happening
double ReactionProb (vector<double> &rr, vector<double> &val, int reac)
{
	//rr is a 5-element array, reac is 0-4
	if (reac == 0)
		return rr[0] * val[0] * val[1];
	else if (reac == 1)
		return rr[1] * val[2];
	else if (reac == 2)
		return rr[2] * val[3] * val[1];
	else if (reac == 3)
		return rr[3] * val[4];
	else if (reac == 4)
		return rr[4] * val[0] * val[5];
	else
		return 0;
}

//using midpoint method, calculate change of one variable
double CalcDeriv (vector<double> &rr, vector<double> &val, int var)
{
	//val is a 6-element array, rr is a 5-element array
	if (var == 2)
		return rr[0] * val[0] * val[1] - rr[1] * val[2];
	else if (var == 1)
		return -rr[0] * val[0] * val[1] + rr[1] * val[2] - rr[2] * val[3] * val[1] + rr[3] * val[4];
	else if (var == 4)
		return rr[2] * val[3] * val[1] - rr[3] * val[4];
	else if (var == 3)
		return -rr[2] * val[3] * val[1] + rr[3] * val[4];
	else if (var == 0)
		return -rr[0] * val[0] * val[1] + rr[1] * val[2];
	else //if (var == 5)
		return 0;
}

void SimGillespie (InitData &idata, ResultData &rdata, double endtime)
{
	static const double delta[5][6] =
	{{-1,-1,1,0,0,0},
	{1,1,-1,0,0,0},
	{0,-1,0,-1,1,0},
	{0,1,0,1,-1,0},
	{-1,0,0,0,0,0}};
	static double rtot;

	rdata.truetime.push_back (0);
	rdata.val.push_back (idata.init);

	while (rdata.truetime.back () < endtime)
	{
		static double rnum, tdoub, time;
		static int reaction;

		rtot = 0;
		for (int a = 0; a < 5; a++)
			rtot += ReactionProb (idata.prob, rdata.val.back (), a);

		//weighted choose reaction
		rnum = (double)rand () / RAND_MAX;
		tdoub = 0;
		for (int a = 0; a < 5; a++)
		{
			tdoub += ReactionProb (idata.prob, rdata.val.back (), a) / rtot;
			if (rnum <= tdoub)
			{
				reaction = a;
				break;
			}
		}

		//weighted choose time
		rnum = (double)rand () / RAND_MAX;
		time = -log (rnum) / rtot;

		//apply reaction to next time step
		rdata.truetime.push_back (rdata.truetime.back () + time);
		rdata.val.push_back (rdata.val.back ());
		for (int a = 0; a < 6; a++)
			rdata.val.back ()[a] += delta[reaction][a];
	}
}

double SimDMP (InitData &idata, ResultData &rdata, double endtime)
{
	static const double stepsize = 1e-5, eps = stepsize;
	static double maxinit;
	vector<double> mpexpect (6);

	rdata.truetime.push_back (0);
	rdata.val.push_back (idata.init);

	//find max init var
	maxinit = 0;
	for (int a = 0; a < 6; a++)
		maxinit = max (maxinit, rdata.val[0][a]);

	for (int a = 1; rdata.truetime.back () < endtime; a++)
	{
		rdata.val.push_back (rdata.val.back ());
		rdata.truetime.push_back (rdata.truetime.back () + stepsize);

		for (int b = 0; b < 6; b++)
			mpexpect[b] = rdata.val[a - 1][b] + stepsize / 2 * CalcDeriv (idata.prob, rdata.val[a - 1], b);

		for (int b = 0; b < 6; b++)
			rdata.val[a][b] += stepsize * CalcDeriv (idata.prob, mpexpect, b);

		//stop when reaching eq.
		if (!sptime)
		{
			static bool exit;
			exit = true;
			for (int b = 0; b < 6; b++)
			{
				if (!CloseTo (stepsize * CalcDeriv (idata.prob, mpexpect, b), 0, eps * maxinit))
				{
					exit = false;
					break;
				}
			}
		}
	}

	return rdata.truetime.back ();
}

bool OutDataComp (OutputData &x, OutputData &y)
{
	return x.truetime < y.truetime;
}

int main ()
{
	InitData idata;
	ifstream in ("in.txt");
	ofstream out ("out.csv");
	int cpulse, outuntil;
	char letter_cor[6] = {'L','R','B','D','C','X'}; //the letters each variable corresponds to
	double eqtime;
	pair< double, vector<double> > lastval[2], //last data point for both simulations
		nextval;

	srand (time (NULL));

	//input data
	idata.init.resize (6);
	idata.prob.resize (5);
	for (int a = 0; a < 6; a++)
		in >> idata.init[a];
	for (int a = 0; a < 5; a++)
		in >> idata.prob[a];
	in >> cpulse;
	idata.pulse.resize (cpulse);
	for (int a = 0; a < cpulse; a++)
		in >> idata.pulse[a].var >> idata.pulse[a].time[0] >> idata.pulse[a].time[1] >> idata.pulse[a].add;
	in.close ();

	SimGillespie (idata, gilldata, MAXTIME);

	//insert OUTROW of each data into outdata, then sort outdata by truetime
	for (int a = 1; a < /*OUTROW*/gilldata.val.size (); a++)
	{
		static int b;
		b = (double)a * gilldata.val.size () / /*OUTROW*/gilldata.val.size ();
		if (b == (int)((double)(a - 1) * (int)gilldata.val.size () / /*OUTROW*/gilldata.val.size ()))
			continue;
		outdata.push_back (OutputData ());
		outdata.back ().truetime = gilldata.truetime[b];
		outdata.back ().val[1] = gilldata.val[b];
	}

	//output rdata
	out << fixed << setprecision (10);
	out << "truetime,L,R,B,D,C\n";
	for (int b = 0; b < outdata.size (); b++)
	{
		out << outdata[b].truetime;
		for (int a = 0;a < 5;a++)
		{
			out << "," << outdata[b].val[1][a];
		}
		out << "\n";
	}
	out.close ();

	return 0;
}