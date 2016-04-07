#include "SoundSource.h"

SoundSource::SoundSource(int X, int Y, int Z) : x(X), y(Y), z(Z) {
	static int idGen = 0;
	id = idGen++;
}
