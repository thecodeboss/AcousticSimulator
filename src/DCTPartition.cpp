#include <fftw3.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <memory>
#include "DCTPartition.h"

DCTPartition::DCTPartition(int gx, int gy, int w, int h, int d)
	: Partition(gx, gy, w, h, d), pressure(w, h), force(w, h) {
	prev_modes = (double*)malloc(width*height*sizeof(double));
	new_modes = (double*)malloc(width*height*sizeof(double));
	cwt = (double*)malloc(width*height*sizeof(double));
	ang2 = (double*)malloc(width*height*sizeof(double));

	memset((void*)prev_modes, 0, width*height*sizeof(double));
	memset((void*)new_modes, 0, width*height*sizeof(double));

	w2 = width*width;
	h2 = height*height;
	d2 = depth*depth;

	// Precompute some angular frequencies and cosines
	for (int i = 1; i <= height; i++) {
		for (int j = 1; j <= width; j++) {
			int idx = (i - 1)*width + (j - 1);
			double w = speedOfSound*M_PI*sqrt(i*i / h2 + j*j / w2); // @TODO depth
			ang2[idx] = w*w;
			cwt[idx] = cos(w*dt);
		}
	}
}

DCTPartition::~DCTPartition() {
	free(prev_modes);
	free(new_modes);
	free(cwt);
	free(ang2);
}

void DCTPartition::step(double t) {
	// Ft = DCT(F)
	force.executeDCT();

	// Mn[i,j] = 2*M[i,j]*cos(wdt) - Mp[i,j] + 2*Ft[i,j]*(1-cos(wdt))/(w^2)
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int idx = i*width + j;
			new_modes[idx] =
				2.0 * pressure.getMode(j, i) * cwt[idx]
				- prev_modes[idx]
				+ (2.0 * force.getMode(j, i) / ang2[idx]) * (1.0 - cwt[idx]);
		}
	}
	memcpy((void*)prev_modes, (void*)pressure.modes, width*height*sizeof(double));
	memcpy((void*)pressure.modes, (void*)new_modes, width*height*sizeof(double));
	// P = iDCT2(M)
	pressure.executeIDCT();
	// Update forcing term for sources
	if (width == 160) {
		force.setValue(80, 80, exp(-0.5*(t - 8.0)*(t - 8.0)) / sqrt(2 * M_PI));
	}
}

double* DCTPartition::getPressureField() {
	return pressure.values;
}

double DCTPartition::getPressure(int x, int y) {
	return pressure.getValue(x, y);
}

void DCTPartition::setForce(int x, int y, double f) {
	force.setValue(x, y, f);
}
