#ifndef Processing_H
#define Processing_H

#include <cstddef>

class Hysteresis
{
private:
	float thr_lo;
	float thr_hi;

	mutable bool state;

public:
	Hysteresis(float tl, float th, bool s = false) : thr_lo(tl), thr_hi(th), state(s)
	{
	}
	~Hysteresis()
	{
	}

	bool evaluate(float value) const
	{
		bool newState = state;

		if (!state && value >= thr_hi)
			state = true;

		else if (state && value <= thr_lo)
			state = false;

		return state;
	}
};

class BoolLowpass
{
private:
	size_t req_cont;

	mutable size_t curr_cont = 0;
	mutable bool state;

public:
	BoolLowpass(size_t rs = 1, bool s = false) : req_cont(rs), state(s)
	{
	}
	~BoolLowpass()
	{
	}

	bool evaluate(bool value) const
	{
		if (value == state)
			curr_cont = 0;
		else if (++curr_cont >= req_cont)
			state = value;

		return state;
	}
};

class PosEdge
{
private:
	mutable bool prev;

public:
	PosEdge(bool p = false) : prev(p)
	{
	}
	~PosEdge()
	{
	}

	bool evaluate(bool value) const
	{
		bool ret = !prev && value;
		prev = value;

		return ret;
	}
};

class NegEdge
{
private:
	mutable bool prev;

public:
	NegEdge(bool p = false) : prev(p)
	{
	}
	~NegEdge()
	{
	}

	bool evaluate(bool value) const
	{
		bool ret = prev && !value;
		prev = value;

		return ret;
	}
};

#endif