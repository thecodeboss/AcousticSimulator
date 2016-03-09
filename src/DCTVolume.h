#pragma once
#include <fftw3.h>

class DCTVolume {
	int width;
	int height;
	int depth; // @TODO
	double* values;
	double* modes;
	fftw_plan dct_plan;
	fftw_plan idct_plan;
public:
	DCTVolume(int w, int h);
	~DCTVolume();
	void executeDCT();
	void executeIDCT();
	double getValue(int x, int y);
	double getMode(int x, int y);
	void setValue(int x, int y, double v);
	void setMode(int x, int y, double m);

	friend class Partition;
};
