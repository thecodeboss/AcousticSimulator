#include "Partition.h"
#include "Boundary.h"
#include "DCTPartition.h"
#include <fstream>
#include <iostream>
#include <chrono>

Partition::Partition(int gx, int gy, int w, int h, int d)
		: globalX(gx)
		, globalY(gy)
		, width(w)
		, height(h)
		, depth(d) {
	worker = std::thread(&Partition::main, this);
	for (int i = 0; i < w; i++) {
		topFreeBorders.push_back(true);
		bottomFreeBorders.push_back(true);
	}
	for (int i = 0; i < h; i++) {
		leftFreeBorders.push_back(true);
		rightFreeBorders.push_back(true);
	}
}

Partition::~Partition() {
	quit = true;
	cv_wait.notify_one();
	cv_finish.notify_one();
	worker.join();
}

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

void Partition::addSource(std::shared_ptr<SoundSource>& source) {
	sources.push_back(source);
}

void Partition::computeSourceForcingTerms(double t) {
	for (auto source : sources) {
		setForce(source->x - globalX, source->y - globalY, source->sample(t));
	}
}

void Partition::main() {
	// Begin the main loop
	while (!quit) {
		// Wait for a time step to be triggered
		while (wait) {
			std::unique_lock<std::mutex> lk(cv_m_wait);
			cv_wait.wait_for(lk, std::chrono::milliseconds(100));
			if (quit) {
				break;
			}
		}
		if (quit) {
			break;
		}
		wait = true;
		step(0.0);
		finish = true;
		cv_finish.notify_one();
	}
}

void Partition::startStep(double t) {
	wait = false;
	cv_wait.notify_one();
}

void Partition::waitForStepFinish() {
	while (!finish) {
		std::unique_lock<std::mutex> lk(cv_m_finish);
		//cv_finish.wait(lk);
		cv_finish.wait_for(lk, std::chrono::milliseconds(100));
	}
	finish = false;
}

std::vector<std::shared_ptr<Partition>> Partition::readFromRecFile(std::string filename) {
	std::vector<std::shared_ptr<Partition>> partitions;
	std::ifstream file;
	file.open(filename, std::ifstream::in);
	while (file.good()) {
		int xMin, yMin, zMin;
		int width, height, depth;
		// Rectangular decomposition has y-up, while we have
		// z-up here.
		file >> xMin >> zMin >> yMin;
		file >> width >> depth >> height;
		if (file.eof()) break;

		partitions.push_back(std::make_shared<DCTPartition>(
			xMin / 2,
			yMin / 2,
			width / 2,
			height / 2));
	}

	return partitions;
}
