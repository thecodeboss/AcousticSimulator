#include "IRSFile.h"
#include <fftw3.h>

IRSHeader::IRSHeader() {}

IRSHeader::IRSHeader(int32_t sx, int32_t sy, int32_t sz, int32_t ns, int32_t nl)
	: sizeX(sx), sizeY(sy), sizeZ(sz), nSources(ns), nListeners(nl) {}

Listener::Listener() {}

Listener::Listener(int32_t x, int32_t y, int32_t z) : xPos(x), yPos(y), zPos(z) {
	static int32_t idGen = 0;
	id = idGen++;
}

IRSFile::IRSFile(std::string& fileName) {
	outputFile.open(fileName, std::ios::binary);
}

IRSFile::IRSFile() {

}

IRSFile::~IRSFile() {
	outputFile.close();
}

void IRSFile::writeHeader(IRSHeader& header) {
	outputFile.write((char*)&header, sizeof(IRSHeader));
}

void IRSFile::writeSources(std::vector<std::shared_ptr<SoundSource>>& sources) {
	int32_t nEntries = sources.size();
	int32_t chunkSize = 8 + 24*nEntries;
	int32_t gaussian = 0;
	int32_t nSamples = 22050;
	if (outputFile.good()) {
		outputFile.write((char*)&chunkSize, 4);
		outputFile.write((char*)&nEntries, 4);
		for (auto source : sources) {
			outputFile.write((char*)&(source->id), 4);
			outputFile.write((char*)&(source->x), 4);
			outputFile.write((char*)&(source->y), 4);
			outputFile.write((char*)&(source->z), 4);
			outputFile.write((char*)&gaussian, 4);
			outputFile.write((char*)&nSamples, 4);
		}
	}
}

void IRSFile::writeListeners(std::vector<std::shared_ptr<Listener>>& inLlisteners) {
	listeners = inLlisteners;
	int32_t nEntries = listeners.size();
	int32_t chunkSize = 8 + 16 * nEntries;
	outputFile.write((char*)&chunkSize, 4);
	outputFile.write((char*)&nEntries, 4);
	for (auto listener : listeners) {
		outputFile.write((char*)listener.get(), sizeof(Listener));
	}
	samples.resize(listeners.size());
}

void IRSFile::addData(double* pressureField) {
	std::lock_guard<std::mutex> lock(samples_mutex);
	int width = maxX - minX;
	for (auto listener : listeners) {
		int idx = (listener->yPos - minY) * width + listener->xPos - minX;
		float pressure = static_cast<float>(pressureField[idx]);
		samples[listener->id].push_back(pressure);
	}
}

int IRSFile::numSamplesPrepared() {
	std::lock_guard<std::mutex> lock(samples_mutex);
	return samples[0].size();
}

void postProcess(std::vector<float>& sampleList) {
	// Find all the peaks, and create a new impulse response out of them
	std::vector<double> peaks(sampleList.size() * 2);
	for (int i = 1; i < sampleList.size() - 1; i++) {
		if (sampleList[i] > sampleList[i - 1] && sampleList[i] > sampleList[i + 1]) {
			peaks[2*i] = static_cast<double>(sampleList[i]);
		} else if (sampleList[i] < sampleList[i - 1] && sampleList[i] < sampleList[i + 1]) {
			peaks[2*i] = static_cast<double>(sampleList[i]);
		}
	}

	// Now Fourier transform both impulse responses
	int n = sampleList.size();
	double* IR = (double*)fftw_malloc(sizeof(double) * n * 2);
	for (int i = 0; i < n-1; i++) {
		IR[2 * i] = static_cast<double>(sampleList[i]);
		IR[2 * i + 1] = static_cast<double>(sampleList[i] + sampleList[i+1])/2.0;
	}
	IR[n*2 - 1] = 0.0;
	fftw_complex* IR_fft = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * n * 2);
	fftw_complex* peaks_fft = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * n * 2);

	fftw_plan IR_fft_plan;
	fftw_plan peaks_fft_plan;
	fftw_plan ifft_plan;

	IR_fft_plan = fftw_plan_dft_r2c_1d(n*2, IR, IR_fft, FFTW_ESTIMATE);
	peaks_fft_plan = fftw_plan_dft_r2c_1d(n*2, &peaks.front(), peaks_fft, FFTW_ESTIMATE);

	fftw_execute(IR_fft_plan);
	fftw_execute(peaks_fft_plan);

	for (int i = 0; i < n*2; i++) {
		IR_fft[i][0] = IR_fft[i][0] + peaks_fft[i][0];
		IR_fft[i][1] = IR_fft[i][1] + peaks_fft[i][1];
	}

	ifft_plan = fftw_plan_dft_c2r_1d(n*2, IR_fft, IR, FFTW_ESTIMATE);
	fftw_execute(ifft_plan);

	// Update the original sample list with the new stuff
	sampleList.resize(n * 2);
	for (int i = 0; i < n * 2; i++) {
		sampleList[i] = static_cast<float>(IR[i]);
	}

	fftw_destroy_plan(IR_fft_plan);
	fftw_destroy_plan(peaks_fft_plan);
	fftw_destroy_plan(ifft_plan);
	fftw_free(IR);
	fftw_free(IR_fft);
	fftw_free(peaks_fft);
}

void IRSFile::finalize() {
	std::lock_guard<std::mutex> lock(samples_mutex);
	int32_t zero = 0;
	for (int i = 0; i < samples.size(); i++) {
		auto sampleList = samples[i];
		postProcess(sampleList);
		int32_t chunkSize = 12 + sizeof(float)*sampleList.size();
		outputFile.write((char*)&chunkSize, 4);
		outputFile.write((char*)&zero, 4);
		outputFile.write((char*)&i, 4);
		outputFile.write((char*)&sampleList.front(), sizeof(float)*sampleList.size());
	}
}

void IRSFile::readIRSFile(std::string filename) {
	std::ifstream file;
	file.open(filename, std::ios::in | std::ios::binary);
	if (file.good()) {
		// Make sure we are reading from the beginning of the file
		file.seekg(0, std::ios::beg);
		file.read((char*)&header, sizeof(header));
		int32_t size = 0;
		int32_t nEntries = 0;
		file.read((char*)&size, 4);
		file.read((char*)&nEntries, 4);
		for (int i = 0; i < nEntries; i++) {
			auto source = std::make_shared<IRSSource>();
			file.read((char*)source.get(), sizeof(IRSSource));
			sources.push_back(source);
		}
		file.read((char*)&size, 4);
		file.read((char*)&nEntries, 4);
		for (int i = 0; i < nEntries; i++) {
			auto listener = std::make_shared<Listener>();
			file.read((char*)listener.get(), sizeof(Listener));
			listeners.push_back(listener);
		}

		samples.resize(listeners.size());
		for (size_t i = 0; i < listeners.size(); i++) {
			int sourceID = 0;
			int listenerID = 0;
			file.read((char*)&size, 4);
			file.read((char*)&sourceID, 4);
			file.read((char*)&listenerID, 4);

			samples[i].resize((size - 12) / 4);
			file.read((char*)&samples[i].front(), size - 12);
		}
		std::ofstream test("test.txt");
		for (float sample : samples[75]) {
			test << sample << ", ";
		}
	}
}
