//============================================================================
// Name        : FileSystem53.hpp
// Author      : John Nguyen, Van Nguyen , Saikripa Mukund, Ali Aijaz
// Version     :
// Copyright   : Your copyright notice
// Description : First Project Lab
//============================================================================


#ifndef FILESYSTEMPARSER_HPP
#define FILESYSTEMPARSER_HPP


#include <iostream>
#include <string>
#include <fstream>
#include "FileSystem53.hpp"


class FileSystemParser {
private:
	void cr(FileSystem53& f);
	void de(FileSystem53& f);
	void op(FileSystem53& f);
	void cl(FileSystem53& f);
	void rd(FileSystem53& f);
	void wr(FileSystem53& f);
	void sk(FileSystem53& f);
	void dr(FileSystem53& f);
	void in(FileSystem53& f);
	void sv(FileSystem53& f);

public:
	void parse(FileSystem53& f) {
		while(true) {
			std::string input;
			std::cin >> input;

			if (input == "cr") // create file
				cr(f);
			else if (input == "de") // destroy file
				de(f);
			else if (input == "op") // open file
				op(f);
			else if (input == "cl") // close file
				cl(f);
			else if (input == "rd") // read bytes
				rd(f);
			else if (input == "wr") // write bytes
				wr(f);
			else if (input == "sk") // get OFT cp
				sk(f);
			else if (input == "dr") // print directory
				dr(f);
			else if (input == "in") // initialize/restore disk
				in(f);
			else if (input == "sv") // save disk
				sv(f);
			else if (input == "quit")
				break;
			else {
				std::string waste;
				getline(std::cin, waste);
				std::cout<<"error"<<std::endl;
			}
		}
	}
};

void FileSystemParser::cr(FileSystem53& f) {
	std::string name, s;

	std::cin >> name;
	getline(std::cin, s);

	if(s.length() != 0)
		std::cout << "error" << std::endl;
	else {
		int errorCheck = f.create(name);

		if (errorCheck == -1)
			std::cout << "max number of open files has been reached" << std::endl;
		else if (errorCheck == -2)
			std::cout << "Error" << std::endl;
		else
			std::cout << "file "<< name <<" created"<<std::endl;
	}

	std::cin.clear();
}

void FileSystemParser::de(FileSystem53& f) {
	std::string name, s;

	std::cin >> name;
	getline(std::cin, s);

	if(s.length() != 0)
		std::cout << "error" << std::endl;
	else {
		int errorCheck = f.deleteFile(name);

		if (errorCheck == -1)
			std::cout << "file does not exist" << std::endl;
		else if (errorCheck == -2)
			std::cout << "File is currently open" << std::endl;
		else
			std::cout << "file " << name << " deleted" << std::endl;
	}

	std::cin.clear();
}

void FileSystemParser::op(FileSystem53& f) {
	std::string name, s;

	std::cin >> name;
	getline(std::cin, s);

	if(s.length() != 0)
		std::cout << "error" << std::endl;
	else {
		int errorCheck = f.open(name);

		if (errorCheck == -1)
			std::cout << "file not found" << std::endl;
		else if (errorCheck == -2)
			std::cout << "all OFT are occupied" << std::endl;
		else if (errorCheck == -3)
			std::cout << "file already open" << std::endl;
		else
			std::cout << "file " << name << " opened, Index=" << errorCheck+1 << std::endl;
	}

	std::cin.clear();
}

void FileSystemParser::cl(FileSystem53& f) {
	int index;
	std::cin>>index;
	if (std::cin.fail()){
		std::cin.clear();
		return;
	}


	std::string s;
	getline(std::cin, s);

	if(s.length() != 0)
		std::cout << "error" << std::endl;
	else {
		int errorCheck = f.check_OFT_cp(index-1);

		if (errorCheck == -1)
			std::cout << "entry already closed" << std::endl;
		else
			f.close(index-1);
		std::cout<<"file with index "<< index << " closed"<<std::endl;
	}

	std::cin.clear();
}

void FileSystemParser::rd(FileSystem53& f) {
	int index, count;

	std::cin >> index;
	std::cin >> count;

	std::string s;
	getline(std::cin, s);

	if (s.length() != 0)
		std::cout << "error" << std::endl;
	else {
		char* mem_area = new char[count];
		int readOutput = f.read(index-1, mem_area, count);

		if (readOutput == -1)
			std::cout << "file hasn't been opened" << std::endl;
		else if (readOutput == -2)
			std::cout << "maximum file size reached" << std::endl;
		else {
			std::cout<< readOutput <<" bytes read: ";

			for (int i=0; i<readOutput; ++i)
				std::cout << mem_area[i];

			std::cout << std::endl;
		}
		delete[] mem_area;
	}

	std::cin.clear();
}

void FileSystemParser::wr(FileSystem53& f) {
	int index, count;
	char value;

	std::cin >> index;
	std::cin >> value;
	std::cin >> count;

	std::string s;
	getline(std::cin, s);

	if (s.length() != 0)
		std::cout << "error" << std::endl;
	else {
		int errorCheck = f.write(index-1, value, count);

		if (errorCheck == -1)
			std::cout << "file hasn't been opened" << std::endl;
		else if (errorCheck == -2)
			std::cout << "end of file has been reached" << std::endl;
		else if (errorCheck == -3)
			std::cout << "file index out of bounds" << std::endl;
		else
			std::cout << count << " bytes written" << std::endl;
	}

	std::cin.clear();
}

void FileSystemParser::sk(FileSystem53& f) {
	int index, pos;

	std::cin>>index;
	std::cin>>pos;

	std::string s;
	getline(std::cin, s);

	if (s.length() != 0)
		std::cout << "error" << std::endl;
	else {
		int errorCheck = f.lseek(index-1, pos);

		if (errorCheck == -1)
			std::cout << "file hasn't been opened" << std::endl;
		else
			std::cout << "current position is " << pos << std::endl;
	}

	std::cin.clear();
}

void FileSystemParser::dr(FileSystem53& f) {
	std::string s;
	getline(std::cin, s);

	if (s.length() != 0)
		std::cout << "error" << std::endl;
	else
		std::ifstream inFile("dsk.txt");
	f.directory();
	std::cin.clear();

}

void FileSystemParser::in(FileSystem53& f) {
	//	std::string filename;
	//	std::cin >> filename;

	std::string s;
	getline(std::cin, s);

	if (s.length() != 0)
		std::cout << "error" << std::endl;
	else {
		std::ifstream inFile("dsk.txt");

		if (inFile.good()) {
			f.restore("dsk.txt");
			std::cout<<"disk restored"<<std::endl;
		} else {
			f.format();
			std::cout<<"disk initialized"<<std::endl;
		}
	}

	std::cin.clear();

}

void FileSystemParser::sv(FileSystem53& f) {
	//	std::string fileName;
	//	std::cin>>fileName;

	std::string s;
	getline(std::cin, s);

	if (s.length() != 0)
		std::cout << "error" << std::endl;
	else {
		f.save("dsk.txt");
		std::cout<<"disk saved"<<std::endl;
	}

	std::cin.clear();
}



#endif // FILESYSTEMPARSER_HPP
