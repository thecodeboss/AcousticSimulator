#pragma once

#include <condition_variable>
#include <vector>
#include <thread>
#include <mutex>
#include <SDL.h>
#include "IRSFile.h"
#include "SoundSource.h"

class Partition;
class Boundary;

class Simulation {
	std::vector<std::shared_ptr<Partition>> partitions;
	std::vector<std::shared_ptr<Boundary>> boundaries;
	std::thread worker;
	std::condition_variable cv;
	std::mutex cv_m;

	std::condition_variable cv_wait;
	std::mutex cv_m_wait;

	bool started{ false };
	bool ready{ false };
	bool quit{ false };
	std::vector<Uint32> pixels;
	std::mutex pixels_lock;
	IRSFile irsFile;

	// After parsing all the partitions, we determine the min
	// and max coordinates such that the entire scene fits inside
	int minX, minY, minZ;
	int maxX, maxY, maxZ;

	void main();
public:
	int sizeX, sizeY, sizeZ;

	Simulation(std::vector<std::shared_ptr<Partition>>& p, std::string output);
	~Simulation();
	void start(std::vector<std::shared_ptr<SoundSource>>& sources);
	bool isReady();
	bool done();
	void stop();
	std::shared_ptr<decltype(pixels)> getPixels();
};
