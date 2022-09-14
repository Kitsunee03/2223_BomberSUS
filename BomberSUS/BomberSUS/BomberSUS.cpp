#include <iostream>
#include <random> 
#include <vector> 
#include <string>
#include <fstream>

void OpenFile() {
	std::fstream file;
	file.open("newlevel.sus", std::ios::in);
	if (!file.is_open()) {
		std::cout << "ERROR: \"newlevel.sus\" not found" << std::endl;
		system("pause");
		exit(0);
	}
	else { std::cout << "file opened!" << std::endl; }

	std::string line;
	std::getline(file, line);
	std::cout << line << std::endl;

	file.close();
}

int main() {
	OpenFile();

	return 0;
}