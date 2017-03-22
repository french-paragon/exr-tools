#include "getversionfield.h"

#include <fstream>

uint32_t getVersionField(const char* fileName) {
	std::ifstream f (fileName, std::ios_base::binary);
	uint32_t n;
	f.seekg (sizeof (n)); //skip magic number
	f.read ((char*) &n, sizeof(n)); //get version number

	return n;
}
