#include "Simulation.h"
#include "Partition.h"
#include "Boundary.h"
#include <algorithm>
#include <memory>
#include <vector>

Simulation::Simulation(std::vector<std::shared_ptr<Partition>>& p) : partitions(p) {
	worker = std::thread(&Simulation::main, this);

	maxX = maxY = maxZ = std::numeric_limits<int>::min();
	minX = minY = minZ = std::numeric_limits<int>::max();
	for (size_t i = 0; i < partitions.size(); i++) {
		auto partition = partitions[i];
		// Find min and max coordinates of the entire scene
		minX = std::min(minX, partition->globalX);
		maxX = std::max(maxX, partition->globalX + partition->width);
		minY = std::min(minY, partition->globalY);
		maxY = std::max(maxY, partition->globalY + partition->height);
		minZ = std::min(minZ, partition->globalZ);
		maxZ = std::max(maxZ, partition->globalZ + +partition->depth);

		// Compare with every other partition to find neighboring ones
		// We need to do this to find the shared boundaries of partitions.
		for (size_t j = i+1; j < partitions.size(); j++) {
			auto other = partitions[j];
			auto boundary = Boundary::findBoundary(partition, other);
			if (boundary != nullptr) {
				boundaries.push_back(boundary);
			}
		}
	}

	sizeX = maxX - minX;
	sizeY = maxY - minY;
	sizeZ = maxZ - minZ;
}

Simulation::~Simulation() {
	worker.join();
}

void Simulation::main() {
	// Wait on a condition variable until the simulation starts
	while (!started) {
		std::unique_lock<std::mutex> lk(cv_m);
		cv.wait(lk);
	}

	SDL_PixelFormat* fmt = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
	double t = 0.0;

	// Begin the main loop
	while (!quit) {
		for (auto partition : partitions) {
			partition->step(t);
		}
		t += 0.5;
		{
			std::lock_guard<std::mutex> lock(pixels_lock);

			// Zero out all the pixels
			pixels.assign(sizeX*sizeY, 0);

			// Get pressure fields for each partition, and translate pressure values into
			// a color.
			for (auto partition : partitions) {
				double* pressureField = partition->getPressureField();
				int width = partition->width;
				int height = partition->height;
				int gXOffset = partition->globalX;
				int gYOffset = partition->globalY;
				for (int i = 0; i < height; i++) {
					for (int j = 0; j < width; j++) {
						double pressure = pressureField[i*width + j] / (width*height);
						double norm = 0.5*std::max(-1.0, std::min(1.0, pressure)) + 0.5;
						int r = static_cast<int>((norm <= 0.5) ? round(255.0*(1.0 - 2.0*norm)) : 0);
						int g = static_cast<int>((norm <= 0.5) ? round(255.0*2.0*norm) : round(255.0*2.0*(1.0 - norm)));
						int b = static_cast<int>((norm >= 0.5) ? round(255.0*2.0*(norm - 0.5)) : 0);
						pixels[(gYOffset+i)*sizeX + (gXOffset+j)] = SDL_MapRGBA(fmt, 255, r, g, b);
					}
				}
			}
			ready = true;
		}
	}
}

void Simulation::start() {
	started = true;
	cv.notify_all();
}

bool Simulation::isReady() {
	return ready;
}

void Simulation::stop() {
	quit = true;
}

std::shared_ptr<decltype(Simulation::pixels)> Simulation::getPixels() {
	std::lock_guard<std::mutex> lock(pixels_lock);
	ready = false;
	auto copy = std::make_shared<decltype(pixels)>();
	copy->insert(copy->begin(), pixels.begin(), pixels.end());
	return copy;
}
