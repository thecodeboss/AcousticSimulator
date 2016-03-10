#pragma once
#include "Partition.h"

class PMLPartition : public Partition {
public:
	PMLPartition(int gx, int gy, int w, int h, int d = 1);
	~PMLPartition();
	virtual void step(double t);
	virtual double* getPressureField();
	virtual double getPressure(int x, int y);
	virtual void setForce(int x, int y, double f);

	friend class Boundary;
};
