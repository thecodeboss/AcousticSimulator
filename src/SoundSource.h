#pragma once

class SoundSource {
public:
	int id;
	int x;
	int y;
	int z;
	SoundSource(int X, int Y, int Z);
	virtual double sample(double t) = 0;
};
