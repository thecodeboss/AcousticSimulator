#include <fftw3.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <memory>
#include "Partition.h"

Partition::Partition(int gx, int gy, int w, int h, int d) : globalX(gx), globalY(gy), width(w), height(h), depth(d) {
	prev_modes = (double*)malloc(width*height*sizeof(double));
	modes = (double*)malloc(width*height*sizeof(double));
	new_modes = (double*)malloc(width*height*sizeof(double));
	force = (double*)malloc(width*height*sizeof(double));
	force_t = (double*)malloc(width*height*sizeof(double));
	pressure = (double*)malloc(width*height*sizeof(double));
	cwt = (double*)malloc(width*height*sizeof(double));
	ang2 = (double*)malloc(width*height*sizeof(double));

	// Create a plan for doing F = iDCT(Ft)
	force_plan = fftw_plan_r2r_2d(height, width, force_t, force,
		FFTW_REDFT01, FFTW_REDFT01, FFTW_MEASURE);

	// Create a plan for doing Ft = DCT(F)
	force_t_plan = fftw_plan_r2r_2d(height, width, force, force_t,
		FFTW_REDFT10, FFTW_REDFT10, FFTW_MEASURE);

	// Create a plan for doing pressure = iDCT(modes)
	modes_to_pressure_plan = fftw_plan_r2r_2d(height, width, modes, pressure,
		FFTW_REDFT01, FFTW_REDFT01, FFTW_MEASURE);

	memset((void*)prev_modes, 0, width*height*sizeof(double));
	memset((void*)modes, 0, width*height*sizeof(double));
	memset((void*)new_modes, 0, width*height*sizeof(double));
	memset((void*)force, 0, width*height*sizeof(double));
	memset((void*)force_t, 0, width*height*sizeof(double));
	memset((void*)pressure, 0, width*height*sizeof(double));

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

Partition::~Partition() {
	fftw_destroy_plan(force_plan);
	fftw_destroy_plan(force_t_plan);
	fftw_destroy_plan(modes_to_pressure_plan);
	free(prev_modes);
	free(modes);
	free(new_modes);
	free(force);
	free(force_t);
	free(pressure);
	free(cwt);
	free(ang2);
}

void Partition::step(double t) {
	// Ft = DCT(F)
	fftw_execute(force_t_plan);

	// Mn[i,j] = 2*M[i,j]*cos(wdt) - Mp[i,j] + 2*Ft[i,j]*(1-cos(wdt))/(w^2)
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int idx = i*width + j;
			new_modes[idx] =
				2.0 * modes[idx] * cwt[idx]
				- prev_modes[idx]
				+ (2.0 * force_t[idx] / ang2[idx]) * (1.0 - cwt[idx]);
		}
	}
	memcpy((void*)prev_modes, (void*)modes, width*height*sizeof(double));
	memcpy((void*)modes, (void*)new_modes, width*height*sizeof(double));
	// P = iDCT2(M)
	fftw_execute(modes_to_pressure_plan);
	// Update forcing term for sources
	force[20 * width + 10] = (t <= 10.0) ? 4.0*sin(2.0*M_PI*t / 10.0) : 0.0;
	// Update forcing term for boundaries @TODO
}

double* Partition::getPressureField() {
	return pressure;
}
