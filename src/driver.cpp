#include <iostream>
#include <string>
#include "FileSystem53.hpp"
#include <bitset>
#include "FileSystemParser.hpp"

int main() {
	FileSystem53 a(2,11,55,0,0,0,64,64,7,1,3,14,64,32,3,64,-1);
	FileSystemParser p;
	p.parse(a);
}
