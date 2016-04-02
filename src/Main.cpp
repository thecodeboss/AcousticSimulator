#include <SDL.h>
#include <thread>
#include <vector>
#include <memory>
#include "Simulation.h"
#include "DCTPartition.h"
#include "PMLPartition.h"
#include "FDTDPartition.h"

int main(int argc, char ** argv) {
	bool quit = false;

	// Create the scene out of partitions. This is just an arbitrary complex scene.
	// Later, we will read this data from the rectangular decomposition step.
	std::vector<std::shared_ptr<Partition>> partitions;
	partitions.push_back(std::make_shared<DCTPartition>(0, 0, 250, 250));
	partitions.push_back(std::make_shared<DCTPartition>(250, 0, 160, 160));
	partitions.push_back(std::make_shared<DCTPartition>(-40, 90, 40, 60));
	partitions.push_back(std::make_shared<DCTPartition>(60, 250, 100, 60));
	partitions.push_back(std::make_shared<DCTPartition>(350, 160, 60, 150));
	partitions.push_back(std::make_shared<DCTPartition>(180, 280, 170, 30));

	// Create the simulation out of our partitions
	auto simulation = std::make_shared<Simulation>(partitions);
	
	// Determine the size of the window based on the size of the simulation
	int sizeX = simulation->sizeX;
	int sizeY = simulation->sizeY;

	// Init SDL stuff with a window of our needed size
	SDL_Event event;
	SDL_Init(SDL_INIT_VIDEO);
	SDL_PixelFormat* fmt = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
	SDL_Window * window = SDL_CreateWindow("Acoustic Wave Equation Simulation",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, sizeX, sizeY, 0);

	// Setup renderer, surface, and texture
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Texture * texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		sizeX, sizeY);

	// Start!
	simulation->start();

	while (!quit)
	{
		while (SDL_PollEvent(&event)) {
			switch (event.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			}
		}

		// Check simulation thread to see if a new frame is available
		if (simulation->isReady()) {
			auto pixels = simulation->getPixels();
			SDL_UpdateTexture(texture, nullptr, pixels->data(), sizeX * sizeof(Uint32));
		}

		// Render the surface
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);
	}

	// Cleanup
	simulation->stop();
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
