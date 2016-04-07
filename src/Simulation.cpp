#include "Simulation.h"
#include "Partition.h"
#include "PMLPartition.h"
#include "Boundary.h"
#include <algorithm>
#include <memory>
#include <vector>
static volatile bool wait = false;

Simulation::Simulation(std::vector<std::shared_ptr<Partition>>& p, std::string output) : partitions(p), irsFile(output) {
	worker = std::thread(&Simulation::main, this);

	size_t num_partitions = partitions.size();
	for (size_t i = 0; i < num_partitions; i++) {
		auto partition = partitions[i];
		// Compare with every other partition to find neighboring ones
		// We need to do this to find the shared boundaries of partitions.
		for (size_t j = i+1; j < partitions.size(); j++) {
			auto other = partitions[j];
			auto boundary = Boundary::findBoundary(partition, other);
			if (boundary != nullptr) {
				boundaries.push_back(boundary);
				partition->addBoundary(boundary.get());
				other->addBoundary(boundary.get());
			}
		}
	}

	// Now for each partition, find all the edges where nothing is touching
	// and create PML boundaries there
	for (size_t i = 0; i < num_partitions; i++) {
		auto partition = partitions[i];
		// Top
		int start = 0;
		int end = 0;
		bool started = false;
		for (int i = 0; i < partition->width; i++) {
			if (started) {
				end++;
				if (partition->topFreeBorders[i] && i != partition->width - 1) {
					continue;
				} else if (start != end) {
					auto pml = std::make_shared<PMLPartition>(
						PMLPartition::P_TOP,
						partition->globalX + start,
						partition->globalY - 40,
						end - start + 1,
						40);
					partitions.push_back(pml);
					boundaries.push_back(std::make_shared<Boundary>(
						Boundary::Y_BOUNDARY,
						partition->absorption,
						pml,
						partition,
						partition->globalX + start,
						partition->globalX + end,
						partition->globalY,
						partition->globalY));
					started = false;
				}
			} else if (partition->topFreeBorders[i]) {
				start = i;
				end = i;
				started = true;
			}
		}

		// Bottom
		start = 0;
		end = 0;
		started = false;
		for (int i = 0; i < partition->width; i++) {
			if (started) {
				end++;
				if (partition->bottomFreeBorders[i] && i != partition->width - 1) {
					continue;
				} else if (start != end) {
					auto pml = std::make_shared<PMLPartition>(
						PMLPartition::P_BOTTOM,
						partition->globalX + start,
						partition->globalY + partition->height,
						end - start + 1,
						40);
					partitions.push_back(pml);
					boundaries.push_back(std::make_shared<Boundary>(
						Boundary::Y_BOUNDARY,
						partition->absorption,
						partition,
						pml,
						partition->globalX + start,
						partition->globalX + end,
						partition->globalY + partition->height,
						partition->globalY + partition->height));
					started = false;
				}
			} else if (partition->bottomFreeBorders[i]) {
				start = i;
				end = i;
				started = true;
			}
		}

		// Left
		start = 0;
		end = 0;
		started = false;
		for (int i = 0; i < partition->height; i++) {
			if (started) {
				end++;
				if (partition->leftFreeBorders[i] && i != partition->height - 1) {
					continue;
				} else if (start != end) {
					auto pml = std::make_shared<PMLPartition>(
						PMLPartition::P_LEFT,
						partition->globalX - 40,
						partition->globalY + start,
						40,
						end - start + 1);
					partitions.push_back(pml);
					boundaries.push_back(std::make_shared<Boundary>(
						Boundary::X_BOUNDARY,
						partition->absorption,
						pml,
						partition,
						partition->globalX,
						partition->globalX,
						partition->globalY + start,
						partition->globalY + end));
					started = false;
				}
			} else if (partition->leftFreeBorders[i]) {
				start = i;
				end = i;
				started = true;
			}
		}

		// Right
		start = 0;
		end = 0;
		started = false;
		for (int i = 0; i < partition->height; i++) {
			if (started) {
				end++;
				if (partition->rightFreeBorders[i] && i != partition->height - 1) {
					continue;
				} else if (start != end) {
					auto pml = std::make_shared<PMLPartition>(
						PMLPartition::P_RIGHT,
						partition->globalX + partition->width,
						partition->globalY + start,
						40,
						end - start + 1);
					partitions.push_back(pml);
					boundaries.push_back(std::make_shared<Boundary>(
						Boundary::X_BOUNDARY,
						partition->absorption,
						partition,
						pml,
						partition->globalX + partition->width,
						partition->globalX + partition->width,
						partition->globalY + start,
						partition->globalY + end));
					started = false;
				}
			} else if (partition->rightFreeBorders[i]) {
				start = i;
				end = i;
				started = true;
			}
		}
	}

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
		maxZ = std::max(maxZ, partition->globalZ + partition->depth);
	}

	sizeX = maxX - minX;
	sizeY = maxY - minY;
	sizeZ = maxZ - minZ;

	irsFile.minX = minX;
	irsFile.minY = minY;
	irsFile.minZ = minZ;
	irsFile.maxX = maxX;
	irsFile.maxY = maxY;
	irsFile.maxZ = maxZ;
}

Simulation::~Simulation() {
	worker.join();

	irsFile.finalize();
}

void Simulation::main() {
	// Wait on a condition variable until the simulation starts
	while (!started) {
		std::unique_lock<std::mutex> lk(cv_m);
		cv.wait(lk);
	}

	SDL_PixelFormat* fmt = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
	double t = 0.0;

	double* globalPressure = (double*)malloc(sizeX*sizeY*sizeof(double));
	memset((void*)globalPressure, 0, sizeX*sizeY*sizeof(double));

	// Begin the main loop
	while (!quit) {
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
		for (auto partition : partitions) {
			partition->computeSourceForcingTerms(t);
			partition->startStep(t);
		}

		for (auto partition : partitions) {
			partition->waitForStepFinish();
		}

		for (auto boundary : boundaries) {
			boundary->computeForcingTerms();
		}

		t += 0.5;
		{
			std::lock_guard<std::mutex> lock(pixels_lock);

			// Zero out all the pixels
			pixels.assign(sizeX*sizeY, 0);

			// Get pressure fields for each partition, and translate pressure values into
			// a color.
			for (auto partition : partitions) {
				if (!partition->shouldRender) continue;
				double* pressureField = partition->getPressureField();
				int width = partition->width;
				int height = partition->height;
				int gXOffset = partition->globalX - minX;
				int gYOffset = partition->globalY - minY;
				for (int i = 0; i < height; i++) {
					for (int j = 0; j < width; j++) {
						double pressure = pressureField[i*width + j];
						globalPressure[(gYOffset + i)*sizeX + (gXOffset + j)] = pressure;
						double norm = 0.5*std::max(-1.0, std::min(1.0, pressure*80.0)) + 0.5;
						int r = static_cast<int>((norm <= 0.5) ? round(255.0*(1.0 - 2.0*norm)) : 0);
						int g = static_cast<int>((norm <= 0.5) ? round(255.0*2.0*norm) : round(255.0*2.0*(1.0 - norm)));
						int b = static_cast<int>((norm >= 0.5) ? round(255.0*2.0*(norm - 0.5)) : 0);
						pixels[(gYOffset+i)*sizeX + (gXOffset+j)] = SDL_MapRGBA(fmt, 255, r, g, b);
					}
				}
			}
			ready = true;
			wait = true;
		}
		irsFile.addData(globalPressure);
	}
	free(globalPressure);
}

void Simulation::start(std::vector<std::shared_ptr<SoundSource>>& sources) {
	std::vector<std::shared_ptr<Listener>> listeners;
	double xdiff = (maxX - minX) / 20.0;
	double ydiff = (maxY - minY) / 20.0;
	for (int i = 0; i < 20; i++) {
		int xPos = floor(minX + xdiff*i);
		for (int j = 0; j < 20; j++) {
			int yPos = floor(minY + ydiff*j);
			listeners.push_back(std::make_shared<Listener>(xPos, yPos, 0));
		}
	}

	IRSHeader header(sizeX, sizeY, 0, sources.size(), listeners.size());
	irsFile.writeHeader(header);
	irsFile.writeSources(sources);
	irsFile.writeListeners(listeners);

	for (auto source : sources) {
		for (auto partition : partitions) {
			int pxmin = partition->globalX;
			int pxmax = partition->globalX + partition->width;
			int pymin = partition->globalY;
			int pymax = partition->globalY + partition->height;
			if (source->x >= pxmin && source->x <= pxmax - 1 && source->y >= pymin && source->y <= pymax - 1) {
				partition->addSource(source);
				break;
			}
		}
	}

	started = true;
	cv.notify_all();
}

bool Simulation::isReady() {
	return ready;
}

bool Simulation::done() {
	return irsFile.numSamplesPrepared() == 11025;
}

void Simulation::stop() {
	quit = true;
	wait = false;
	cv_wait.notify_one();
}

std::shared_ptr<decltype(Simulation::pixels)> Simulation::getPixels() {
	std::lock_guard<std::mutex> lock(pixels_lock);
	ready = false;
	auto copy = std::make_shared<decltype(pixels)>();
	copy->insert(copy->begin(), pixels.begin(), pixels.end());
	wait = false;
	cv_wait.notify_one();
	return copy;
}
