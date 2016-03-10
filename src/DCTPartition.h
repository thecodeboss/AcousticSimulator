#pragma once
#include "Partition.h"
#include "DCTVolume.h"

class DCTPartition : public Partition {
	// Width, height, and depth squared. Minor optimization that saves
	// recalculating these repeatedly
	double w2, h2, d2;

	// Pressure Field
	DCTVolume pressure;

	// Forcing Term
	DCTVolume force;

	// The update rule for modes has M[i+1], M[i], and M[i-1] in it.
	// These keep track of the values for previous modes, etc.
	double* prev_modes{ nullptr };
	double* new_modes{ nullptr };

	// A matrix containing cos(w*t) for every i,j pair.
	double* cwt{ nullptr };

	// A matrix containing w*w for every i,j pair.
	double* ang2{ nullptr };

public:
	DCTPartition(int gx, int gy, int w, int h, int d = 1);
	~DCTPartition();
	virtual void step(double t);
	virtual double* getPressureField();
	virtual double getPressure(int x, int y);
	virtual void setForce(int x, int y, double f);

	friend class Boundary;
};
