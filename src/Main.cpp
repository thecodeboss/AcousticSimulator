#include <SDL.h>
#include <thread>
#include <vector>
#include <memory>
#include "Simulation.h"
#include "DCTPartition.h"
#include "PMLPartition.h"
#include "FDTDPartition.h"
#include "GaussianSource.h"
#include "IRSFile.h"

int main(int argc, char ** argv) {
	bool quit = false;

	// Create the scene out of partitions. This is just an arbitrary complex scene.
	// Later, we will read this data from the rectangular decomposition step.
	std::vector<std::shared_ptr<Partition>> partitions = Partition::readFromRecFile("assets/test_level2.rec");

	// Sources
	std::vector<std::shared_ptr<SoundSource>> sources;
	sources.push_back(std::make_shared<GaussianSource>(75/2, -120/2, 0));

	// Create the simulation out of our partitions
	auto simulation = std::make_shared<Simulation>(
		partitions,
		"test_level2.irs"
	);
	
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
	simulation->start(sources);

	//char screenshotFilename[100];
	//int screenCount = 0;
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

			//if (screenCount < 3000) {
			//	SDL_Surface * surf = SDL_CreateRGBSurfaceFrom(pixels->data(), sizeX, sizeY, 32, sizeX * 4, 0, 0, 0, 0);
			//	sprintf_s(screenshotFilename, "screens/%04d.bmp", screenCount++);
			//	SDL_SaveBMP(surf, screenshotFilename);
			//	SDL_FreeSurface(surf);
			//}
		}

		// Render the surface
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);

		if (simulation->done()) {
			quit = true;
		}
	}

	// Cleanup
	simulation->stop();
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
//
//int main(int argc, char ** argv) {
//	IRSFile irsFile;
//	irsFile.readIRSFile("22100Hz_new2.irs");
//	return 0;
//}
