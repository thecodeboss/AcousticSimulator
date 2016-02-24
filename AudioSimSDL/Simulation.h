#pragma once

#include <condition_variable>
#include <vector>
#include <thread>
#include <mutex>
#include <SDL.h>

class Partition;

class Simulation {
	std::vector<std::shared_ptr<Partition>> partitions;
	std::thread worker;
	std::condition_variable cv;
	std::mutex cv_m;
	bool started{ false };
	bool ready{ false };
	bool quit{ false };
	std::vector<Uint32> pixels;
	std::mutex pixels_lock;

	// After parsing all the partitions, we determine the min
	// and max coordinates such that the entire scene fits inside
	int minX, minY, minZ;
	int maxX, maxY, maxZ;

	void main();
public:
	Simulation(std::vector<std::shared_ptr<Partition>>& p);
	~Simulation();
	
	void start();
	bool isReady();
	void stop();
	std::shared_ptr<decltype(pixels)> getPixels();
};
