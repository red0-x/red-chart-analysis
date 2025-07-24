#ifndef HANDLER_H
#define HANDLER_H
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

// Config Handling

class CfgHandler {

    public:

        bool load(const std::string&cfgPath="red.cfg");
        std::string getStr(const std::string&key, const std::string&fallback="") const;
        bool getBool(const std::string&key, const bool&fallback=false) const;

    private: 
        std::unordered_map<std::string, std::string> cfg;
};

#endif