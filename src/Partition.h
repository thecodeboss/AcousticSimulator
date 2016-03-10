#pragma once

class Partition {
protected:
	// Time step size
	double dt{ 0.5 };

	// Speed of sound
	double speedOfSound{ 1.0 };

	Partition(int gx, int gy, int w, int h, int d = 1);
	virtual ~Partition();

public:
	// Parameters defining the position in global space, along with the
	// size of this partition
	int globalX, globalY, globalZ;
	int width, height, depth;

	virtual void step(double t) = 0;
	virtual double* getPressureField() = 0;
	virtual double getPressure(int x, int y) = 0;
	virtual void setForce(int x, int y, double f) = 0;

	friend class Boundary;
};
