#include "Partition.h"
#include "Boundary.h"

Partition::Partition(int gx, int gy, int w, int h, int d)
		: globalX(gx)
		, globalY(gy)
		, width(w)
		, height(h)
		, depth(d) {
	for (int i = 0; i < w; i++) {
		topFreeBorders.push_back(true);
		bottomFreeBorders.push_back(true);
	}
	for (int i = 0; i < h; i++) {
		leftFreeBorders.push_back(true);
		rightFreeBorders.push_back(true);
	}
}

Partition::~Partition() {}

void Partition::addBoundary(Boundary* boundary) {
	if (boundary->type == Boundary::X_BOUNDARY) {
		if ((boundary->xStart <= globalX) && (boundary->xEnd >= globalX)) {
			// Left side
			for (int i = boundary->yStart; i < boundary->yEnd; i++) {
				leftFreeBorders[i - globalY] = false;
			}
		} else {
			// Right side
			for (int i = boundary->yStart; i < boundary->yEnd; i++) {
				rightFreeBorders[i - globalY] = false;
			}
		}
	} else if (boundary->type == Boundary::Y_BOUNDARY) {
		if ((boundary->yStart <= globalY) && (boundary->yEnd >= globalY)) {
			// Top
			for (int i = boundary->xStart; i < boundary->xEnd; i++) {
				topFreeBorders[i - globalX] = false;
			}
		}
		else {
			// Bottom
			for (int i = boundary->xStart; i < boundary->xEnd; i++) {
				bottomFreeBorders[i - globalX] = false;
			}
		}
	}
}
