#include "Boundary.h"
#include "Partition.h"
#include <iostream>
#include <fstream>
#include <algorithm>

Boundary::Boundary(BoundaryType bType, double absorp, std::shared_ptr<Partition> A, std::shared_ptr<Partition> B,
	int xs, int xe, int ys, int ye, int zs, int ze)
	: type(bType), absorption(absorp), a(A), b(B), xStart(xs), xEnd(xe), yStart(ys), yEnd(ye), zStart(zs), zEnd(ze) {
}

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
		// Boundary needs global start/end coordinates for a 3-unit wide
		// area on both sides of the boundary. This is due to the 6th order
		// approximation in the interface handling equations.
		int xStart = (isRightBoundary ? x12 - 3 : x11 - 3);
		int xEnd = xStart + 6;
		return std::make_shared<Boundary>(X_BOUNDARY, 1.0, partition, other, xStart, xEnd, yStart, yEnd);
	} else if (yOverlap == 0 && xOverlap > 0) {
		bool isBottomBoundary = (y12 == y21);
		int xOffset = x21 - x11;
		int xStart = x11 + std::max(0, xOffset);
		int xEnd = x11 + std::min(partition->width, other->width + xOffset);
		int yStart = (isBottomBoundary ? y12 - 3 : y11 - 3);
		int yEnd = yStart + 6;
		return std::make_shared<Boundary>(Y_BOUNDARY, 1.0, partition, other, xStart, xEnd, yStart, yEnd);
	}
	return nullptr;
}

void Boundary::computeForcingTerms() {
	static int count = 0;
	if (type == X_BOUNDARY) {
		bool isALeft = (xStart >= a->globalX) && (xStart <= a->globalX + a->width);
		auto left = isALeft ? a : b;
		auto right = isALeft ? b : a;
		//if (count < 300)
		//out << count++ << "," << left->getPressure(150, 80) << std::endl;
		for (int i = yStart; i < yEnd; i++) {
			int lY = i - left->globalY;
			int rY = i - right->globalY;
			for (int j = -3; j < 3; j++) { // Compute 3 voxel layer on either side of boundary
				int lX = left->width + j;
				int rX = j;
				double sip = 0.0;
				double sip1 = 0.0;
				double sip2 = 0.0;

				// Forcing Coefficients
				double coefs[][7] = {
					{ 0,   0,    0,   0,    0,   -2,  2 },
					{ 0,   0,    0,   -2,   27,  -27, 2 },
					{ 0,   -2,   27,  -270, 270, -27, 2 },
					{ 2,   -27,  270, -270, 27,  -2,  0 },
					{ 2,   -27,  27,  -2,   0,   0,   0 },
					{ 2,   -2,   0,   0,    0,   0,   0 } };

				int c_idx = 0;
				for (int k = lX - 3; k < left->width; k++, c_idx++) { // Sum terms from left side of boundary
					sip1 += coefs[j + 3][c_idx] * left->getPressure(k, lY) / 180.0;
				}
				for (int k = 0; c_idx < 7; k++, c_idx++) { // Sum terms from right side of boundary
					sip2 += coefs[j + 3][c_idx] * right->getPressure(k, rY) / 180.0;
				}
				if (j < 0) {
					// F = c^2 * sip;
					if (left->includeSelfTerms) {
						sip = sip1 + sip2;
					} else {
						sip = sip2;
					}
					left->setForce(lX, lY, absorption*(left->speedOfSound)*(left->speedOfSound)*sip);
				} else {
					if (right->includeSelfTerms) {
						sip = sip1 + sip2;
					} else {
						sip = sip1;
					}
					right->setForce(rX, rY, absorption*(right->speedOfSound)*(right->speedOfSound)*sip);
				}
			}
		}
	} else if (type == Y_BOUNDARY) {
		bool isATop = (yStart >= a->globalY) && (yStart <= a->globalY + a->height);
		auto top = isATop ? a : b;
		auto bottom = isATop ? b : a;
		for (int i = xStart; i < xEnd; i++) {
			int tX = i - top->globalX;
			int bX = i - bottom->globalX;
			for (int j = -3; j < 3; j++) { // Compute 3 voxel layer on either side of boundary
				int tY = top->height + j;
				int bY = j;
				double sip = 0.0;
				double sip1 = 0.0;
				double sip2 = 0.0;

				// Forcing Coefficients
				double coefs[][7] = {
					{ 0,   0,    0,   0,    0,   -2,  2 },
					{ 0,   0,    0,   -2,   27,  -27, 2 },
					{ 0,   -2,   27,  -270, 270, -27, 2 },
					{ 2,   -27,  270, -270, 27,  -2,  0 },
					{ 2,   -27,  27,  -2,   0,   0,   0 },
					{ 2,   -2,   0,   0,    0,   0,   0 } };

				int c_idx = 0;
				for (int k = tY - 3; k < top->height; k++, c_idx++) { // Sum terms from top side of boundary
					sip1 += coefs[j + 3][c_idx] * top->getPressure(tX, k) / 180.0;
				}
				for (int k = 0; c_idx < 7; k++, c_idx++) { // Sum terms from bottom side of boundary
					sip2 += coefs[j + 3][c_idx] * bottom->getPressure(bX, k) / 180.0;
				}
				if (j < 0) {
					// F = c^2 * sip;
					if (top->includeSelfTerms) {
						sip = sip1 + sip2;
					} else {
						sip = sip2;
					}
					top->setForce(tX, tY, absorption*(top->speedOfSound)*(top->speedOfSound)*sip);
				} else {
					if (bottom->includeSelfTerms) {
						sip = sip1 + sip2;
					} else {
						sip = sip1;
					}
					bottom->setForce(bX, bY, absorption*(bottom->speedOfSound)*(bottom->speedOfSound)*sip);
				}
			}
		}
	}
}
