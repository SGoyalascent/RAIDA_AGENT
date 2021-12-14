// C++ Program to check if a file exists
#include <string>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

int main()
{
    std::string path = "opt/raida/";
    for (const auto & entry : fs::directory_iterator(path)) {
        std::cout << "Entry_Path: " <<entry.path() << std::endl;
        std::cout << "Entry" <<entry << std::endl;
    }
/********************************/
    /*
    fs::path filePath("opt/raida/agent.primary.config");
    std::error_code ec;
    if(fs::exists(filepath, ec)) {
        cout <<"File exists"<<endl;
        std::cerr <<ec.message() ;
    }
    */

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

/*

int CheckRaidaConfig() {

    ifstream file;

    string path_config_1 = "opt/raida/agent.primary.config"; 
    string path_config_2 = "opt/raida/agent.mirror.config";
    string path_config_3 = "opt/raida/agent.witness.config";

    file.open(path_config_1);
    if(file.is_open()) {
        file.close();
        return 1;
    }
    file.open(path_config_2);
    if(file.is_open()) {
        file.close();
        return 2;
    }
    file.open(path_config_3);
    if(file.is_open()) {
        file.close();
        return 3;
    }
}

*/

/*
 std::filesystem::path p("the_file");
    while(std::filesystem::exists(p));
    std::cout << "gone\n";

*/

}