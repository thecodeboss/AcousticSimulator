#pragma once
#include <memory>

class Partition;

class Boundary {
	std::shared_ptr<Partition> a;
	std::shared_ptr<Partition> b;
	int xStart;
	int xEnd;
	int yStart;
	int yEnd;
	int zStart;
	int zEnd;

	enum BoundaryType {
		X_BOUNDARY,
		Y_BOUNDARY,
		Z_BOUNDARY
	} type;

public:

	Boundary(BoundaryType bType, std::shared_ptr<Partition> A, std::shared_ptr<Partition> B,
		int xs, int xe, int ys, int ye, int zs = 0, int ze = 0);

	static std::shared_ptr<Boundary> findBoundary(
		std::shared_ptr<Partition> partition,
		std::shared_ptr<Partition> other
	);

	void computeForcingTerms();
};
