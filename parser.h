/* Get output from cppcheck and splint utilities. */

#ifndef __codesmith_parser_h__
#define __codesmith_parser_h__
#include <string>
#include <fstream>
#include <vector>
#include <map>
#define CLANG_OUTPUT "output/clang-output.txt"
#define KWS_OUTPUT "output/kws-output.txt"

class Parser {
public:
    Parser(std::string);
    std::map<int, std::string> get_errors() { return errors; }
    std::map<int, std::string> get_style_errors() { return style_errors; }
private:
    void check();
    void parse();

    std::map<int, std::string> errors; // Actual errors
    std::map<int, std::string> style_errors; // Style errors
    std::string filename;
};

#endif
