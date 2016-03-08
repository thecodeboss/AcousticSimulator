#pragma once
#include "DCTVolume.h"

class Partition {
	// Width, height, and depth squared. Minor optimization that saves
	// recalculating these repeatedly
	double w2, h2, d2;

	// Time step size
	double dt{ 0.5 };
	double speedOfSound{ 1.0 };

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
	// Parameters defining the position in global space, along with the
	// size of this partition
	int globalX, globalY, globalZ;
	int width, height, depth;

	Partition(int gx, int gy, int w, int h, int d = 1);
	~Partition();
	void step(double t);
	double* getPressureField();

	friend class Boundary;
};
