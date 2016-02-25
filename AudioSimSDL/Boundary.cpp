#include "Boundary.h"
#include "Partition.h"
#include <algorithm>

Boundary::Boundary(std::shared_ptr<Partition> A, std::shared_ptr<Partition> B,
	int xs, int xe, int ys, int ye, int zs, int ze)
	: a(A), b(B), xStart(xe), xEnd(xe), yStart(ys), yEnd(ye), zStart(zs), zEnd(ze) {}

std::shared_ptr<Boundary> Boundary::findBoundary(
	std::shared_ptr<Partition> partition,
	std::shared_ptr<Partition> other
) {
	int x11 = partition->globalX;
	int x12 = x11 + partition->width;
	int x21 = other->globalX;
	int x22 = x21 + other->width;

	int xOverlap = std::min(x12, x22) - std::max(x11, x21);
	// > 0 ==> partitions overlapping
	// = 0 ==> partitions share exact boundary
	// < 0 ==> partitions not touching

	int y11 = partition->globalY;
	int y12 = y11 + partition->height;
	int y21 = other->globalY;
	int y22 = y21 + other->height;
	int yOverlap = std::min(y12, y22) - std::max(y11, y21);

	if (xOverlap == 0 && yOverlap > 0) {
		bool isRightBoundary = (x12 == x21);
		int yOffset = y21 - y11;
		int yStart = y11 + std::max(0, yOffset);
		int yEnd = y11 + std::min(partition->height, other->height + yOffset);
		// Boundary needs global start/end coordinates for a 6-unit wide
		// area on both sides of the boundary. This is due to the 6th order
		// approximation in the interface handling equations.
		int xStart = (isRightBoundary ? x12 - 6 : x11 - 6);
		int xEnd = xStart + 12;
		return std::make_shared<Boundary>(partition, other, xStart, xEnd, yStart, yEnd);
	} else if (yOverlap == 0 && xOverlap > 0) {
		bool isBottomBoundary = (y12 == y21);
		int xOffset = x21 - x11;
		int xStart = x11 + std::max(0, xOffset);
		int xEnd = x11 + std::min(partition->width, other->width + xOffset);
		int yStart = (isBottomBoundary ? y12 - 6 : y11 - 6);
		int yEnd = yStart + 12;
		return std::make_shared<Boundary>(partition, other, xStart, xEnd, yStart, yEnd);
	}
	return nullptr;
}
