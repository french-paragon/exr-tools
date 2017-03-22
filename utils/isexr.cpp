#include <fstream>

bool isExrFile(const char * fileName) {
	std::ifstream f (fileName, std::ios_base::binary);
	char b[4];
	f.read (b, sizeof (b));
	return !!f && b[0] == 0x76 && b[1] == 0x2f && b[2] == 0x31 && b[3] == 0x01;
}
