/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <math.h>
#include <base/math.h>
#include <base/tl/base.h>
#include <base/vmath.h>


void ValidateFCurve(const vec2& p0, vec2& p1, vec2& p2, const vec2& p3)
{
	// validate the bezier curve
	p1.x = clamp(p1.x, p0.x, p3.x);
	p2.x = clamp(p2.x, p0.x, p3.x);
}

double CubicRoot(double x)
{
	if(x == 0.0)
		return 0.0;
	else if(x < 0.0)
		return -exp(log(-x)/3.0);
	else
		return exp(log(x)/3.0);
}

float SolveBezier(float x, float p0, float p1, float p2, float p3)
{
	// check for valid f-curve
	// we only take care of monotonic bezier curves, so there has to be exactly 1 real solution
	tl_assert(p0 <= x && x <= p3);
	tl_assert((p0 <= p1 && p1 <= p3) && (p0 <= p2 && p2 <= p3));

	double a, b, c, t;
	double x3 = -p0 + 3*p1 - 3*p2 + p3;
	double x2 = 3*p0 - 6*p1 + 3*p2;
	double x1 = -3*p0 + 3*p1;
	double x0 = p0 - x;

	if(x3 == 0.0 && x2 == 0.0)
	{
		// linear
		// a*t + b = 0
		a = x1;
		b = x0;

		if(a == 0.0)
			return 0.0f;
		else
			return -b/a;
	}
	else if(x3 == 0.0)
	{
		// quadratic
		// t*t + b*t +c = 0
		b = x1/x2;
		c = x0/x2;

		if(c == 0.0)
			return 0.0f;

		double D = b*b - 4*c;

		t = (-b + sqrt(D))/2;

		if(0.0 <= t && t <= 1.0001f)
			return t;
		else
			return (-b - sqrt(D))/2;
	}
	else
	{
		// cubic
		// t*t*t + a*t*t + b*t*t + c = 0
		a = x2 / x3;
		b = x1 / x3;
		c = x0 / x3;

		// substitute t = y - a/3
		double sub = a/3.0;

		// depressed form x^3 + px + q = 0
		// cardano's method
		double p = b/3 - a*a/9;
		double q = (2*a*a*a/27 - a*b/3 + c)/2;

		double D = q*q + p*p*p;

		if(D > 0.0)
		{
			// only one 'real' solution
			double s = sqrt(D);
			return CubicRoot(s-q) - CubicRoot(s+q) - sub;
		}
		else if(D == 0.0)
		{
			// one single, one double solution or triple solution
			double s = CubicRoot(-q);
			t = 2*s - sub;

			if(0.0 <= t && t <= 1.0001f)
				return t;
			else
				return (-s - sub);

		}
		else
		{
			// Casus irreductibilis ... ,_,
			double phi = acos(-q / sqrt(-(p*p*p))) / 3;
			double s = 2*sqrt(-p);

			t = s*cos(phi) - sub;

			if(0.0 <= t && t <= 1.0001f)
				return t;

			t = -s*cos(phi+pi/3) - sub;

			if(0.0 <= t && t <= 1.0001f)
				return t;
			else
				return -s*cos(phi-pi/3) - sub;
		}
	}
}
