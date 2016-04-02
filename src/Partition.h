#pragma once
#include <vector>

class Boundary;

class Partition {
protected:
	// Time step size
	double dt{ 0.5 };

	// Speed of sound
	double speedOfSound{ 1.0 };

	// When calculating interface terms, a region using an FDTD/PML scheme should not
	// include its own terms, as this will result in "double forcing". Thus we track a
	// boolean for this.
	bool includeSelfTerms{ true };

	// After all boundaries have been found, these should be filled in with
	// the borders that are not touching anything. This allows for PML boundaries
	// to be easily calculated.
	std::vector<int> topFreeBorders;
	std::vector<int> bottomFreeBorders;
	std::vector<int> leftFreeBorders;
	std::vector<int> rightFreeBorders;

	Partition(int gx, int gy, int w, int h, int d = 1);
	virtual ~Partition();

public:
	// Parameters defining the position in global space, along with the
	// size of this partition
	int globalX, globalY, globalZ;
	int width, height, depth;

	// Whether or not this partition type gets rendered
	bool shouldRender{ true };

	// Absoption coefficient
	// 1.0 = perfect absorption (ie. outside)
	// 0.6 = 60% absorbed, 40% reflected
	// 0.0 = perfect reflection
	double absorption{ 0.8 };

	virtual void step(double t) = 0;
	virtual double* getPressureField() = 0;
	virtual double getPressure(int x, int y) = 0;
	virtual void setForce(int x, int y, double f) = 0;

	void addBoundary(Boundary* boundary);

	friend class Boundary;
	friend class Simulation;
};
