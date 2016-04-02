#pragma once
#include "Partition.h"

class FDTDPartition : public Partition {
	double* p_old;
	double* p;
	double* p_new;
	double* force;

	int getIdx(int x, int y);

public:
	FDTDPartition(int gx, int gy, int w, int h, int d = 1);
	~FDTDPartition();
	virtual void step(double t);
	virtual double* getPressureField();
	virtual double getPressure(int x, int y);
	virtual void setForce(int x, int y, double f);

	friend class Boundary;
};