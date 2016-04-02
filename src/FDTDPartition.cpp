#include "FDTDPartition.h"
#include <memory>

FDTDPartition::FDTDPartition(int gx, int gy, int w, int h, int d)
	: Partition(gx, gy, w, h, d) {
	int size = width*height + 1; // +1 so we can have a zero at the end
	p_old = (double*)malloc(size*sizeof(double));
	p = (double*)malloc(size*sizeof(double));
	p_new = (double*)malloc(size*sizeof(double));

	force = (double*)malloc(size*sizeof(double));

	// Zero out everything
	memset((void*)p_old, 0, size*sizeof(double));
	memset((void*)p, 0, size*sizeof(double));
	memset((void*)p_new, 0, size*sizeof(double));
	memset((void*)force, 0, size*sizeof(double));

	// Special interface handling
	includeSelfTerms = false;
}

FDTDPartition::~FDTDPartition() {
	free(p_old);
	free(p);
	free(p_new);
	free(force);
}

int FDTDPartition::getIdx(int x, int y) {
	if (x < 0 || x >= width) return width*height;
	if (y < 0 || y >= height) return width*height;
	return y*width + x;
}

void FDTDPartition::step(double t) {
	double dx = 1.0;
	double dy = 1.0;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			double coefs[] = { 2.0, -27.0, 270.0, -490.0, 270.0, -27.0, 2.0 };
			double KPx = 0.0;
			double KPy = 0.0;
			for (int k = 0; k < 7; k++) {
				KPx += coefs[k] * p[getIdx(i + k - 3, j)];
				KPy += coefs[k] * p[getIdx(i, j + k - 3)];
			}
			KPx /= 180.0;
			KPy /= 180.0;

			double c = speedOfSound;
			p_new[getIdx(i, j)] = 2 * p[getIdx(i, j)] - p_old[getIdx(i, j)] + c*c*dt*dt*(KPx + KPy + force[getIdx(i, j)]);
		}
	}

	double* temp = p_old;
	p_old = p;
	p = p_new;
	p_new = temp;

	memset((void*)force, 0, width*height*sizeof(double));
	//setForce(40, 80, (t <= 5.0) ? 15.0*sin(3.14159265359*t / 5.0) : 0.0);
}

double* FDTDPartition::getPressureField() {
	return p;
}

double FDTDPartition::getPressure(int x, int y) {
	return p[getIdx(x, y)];
}

void FDTDPartition::setForce(int x, int y, double f) {
	force[getIdx(x, y)] = f;
}
