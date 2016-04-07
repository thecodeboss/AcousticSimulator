#include "GaussianSource.h"
#include <cmath>
#define M_PI (3.14159265358979323846)

GaussianSource::GaussianSource(int X, int Y, int Z) : SoundSource(X, Y, Z) {

}


double GaussianSource::sample(double t) {
	return exp(-0.5*(t - 8.0)*(t - 8.0)) / sqrt(2 * M_PI);
}
