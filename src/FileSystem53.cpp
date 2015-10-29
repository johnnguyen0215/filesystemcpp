#include "FileSystem53.hpp"

#include <iostream>
#include <string>
#include <vector>

#include <sstream>
#include <fstream> // student imported


FileSystem53::FileSystem53(int DESC_BLOCK_NO, int MAX_ENTRY_NO, int MAX_ENTRY_BYTES_PER_BLOCK,
		int CURR_NO_FILES,int TOTAL_DIRECTORY_BLOCKS_USED, int TOTAL_DIRECTORY_BYTES,
		int L,int B,int K,int FILE_SIZE_FIELD,int ARRAY_SIZE,int MAX_FILE_NO,
		int MAX_BLOCK_NO,int MAX_FILE_NAME_LEN,int MAX_OPEN_FILE,int FILEIO_BUFFER_SIZE,
		int _EOF) : DESC_BLOCK_NO(DESC_BLOCK_NO), MAX_ENTRY_NO(MAX_ENTRY_NO), MAX_ENTRY_BYTES_PER_BLOCK(MAX_ENTRY_BYTES_PER_BLOCK),
		CURR_NO_FILES(CURR_NO_FILES), TOTAL_DIRECTORY_BLOCKS_USED(TOTAL_DIRECTORY_BLOCKS_USED), TOTAL_DIRECTORY_BYTES(TOTAL_DIRECTORY_BYTES),
		L(L), B(B), K(K), FILE_SIZE_FIELD(FILE_SIZE_FIELD), ARRAY_SIZE(ARRAY_SIZE), MAX_FILE_NO(MAX_FILE_NO), MAX_BLOCK_NO(MAX_BLOCK_NO),
		MAX_FILE_NAME_LEN(MAX_FILE_NAME_LEN), MAX_OPEN_FILE(MAX_OPEN_FILE), FILEIO_BUFFER_SIZE(FILEIO_BUFFER_SIZE), _EOF(_EOF){
	DESCR_SIZE = FILE_SIZE_FIELD+ARRAY_SIZE;
	MAX_BLOCK_NO_DIV8 = MAX_BLOCK_NO/8;
	OFT_SIZE = B + 2;
	format();
	OpenFileTable();
}

void FileSystem53::OpenFileTable() {
	// initializes OFT[3][66]; use OFT[index][B] to access current position
	OFT = new char*[MAX_OPEN_FILE];
	for (int i=0; i<MAX_OPEN_FILE; ++i)
		OFT[i] = new char[OFT_SIZE];

	for (int i=0; i<MAX_OPEN_FILE; ++i)
		for (int j=0; j<B+2; ++j)
			OFT[i][j] = -1;
}

int FileSystem53::find_oft() {
	for (int i=0; i<MAX_OPEN_FILE; ++i)
		if (OFT[i][B] == -1){  // find an OFT with null position
			OFT[i][B] = 0;
			return i;
		}

	return -1;
}

void FileSystem53::deallocate_oft(int index) {
	for (int i=0; i<B+2; i++){
		OFT[index][i] = -1;
	}
}



void FileSystem53::format() {
	ldisk = new char*[L];

	for (int i=0; i<L; ++i)
		ldisk[i] = new char[B];

	desc_table = new char*[K];
	for (int i=0; i<K; ++i)
		desc_table[i] = new char[B];


	for (int i=0; i<L; ++i)
		for (int j=0; j<B; ++j)
			ldisk[i][j] = -1;



	for (int i=0; i<K; i++){
		for (int j=0; j<B; j++){
			desc_table[i][j] = ldisk[i][j];
		}
	}

	int dataBlockIndex = find_empty_block();

	unsigned char c = dataBlockIndex;
	TOTAL_DIRECTORY_BLOCKS_USED++;
	desc_table[1][1] = c;
	desc_table[1][0] = 0;


}


//copy char array, starting from pointer p to the buffer
void FileSystem53::read_block(int i, char *p) {
	for (int j=0; j<B; ++j)
		p[j] = ldisk[i][j];
}


void FileSystem53::write_block(int i, char *p) {

	for (int j=0; j<B; ++j){
		ldisk[i][j] = p[j];
	}
}

char* FileSystem53::read_descriptor(int no) {
	return desc_table[DESC_BLOCK_NO] + no; // if every descrip is 4 bytes, 4*no goes to beginning of descrip
}

void FileSystem53::clear_descriptor(int no) {
	desc_table[DESC_BLOCK_NO][no] = -1; // reset file descrip

	char deleteBlock[B];
	for (int i=0; i<B; ++i)
		deleteBlock[i] = -1;

	for (int i=1; i<DESCR_SIZE; ++i) {
		int blockToDelete = desc_table[DESC_BLOCK_NO][no+i]; // get block number from file descriptor

		if (blockToDelete == -1) // if no more blocks to delete, break
			break;

		write_block(blockToDelete, deleteBlock);
		desc_table[0][blockToDelete] = -1; // reset bytemap blocks
		desc_table[DESC_BLOCK_NO][no+i] = -1; // reset file descriptor block
	}
}

void FileSystem53::write_descriptor(int no){
	for (int i=1; i<DESCR_SIZE; ++i){
		if (desc_table[DESC_BLOCK_NO][no+i] == -1) {
			int dataBlockIndex = find_empty_block();
			if (dataBlockIndex == -1){
				return;
			}
			desc_table[DESC_BLOCK_NO][no+i] = dataBlockIndex;
			break;
		}
	}
}

int FileSystem53::find_empty_descriptor(){
	for (int i=0; i<B; i+=DESCR_SIZE)
		if (desc_table[DESC_BLOCK_NO][i] == -1)
			return i;

	return -1;
}



int FileSystem53::find_empty_block(){
	for (int i = K; i < L; i++)
		if(desc_table[0][i] == -1){
			desc_table[0][i] = '1';
			return i;
		}

	return -1;
}

int FileSystem53::fgetc(int index){
	if (feof(index)) // check if end of file
		return _EOF;

	int currPosition = (unsigned char)OFT[index][B]++ % B;
	return OFT[index][currPosition]; // return char at cp and increment cp
}



int FileSystem53::fputc(int c, int index){
	if ((int)(unsigned char)OFT[index][B] >= B * ARRAY_SIZE) // check if end of file
		return -2;

	int currPosition = (unsigned char)OFT[index][B]++ % B;

	OFT[index][currPosition] = c; // add char and increment cp

	return c;
}


bool FileSystem53::feof(int index){
	return OFT[index][(int)(unsigned char)OFT[index][B]%B] == -1 ? true : false;
}

int FileSystem53::deleteFile(std::string fileName){
	int search = search_dir(fileName);
	if (search == -1){
		return -1;
	}

	int directoryBlockIndex = int(desc_table[1][(search/MAX_ENTRY_BYTES_PER_BLOCK)+1]);
	int directoryDataIndex = search%MAX_ENTRY_BYTES_PER_BLOCK;

	char dirBlock[B];
	read_block(directoryBlockIndex, dirBlock);
	int descriptorIndex = dirBlock[directoryDataIndex+10];

	int openFileDirIndex;
	for (int i=0; i<MAX_OPEN_FILE; ++i){
		openFileDirIndex = OFT[i][B+1]/B;
		read_block((int)desc_table[1][openFileDirIndex+1], dirBlock);
		if ((int)dirBlock[((int)OFT[i][B+1]%B)+10] == descriptorIndex){
			return -2;
		}
	}

	clear_descriptor(descriptorIndex); // clearing descriptor from desc_table[2], bytemap, and its associated datablocks

	read_block(directoryBlockIndex, dirBlock);

	for (int i=0; i<MAX_ENTRY_NO; ++i)                     // Setting entry in directory to -1
		dirBlock[directoryDataIndex+i] = -1;


	write_block(directoryBlockIndex, dirBlock);

	if (CURR_NO_FILES > 0)
		--CURR_NO_FILES;

	return 0;
}


int FileSystem53::find_unfilled_dir_block(){
	for (int i=1; i<DESCR_SIZE; ++i){
		if (desc_table[1][i] == -1){
			int dataBlockIndex = find_empty_block();
			if (dataBlockIndex == -1){
				return -1;
			}
			++TOTAL_DIRECTORY_BLOCKS_USED;
			unsigned char c = dataBlockIndex;
			desc_table[1][i] = c;
			return i;
		}
		char openDirBlock[B];
		read_block(int(desc_table[1][i]), openDirBlock);

		for (int j=0; j<B; j+=MAX_ENTRY_NO)
			if (openDirBlock[j] == -1)
				return i;
	}
	return -1;
}

int FileSystem53::create(std::string symbolic_file_name){
	// Error Checking: check if file already exists

	if (CURR_NO_FILES >= MAX_FILE_NO){
		return -1;
	}

	else if (search_dir(symbolic_file_name) != -1){
		return -2;
	}

	else{
		int descriptorIndex = find_empty_descriptor();
		desc_table[DESC_BLOCK_NO][descriptorIndex] = 0;

		write_descriptor(descriptorIndex);
		int CURR_DIRECTORY_INDEX = find_unfilled_dir_block();

		if (CURR_DIRECTORY_INDEX == -1){
			return -1;
		}

		int currDirectoryIndex = int(desc_table[1][CURR_DIRECTORY_INDEX]);

		char dirBlock[B];
		read_block(currDirectoryIndex, dirBlock);

		for (int i=0; i<B; ++i){
			if (dirBlock[i] == -1){
				unsigned int counter = i;
				for (unsigned int j = 0; j < symbolic_file_name.size(); j++)
					dirBlock[j+counter] = symbolic_file_name[j];

				counter += symbolic_file_name.size();
				int diff = (MAX_ENTRY_NO-1) - symbolic_file_name.size(); // represents how many spaces are left over if the file name size did not reach 10 characters
				for (int k = 0; k < diff; k++, counter++)
					dirBlock[counter] = ' ';

				unsigned char descIndex = descriptorIndex;
				dirBlock[counter] = descIndex;

				write_block(currDirectoryIndex, dirBlock);
				break;
			}
		}

		TOTAL_DIRECTORY_BYTES += MAX_ENTRY_NO;

		unsigned char c = TOTAL_DIRECTORY_BYTES;
		desc_table[1][0] = c;

		++CURR_NO_FILES;

		return 0;
	}
}

int FileSystem53::search_dir(std::string symbolic_file_name){
	int rawIndex = 0;

	char dirBlock[B];

	for (int i = 1; i <= TOTAL_DIRECTORY_BLOCKS_USED; i++){
		if (desc_table[1][i] == -1)
			break;

		int counter = 0;
		int currentBlock = int(desc_table[1][i]);
		read_block(currentBlock, dirBlock);
		while (counter < B){
			unsigned int size_counter= 0;
			for (unsigned int j=0; j<symbolic_file_name.size(); ++j)
				if (symbolic_file_name[j] == dirBlock[counter+j]) // here cp should be 5
					size_counter++;
				else
					break;

			if (size_counter == symbolic_file_name.size())
				return rawIndex;
			else{
				counter += MAX_ENTRY_NO;
				if (counter < B)
					rawIndex += MAX_ENTRY_NO;
			}
		}
	}

	return -1;
}


int FileSystem53::open_desc(int desc_no){
	int OFTIndex = find_oft();

	if (OFTIndex != -1) {
		OFT[OFTIndex][0] = 0; // allocate OFT
		OFT[OFTIndex][B] = 0; // cp = 0
		OFT[OFTIndex][B+1] = desc_no; // give file index to OFT
		return OFTIndex;
	}
	return -1;
}


int FileSystem53::open(std::string symbolic_file_name) {
	int directorySearch = search_dir(symbolic_file_name); // search for file
	if (directorySearch == -1) // if couldn't find file, return -1 error status
		return -1;

	int directoryIndex = directorySearch/B;
	char checkBlock[B];
	read_block((int)desc_table[1][directoryIndex+1], checkBlock);

	int descriptorIndex = checkBlock[(directorySearch%B)+10];
	int openFileDirIndex;
	for (int i = 0; i < MAX_OPEN_FILE; i++){
		openFileDirIndex = OFT[i][B+1]/B;
		read_block((int)desc_table[1][openFileDirIndex+1], checkBlock);
		if ((int)checkBlock[(OFT[i][B+1]%B) + 10] == descriptorIndex){
			return -3;
		}
	}

	int OFTEntry = find_oft(); // search for a open file table
	if (OFTEntry == -1){ // if no available OFT, return -2 error status
		return -2;
	}


	lseek(OFTEntry,0); // initialize current position


	OFT[OFTEntry][B+1] = directorySearch;  // initialize descrip number in OFT
	if (desc_table[DESC_BLOCK_NO][descriptorIndex+1] != -1)
		read_block((int)desc_table[DESC_BLOCK_NO][descriptorIndex+1], OFT[OFTEntry]);

	return OFTEntry;
}


int FileSystem53::read(int index, char* mem_area, int count) {
	if (OFT[index][B] == -1)
		return -1;

	char* fileDescriptor = read_descriptor(OFT[index][B+1]);
	if ((int)(unsigned char)OFT[index][B] >= (int)(unsigned char)fileDescriptor[0]){
		return -2;
	}

	int toRead = 0;

	int currentBlock = (((unsigned char)OFT[index][B])/B)+1;
	read_block(fileDescriptor[currentBlock], OFT[index]);

	while (toRead < count) {
		int charToAdd = fgetc(index);
		if (charToAdd == _EOF) {
			return toRead; // maximum file size reached
		}
		else if ((OFT[index][B] != 0) && (OFT[index][B]%B == 0)){
			currentBlock = (((unsigned char)OFT[index][B])/B)+1;
			if (currentBlock <= MAX_OPEN_FILE){
				read_block(fileDescriptor[currentBlock], OFT[index]);
			}
		}
		mem_area[toRead++] = charToAdd;
	}

	return toRead;
}

int FileSystem53::write(int index, char value, int count){
	if (!(0 <= index && index <= MAX_OPEN_FILE-1))
		return -3;
	else if (OFT[index][B] == -1){
		return -1;
	}

	char * fileDescriptor = read_descriptor(OFT[index][B+1]);
	int toWrite = 0;
	int fileSizeIndicator = 0;

	int currentBlock = (((unsigned char)OFT[index][B])/B)+1;
	char checkBlock[B];
	read_block((int)fileDescriptor[currentBlock], checkBlock);
	if (check_full_block(checkBlock)){
		char dataBlock[B];
		read_block(fileDescriptor[currentBlock], dataBlock);
		if (!check_equal_block((int)OFT[index][B]%B, B, OFT[index], dataBlock)){
			read_block(fileDescriptor[currentBlock], OFT[index]);
		}
		fileSizeIndicator = B; // Indicates that the dataBlock being written to is already full
	}

	while (toWrite < count) {
		int charToAdd = fputc(value, index);

		if (charToAdd == -2){
			return -2;
		} else if ( (OFT[index][B] != 0) && (OFT[index][B]%B == 0)){
			int oldBlock = currentBlock;
			currentBlock = ((unsigned char)OFT[index][B])/B+1;

			if (currentBlock < DESCR_SIZE && fileDescriptor[currentBlock] == -1){
				write_descriptor(OFT[index][B+1]);
			}
			write_block(fileDescriptor[oldBlock], OFT[index]);
			if (currentBlock < DESCR_SIZE){
				read_block(fileDescriptor[currentBlock], OFT[index]); // Set Buffer to values of next data block
			}
		}
		++toWrite;
	}
	write_block(fileDescriptor[currentBlock], OFT[index]);
	if (fileSizeIndicator != B){ // If the datablock is not full, update the size
		fileDescriptor[0] += count;
	}

	return toWrite;
}

int FileSystem53::lseek(int index, int pos) {
	if (OFT[index][B] != -1) {
		OFT[index][B] = pos;
		return 0;
	}
	return -1;
}

int FileSystem53::close(int index){
	if (OFT[index][B] != -1){
		char * fileDescriptor = read_descriptor(OFT[index][B+1]);
		int descriptorIndex = (unsigned char)OFT[index][B]/B;
		if ((OFT[index][B] != 0) && (OFT[index][B]%B == 0)){
			descriptorIndex = (unsigned char)(((OFT[index][B])/B)-1);
		}
		if (descriptorIndex+1 <= DESCR_SIZE){
			write_block((int)fileDescriptor[descriptorIndex+1], OFT[index]);
		}
		else{
			write_block((int)fileDescriptor[descriptorIndex], OFT[index]); 	// cp == 192, so descriptorIndex = 3 so we'll write
		}																	// to into block 3 of the descriptor
		deallocate_oft(index);
		return 0; // Return 0 for successful close
	}
	return -1;

}

void FileSystem53::directory(){

	char dirBlock[B];
	for (int i = 1; i < DESCR_SIZE; i++){
		if (desc_table[1][i] == -1)
			break;
		else {
			int indextoLdisk = int(desc_table[1][i]);

			read_block(indextoLdisk, dirBlock);

			for (int j=0; j<B; j+=MAX_ENTRY_NO) {
				std::string FileName = "";
				if (dirBlock[j] != -1){
					int temp = j;
					while (dirBlock[temp] != ' ')
						FileName += dirBlock[temp++];

					int size = (unsigned char)desc_table[DESC_BLOCK_NO][int(dirBlock[j+10])];
					std::cout << FileName << " " << size << " bytes" << std::endl;
				}
			}
		}
	}
}

void FileSystem53::restore(std::string fileName) {
	std::ifstream inFile(fileName.c_str());
	std::string testy;
	int toChar;

	for (int i=0; i<K; ++i){
		for (int j = 0; j<B; ++j){
			std::getline(inFile, testy, ' ');
			std::stringstream ss(testy);
			ss >> toChar;
			desc_table[i][j] = char(toChar);
		}
	}

	for (int i=K; i<L; ++i) {
		for (int j=0; j<B; ++j) {
			std::getline(inFile, testy, ' ');
			std::stringstream ss(testy);
			ss >> toChar;
			ldisk[i][j] = char(toChar); // Changes from ascii to char
		}
	}

	for (int i=0; i<MAX_OPEN_FILE; ++i) { // reset OFT
		OFT[i][B]   = -1;
		OFT[i][B+1] = -1;
	}

	inFile.close();
}


void FileSystem53::save(std::string filename) {
	std::ofstream outFile(filename.c_str());
	int charToInt;
	for (int i = 0; i < K; i++){
		for (int j = 0; j <B; j++){
			charToInt = desc_table[i][j];
			outFile << charToInt << " ";
		}
		outFile << "\n";
	}

	for (int i=K; i<L; i++){
		for (int j=0; j<B; j++) {
			charToInt = ldisk[i][j];
			outFile << charToInt << " ";
		}
		outFile << "\n";
	}

	outFile.close();
}
// Disk dump, from block 'start' to 'start+size-1'.
void FileSystem53::diskdump(int start, int size){

}

bool FileSystem53::check_full_block(char* block){
	for (int i=0; i<B; ++i)
		if (block[i] == -1)
			return false;
	return true;
}


int FileSystem53::check_OFT_cp(int index) {
	return OFT[index][B];

}

std::string FileSystem53::getFileName(int index) {
	std::string toReturn;

	// TODO probably wrong; weird for loop
	for (char* cp = ldisk[1] + (int)OFT[index][B+1]; *cp != '"'; ++cp) // get first char of OFT's filename
		toReturn += *cp;

	return toReturn;
}

bool FileSystem53::check_equal_block(int start, int stop, char* b1, char* b2){
	for (int i = start; i < stop; i++){
		if (b1[i] != b2[i]){
			return false;
		}
	}
	return true;
}






