/* parser.cc - Algorithms for CodeSmith application. */
#include "parser.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <regex>
#include <map>
#include <cstdio> // For popen()
#include <cctype> // For isdigit()

Parser::Parser(std::string farg) {
    filename = farg;
    check();
    parse();
}

void Parser::check()
{
    FILE *pipe;
    std::ofstream ofile(CLANG_OUTPUT, std::ofstream::out | std::ofstream::trunc);
    std::string clang_cmd = "clang-tidy " + filename;
    std::string kws_cmd = "KWStyle -gcc " + filename;
    int c;
    
    pipe = popen(clang_cmd.c_str(), "r"); // There should be a C++ version of this
    if (pipe == NULL) throw std::runtime_error("Unable to execute command.");
    while ((c = getc(pipe)) != EOF) {
        ofile << (char) c;
        std::cout << (char) c;
    }
    pclose(pipe);
    ofile.close();
    ofile.open(KWS_OUTPUT, std::ofstream::out | std::ofstream::trunc);
    pipe = popen(kws_cmd.c_str(), "r");
    if (pipe == NULL) throw std::runtime_error("Unable to execute command.");
    while ((c = getc(pipe)) != EOF) {
        ofile << (char) c;
        std::cout << (char) c;
    }
    pclose(pipe);
    ofile.close();
}

void Parser::parse()
{
    std::ifstream ifile(CLANG_OUTPUT, std::ifstream::in);
    std::string line;
    std::regex pattern(filename);
    std::smatch results;
    while (std::getline(ifile, line)) {
        if (std::regex_search(line, results, pattern)) {
            int i, lnum;
            std::string temp;
            std::string errmsg;
            for (i = (line.find(filename) + filename.length() + 1); isdigit(line.at(i)) && i < line.length(); i++) {
                temp += line.at(i);
            }
            errmsg = line.substr(i);
            lnum = std::atoi(temp.c_str());
            errors.insert({lnum, errmsg});
        }
    }
    for (auto it = errors.cbegin(); it != errors.cend(); it++)
        std::cout << it->first << " : " << it->second << std::endl;
    ifile.close();
    ifile.open(KWS_OUTPUT, std::ifstream::in);
    
    while (std::getline(ifile, line)) {
    if (std::regex_search(line, results, pattern)) {
            int i, lnum;
            std::string temp;
            std::string errmsg;
            for (i = (line.find(filename) + filename.length() + 1); isdigit(line.at(i)) && i < line.length(); i++) {
                temp += line.at(i);
            }
            errmsg = line.substr(i);
            lnum = std::atoi(temp.c_str());
            style_errors.insert({lnum, errmsg});
        }
    }
    for (auto it = style_errors.cbegin(); it != style_errors.cend(); it++)
        std::cout << it->first << " : " << it->second << std::endl;
    ifile.close();

}
