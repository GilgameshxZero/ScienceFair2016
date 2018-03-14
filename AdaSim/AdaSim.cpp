#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <map>
#include <functional>

using namespace std;
typedef long long ll;

/*
Equations
L+R<->B
D1+R<->C1
L+X->X
D2+R<->C2
*/

struct PulseData
{
	ll var, //the variable to pulse
		time[2]; //time to start and end pulse
	double add; //the amount to pulse each time-step
};

struct InitData
{
	vector<PulseData> pulse;
	vector<double> prob, //binding affinities (r1, r-1, r2, r-2, r3, r4, r-4)
		init; //initial values of vars: {L, R, B, D1, C1, X, D2, C2}
};

struct ResultData
{
	vector<double> truetime; //time of a reaction finish
	vector< vector<double> > val; //values of parameters at some time (truetime)
};

struct InputData
{
	vector<double> p, r;
	double l0, r0, d0, t;
	ll n, z;
};

struct VirusType
{
	ll wt, hr;

	VirusType ()
	{
		wt = hr = 0;
	}

	VirusType (ll x, ll y)
	{
		wt = x;
		hr = y;
	}
};

struct State
{
	vector<VirusType> cnt;
};

//check if two doubles are close to each other (equal)
bool CloseTo (double a, double b, double EPS = 1e-5)
{
	if (abs (a - b) < EPS)
		return true;
	return false;
}

//2^56 size random ll
ll LRand ()
{
	ll ret = 0;
	for (int a = 0;a < 7;a++)
		ret = (ret << 8) + (rand () % (1 << 8));
	return ret;
}

//random probability
double ProbRand ()
{
	return (double)LRand () / (1LL << 56);
}

//returns probability of reaction 'reac' happening
double ReactionProb (vector<double> &rr, vector<double> &val, int reac)
{
	//reac is 0-6
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
	else if (reac == 5)
		return rr[5] * val[6] * val[1];
	else if (reac == 6)
		return rr[6] * val[7];
}

void SimGillespie (InitData &idata, ResultData &rdata, double endtime)
{
	static const double delta[7][8] =
	{{-1,-1,1,0,0,0,0,0},
	{1,1,-1,0,0,0,0,0},
	{0,-1,0,-1,1,0,0,0},
	{0,1,0,1,-1,0,0,0},
	{-1,0,0,0,0,0,0,0},
	{0,-1,0,0,0,0,-1,1},
	{0,1,0,0,0,0,1,-1}};
	static double rtot;

	rdata.truetime.push_back (0);
	rdata.val.push_back (idata.init);

	while (rdata.truetime.back () < endtime)
	{
		static double rnum, tdoub, time;
		static int reaction;

		rtot = 0;
		for (int a = 0; a < 7; a++)
			rtot += ReactionProb (idata.prob, rdata.val.back (), a);

		//weighted choose reaction
		rnum = (double)rand () / RAND_MAX;
		tdoub = 0;
		for (int a = 0; a < 7; a++)
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
		for (int a = 0; a < 8; a++)
			rdata.val.back ()[a] += delta[reaction][a];
	}
}

int main ()
{
	InputData idata;
	State cur, next;
	ifstream in ("in.txt");
	ofstream out ("out.csv");
	ll genswitch, maxgen, hreffec; //switch environments at genswitch, such that multiple copies don't offer benefits

	srand (time (NULL));

	/*for (int a = 0;a < 50;a++)
		cout << ProbRand () << ' ';
	cin.get ();*/

	idata.p.resize (4);
	idata.r.resize (4);
	for (int a = 0;a < 4;a++)
		in >> idata.p[a];
	for (int a = 0; a < 4; a++)
		in >> idata.r[a];
	in >> idata.l0 >> idata.r0 >> idata.d0 >> idata.n >> idata.z >> idata.t;
	in >> genswitch >> maxgen >> hreffec;
	in.close ();

	cur.cnt.insert (cur.cnt.begin (), idata.n, VirusType (1, 0));
	out << fixed << setprecision (10);
	out << "Generation, Wild-Type per Virus, H47R per Virus\n";
	for (ll gen = 0;gen < maxgen;gen++)
	{
		//user notify
		cout << "\rGen: " << gen + 1;

		//output average wt and hr in this generation
		ll wttot, hrtot, virtot;
		wttot = hrtot = virtot = 0;
		for (int a = 0;a < cur.cnt.size ();a++)
		{
			wttot += cur.cnt[a].wt;
			hrtot += cur.cnt[a].hr;
			virtot++;
		}

		out << gen << "," << (double)wttot / virtot << "," << (double)hrtot / virtot << "\n";

		//produce z offspring for every virus in state, which vary by mutation parameters p
		next.cnt.resize (0);
		for (int a = 0; a < cur.cnt.size (); a++)
		{
			bool fatal;
			for (int b = 0;b < idata.z;b++)
			{
				next.cnt.push_back (cur.cnt[a]);
				fatal = false;

				//apply p to every gene in parent
				for (int c = 0;c < cur.cnt[a].wt;c++)
				{
					if (ProbRand () < idata.p[0]) //mutate
					{
						next.cnt.back ().wt--;
						next.cnt.back ().hr++;

						if (ProbRand () < idata.p[3]) //del
						{
							next.cnt.back ().hr--;
						}
					}
					else if (ProbRand () < idata.p[3]) //del
					{
						next.cnt.back ().wt--;
					}

					if (ProbRand () < idata.p[1]) //fatal
					{
						fatal = true;
					}
					if (ProbRand () < idata.p[2]) //dupe
					{
						next.cnt.back ().wt++;
					}
				}
				for (int c = 0; c < cur.cnt[a].hr; c++)
				{
					if (ProbRand () < idata.p[1]) //fatal
					{
						fatal = true;
					}
					if (ProbRand () < idata.p[2]) //dupe
					{
						next.cnt.back ().hr++;
					}
					if (ProbRand () < idata.p[3]) //del
					{
						next.cnt.back ().hr--;
					}
				}

				if (fatal)
					next.cnt.pop_back ();
				else if (next.cnt.back ().hr == 0 && next.cnt.back ().wt == 0) //(0, 0) is already dead
					next.cnt.pop_back ();
			}
		}

		//calculate B at time t for each of the offspring//use gillespie
		vector<double> rbt (next.cnt.size ());
		InitData initdata;
		ResultData rdata;

		initdata.prob = idata.r;
		initdata.prob.push_back (0); //not used
		initdata.prob.push_back (idata.r[2] * hreffec);
		initdata.prob.push_back (idata.r[3]);

		initdata.init.push_back (idata.l0);
		initdata.init.push_back (idata.r0);
		initdata.init.push_back (0);
		initdata.init.push_back (0); //d0, will be adjusted later
		initdata.init.push_back (0);
		initdata.init.push_back (0); //x0, not used
		initdata.init.push_back (0); //d0, for h47r, will be adjusted later
		initdata.init.push_back (0);

		for (int a = 0;a < next.cnt.size ();a++)
		{
			if (gen < genswitch)
			{
				initdata.init[3] = idata.d0 * next.cnt[a].wt;
				initdata.init[6] = idata.d0 * next.cnt[a].hr;
			}
			else
			{
				initdata.init[3] = initdata.init[6] = 0;
				if (next.cnt[a].hr >= 1)
					initdata.init[6] = idata.d0;
				else if (next.cnt[a].wt >= 1)
					initdata.init[3] = idata.d0;
			}

			SimGillespie (initdata, rdata, idata.t);

			rbt[a] = idata.r0 - rdata.val.back ()[2];
			
			rdata.truetime.clear ();
			rdata.val.clear ();
		}

		//select n offspring to live to next gen, weighted by r0 - B_t. some can be selected multiple times
		double bttot = 0, curprob = 0;
		vector<double> probrand;

		cur.cnt.clear ();

		for (int a = 0;a < rbt.size ();a++)
			bttot += rbt[a];

		for (int a = 0;a < idata.n;a++)
			probrand.push_back (ProbRand ());
		sort (probrand.begin (), probrand.end ());

		for (int a = 0, b = 0;a < rbt.size ();a++)
		{
			curprob += rbt[a] / bttot;
			while (b < probrand.size () && probrand[b] <= curprob)
			{
				b++;
				cur.cnt.push_back (next.cnt[a]);
			}
		}
		/*vector< pair<double, int> > ss;
		for (int a = 0;a < rbt.size ();a++)
			ss.push_back (make_pair (rbt[a], a));
		sort (ss.begin (), ss.end (), greater< pair<double, int> > ());
		cur.cnt.clear ();
		for (int a = 0;a < idata.n && a < ss.size ();a++)
			cur.cnt.push_back (next.cnt[ss[a].second]);*/
	}

	return 0;
}