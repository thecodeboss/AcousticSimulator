#pragma once
#include "Partition.h"

class PMLPartition : public Partition {
	double* p_old;
	double* p;
	double* p_new;
	double* phi_x;
	double* phi_x_new;
	double* phi_y;
	double* phi_y_new;
	double* force;

	int getIdx(int x, int y);

public:
	// PML damping values
	double kxMin{ 0.1 };
	double kxMax{ 0.1 };
	double kyMin{ 0.1 };
	double kyMax{ 0.1 };
	double kzMin{ 0.1 };
	double kzMax{ 0.1 };

	enum PMLType {
		P_RIGHT,
		P_LEFT,
		P_TOP,
		P_BOTTOM,
		P_FRONT,
		P_BACK
	} type;

	PMLPartition(PMLType t, int gx, int gy, int w, int h, int d = 1);
	~PMLPartition();
	virtual void step(double t);
	virtual double* getPressureField();
	virtual double getPressure(int x, int y);
	virtual void setForce(int x, int y, double f);

	friend class Boundary;
};
