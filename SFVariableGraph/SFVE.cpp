#include "SFVE.h"

namespace SFVG
{
	double SFVE::r1;
	double SFVE::rn1;
	double SFVE::r2;
	double SFVE::rn2;
	double SFVE::lt;
	double SFVE::rt;
	double SFVE::dt;

	double SFVE::DT1 (double val)
	{
		if (SFVE::r2 * (SFVE::dt - val) == 0)
			return SFVE::rt - val - (SFVE::rn2 * val) / 1e-9;
		return SFVE::rt - val - (SFVE::rn2 * val) / (SFVE::r2 * (SFVE::dt - val));
	}

	double SFVE::DT2 (double val)
	{
		if (SFVE::SFVE::r1 * (SFVE::SFVE::lt - val) == 0)
			return SFVE::SFVE::rt - val - (SFVE::SFVE::rn1 * val) / 1e-9;
		return SFVE::SFVE::rt - val - (SFVE::SFVE::rn1 * val) / (SFVE::SFVE::r1 * (SFVE::SFVE::lt - val));
	}

	double SFVE::Cubic1 (double val)
	{
		return (-SFVE::r2*SFVE::r2*SFVE::rn1 + SFVE::r2*SFVE::r1*SFVE::rn2)*val*val*val + (2 * SFVE::r2*SFVE::r2*SFVE::rn1*SFVE::dt + SFVE::r2*SFVE::r2*SFVE::rn1*SFVE::rt - SFVE::r2*SFVE::r1*SFVE::SFVE::rn2*SFVE::rt - SFVE::r2*SFVE::r1*SFVE::SFVE::rn2*SFVE::dt + SFVE::r2*SFVE::r1*SFVE::SFVE::rn2*SFVE::lt + SFVE::rn2*SFVE::r2*SFVE::rn1 - SFVE::rn2*SFVE::r1*SFVE::SFVE::rn2)*val*val + (-2 * SFVE::r2*SFVE::r2*SFVE::rn1*SFVE::rt*SFVE::dt - SFVE::r2*SFVE::r2*SFVE::rn1*SFVE::dt*SFVE::dt + SFVE::r2*SFVE::r1*SFVE::SFVE::rn2*SFVE::rt*SFVE::dt - SFVE::r2*SFVE::r1*SFVE::SFVE::rn2*SFVE::dt*SFVE::lt - SFVE::rn2*SFVE::r2*SFVE::rn1*SFVE::dt)*val + SFVE::r2*SFVE::r2*SFVE::rn1*SFVE::rt*SFVE::dt*SFVE::dt;
	}

	double SFVE::Cubic2 (double val)
	{
		return (-SFVE::r1*SFVE::r1*SFVE::rn2 + SFVE::r1*SFVE::r2*SFVE::rn1)*val*val*val + (2 * SFVE::r1*SFVE::r1*SFVE::rn2*SFVE::lt + SFVE::r1*SFVE::r1*SFVE::rn2*SFVE::rt - SFVE::r1*SFVE::r2*SFVE::SFVE::rn1*SFVE::rt - SFVE::r1*SFVE::r2*SFVE::SFVE::rn1*SFVE::lt + SFVE::r1*SFVE::r2*SFVE::SFVE::rn1*SFVE::dt + SFVE::rn1*SFVE::r1*SFVE::rn2 - SFVE::rn1*SFVE::r2*SFVE::SFVE::rn1)*val*val + (-2 * SFVE::r1*SFVE::r1*SFVE::rn2*SFVE::rt*SFVE::lt - SFVE::r1*SFVE::r1*SFVE::rn2*SFVE::lt*SFVE::lt + SFVE::r1*SFVE::r2*SFVE::SFVE::rn1*SFVE::rt*SFVE::lt - SFVE::r1*SFVE::r2*SFVE::SFVE::rn1*SFVE::lt*SFVE::dt - SFVE::rn1*SFVE::r1*SFVE::rn2*SFVE::lt)*val + SFVE::r1*SFVE::r1*SFVE::rn2*SFVE::rt*SFVE::lt*SFVE::lt;
	}

	double SFVE::RTLine (double val)
	{
		return std::min (SFVE::rt, SFVE::lt);
	}

	double SFVE::DTLine (double val)
	{
		return std::min (SFVE::dt, SFVE::rt);
	}
}