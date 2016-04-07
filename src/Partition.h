#pragma once
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "SoundSource.h"

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

	std::vector<std::shared_ptr<SoundSource>> sources;

	std::thread worker;
	std::condition_variable cv_wait;
	std::mutex cv_m_wait;

	std::condition_variable cv_finish;
	std::mutex cv_m_finish;
	volatile bool finish{ false };
	volatile bool quit{ false };
	volatile bool wait{ true };

	// After all boundaries have been found, these should be filled in with
	// the borders that are not touching anything. This allows for PML boundaries
	// to be easily calculated.
	std::vector<int> topFreeBorders;
	std::vector<int> bottomFreeBorders;
	std::vector<int> leftFreeBorders;
	std::vector<int> rightFreeBorders;

	Partition(int gx, int gy, int w, int h, int d = 1);
	virtual ~Partition();

	void main();

	void computeSourceForcingTerms(double t);

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
	void addSource(std::shared_ptr<SoundSource>& source);
	void startStep(double t);
	void waitForStepFinish();

	static std::vector<std::shared_ptr<Partition>> readFromRecFile(std::string filename);

	friend class Boundary;
	friend class Simulation;
};
