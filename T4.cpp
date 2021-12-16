// C++ Program to check if a file exists
#include <string>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

int main()
{
    

/**************************/
/*
 string filepath = "opt/raida/agent.primary.config";
    string dirpath = "opt/raida/";

    if(filepath.find(dirpath) != string::npos) {
        cout << "File exists";
    }

*/

/***************************/
/*
#include <boost/filesystem.hpp>

if( boost::filesystem::exists("agent.primary.config")) {
    cout <<"file exists" << endl;
}

*/

/*****************/
/* 
#include <io.h>

if(_access(filename, 0) == -1) {
    cout <<"File exists";
}
*/

/********************/
/*
#include <fstream>
bool check(const char* filename) {
    ifstream infile(filename);
    return infile.good();
}
*/

/*********************/
/*
#include <sys/stat.h>

bool fileexists(const std::string& file) {
    struct stat buf;
    return (stat(file.c_str(), &buf) == 0);
}

int main() {
    if(fileexists() == false) {
        std::cerr <<doesn't exist<< endl;
    }
}

*/

}