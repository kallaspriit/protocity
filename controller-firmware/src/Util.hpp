#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <sstream>
#include <vector>

class Util {

public:
    template<typename Out>
    static void split(const std::string &s, char delim, Out result) {
        std::stringstream ss;
        ss.str(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            *(result++) = item;
        }
    }


    static std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        split(s, delim, std::back_inserter(elems));
        return elems;
    }

};

#endif
