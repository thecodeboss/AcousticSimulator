#pragma once
#include "SoundSource.h"

class GaussianSource : public SoundSource {
public:
	GaussianSource(int X, int Y, int Z);
	virtual double sample(double t) override;
};
