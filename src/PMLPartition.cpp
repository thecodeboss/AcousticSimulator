#include "PMLPartition.h"

PMLPartition::PMLPartition(int gx, int gy, int w, int h, int d)
	: Partition(gx, gy, w, h, d) {
}

PMLPartition::~PMLPartition() {
}

void PMLPartition::step(double t) {
}

double* PMLPartition::getPressureField() {
	return nullptr;
}

double PMLPartition::getPressure(int x, int y) {
	return 0.0;
}

void PMLPartition::setForce(int x, int y, double f) {
}
