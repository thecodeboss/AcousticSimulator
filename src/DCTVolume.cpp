#include "DCTVolume.h"
#include <cmath>
#include <memory>

DCTVolume::DCTVolume(int w, int h) : width(w), height(h) {
	values = (double*)malloc(width*height*sizeof(double));
	modes = (double*)malloc(width*height*sizeof(double));

	// Create a plan for doing modes = DCT(values)
	dct_plan = fftw_plan_r2r_2d(height, width, values, modes,
		FFTW_REDFT10, FFTW_REDFT10, FFTW_MEASURE);

	// Create a plan for doing values = iDCT(modes)
	idct_plan = fftw_plan_r2r_2d(height, width, modes, values,
		FFTW_REDFT01, FFTW_REDFT01, FFTW_MEASURE);

	memset((void*)values, 0, width*height*sizeof(double));
	memset((void*)modes, 0, width*height*sizeof(double));
}

DCTVolume::~DCTVolume() {
	fftw_destroy_plan(dct_plan);
	fftw_destroy_plan(idct_plan);
	free(values);
	free(modes);
}

void DCTVolume::executeDCT() {
	fftw_execute(dct_plan);
	// FFTW3 does not normalize values, so we must perform this
	// step, or values will be wacky.
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			modes[i*width + j] /= 2.0 * sqrt(2.0*width*height);
		}
	}
}

void DCTVolume::executeIDCT() {
	fftw_execute(idct_plan);
	// Normalize
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			values[i*width + j] /= sqrt(2.0*width*height);
		}
	}
}

double DCTVolume::getValue(int x, int y) {
	return values[y*width + x];
}

double DCTVolume::getMode(int x, int y) {
	return modes[y*width + x];
}

void DCTVolume::setValue(int x, int y, double v) {
	values[y*width + x] = v;
}

void DCTVolume::setMode(int x, int y, double m) {
	modes[y*width + x] = m;
}
