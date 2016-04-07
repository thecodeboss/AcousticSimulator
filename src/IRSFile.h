#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <vector>
#include <memory>
#include <map>
#include <mutex>
#include "SoundSource.h"

#pragma pack(push,1)
struct IRSHeader {
	// Should always contain the text "iSim"
	char format[4] = {'i', 'S', 'i', 'm'};

	// File format version
	int32_t version{ 1 };

	// Total size of the header in bytes
	int32_t headerSize{ 44 };

	// The length of the scene in voxels
	int32_t sizeX{ 0 };

	// The height of the scene in voxels
	int32_t sizeY{ 0 };

	// The depth of the scene in voxels
	int32_t sizeZ{ 0 };

	// Number of samples per second
	int32_t samplingRate{ 44100 };

	// Speed of sound (voxels/sample)
	float speedOfSound{ 1.0f };

	// Voxels per metre
	float scale{ 65.0f };

	// The number of sources in the file
	int32_t nSources{ 0 };

	// The number of listeners in the file
	int32_t nListeners{ 0 };

	IRSHeader();
	IRSHeader(int32_t sx, int32_t sy, int32_t sz, int32_t ns, int32_t nl);
};
#pragma pack(pop)

#pragma pack(push,1)
struct Listener {
	int32_t id;
	int32_t xPos;
	int32_t yPos;
	int32_t zPos{ 0 };
	Listener();
	Listener(int32_t x, int32_t y, int32_t z);
};
#pragma pack(pop)

#pragma pack(push,1)
struct IRSSource {
	int32_t id;
	int32_t xPos;
	int32_t yPos;
	int32_t zPos{ 0 };
	int32_t type;
	int32_t nSamples;
};
#pragma pack(pop)

class IRSFile {
	std::ofstream outputFile;
	IRSHeader header;
	std::vector<std::shared_ptr<Listener>> listeners;
	std::vector<std::shared_ptr<IRSSource>> sources;
	std::vector<std::vector<float>> samples;
	std::mutex samples_mutex;
public:
	int minX, minY, minZ;
	int maxX, maxY, maxZ;

	IRSFile(std::string& fileName);
	IRSFile();
	~IRSFile();
	void writeHeader(IRSHeader& header);
	void writeSources(std::vector<std::shared_ptr<SoundSource>>& sources);
	void writeListeners(std::vector<std::shared_ptr<Listener>>& inListeners);
	void addData(double* pressureField);
	int numSamplesPrepared();
	void finalize();

	void readIRSFile(std::string filename);
};