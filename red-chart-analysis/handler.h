#ifndef HANDLER_H
#define HANDLER_H
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <functional> 
#include <tuple>      


// Global Structs

struct Time {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;

        bool operator==(const Time& other) const {
            return std::tie(year, month, day, hour, minute, second) ==
                   std::tie(other.year, other.month, other.day, other.hour, other.minute, other.second);
        }
};

struct Candlestick {
        double open;
        double high;
        double low;
        double close;
        double volume;
};

// Hash Specialization

namespace std {
    template<>
    struct hash<Time> {
        std::size_t operator()(const Time& t) const {
            std::size_t h1 = std::hash<int>{}(t.year);
            std::size_t h2 = std::hash<int>{}(t.month);
            std::size_t h3 = std::hash<int>{}(t.day);
            std::size_t h4 = std::hash<int>{}(t.hour);
            std::size_t h5 = std::hash<int>{}(t.minute);
            std::size_t h6 = std::hash<int>{}(t.second);

            return ((((((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1)) >> 1) ^ (h4 << 1)) >> 1) ^ (h5 << 1) ^ (h6 << 1);
        }
    };
}

// Config Handling

class CfgHandler {

    public:

        bool load(const std::string&cfgPath="red.cfg");
        std::string getStr(const std::string&key, const std::string&fallback="") const;
        bool getBool(const std::string&key, const bool&fallback=false) const;

    private: 
        std::unordered_map<std::string, std::string> cfg;
};

// CSV Handling 

class CsvHandler {
public:
    CsvHandler() = default; 

    bool load(const std::string& csvPath = "data/eu5.csv");

    // const std::unordered_map<Time, Candlestick>& getChart() const { return Chart; }
    std::unordered_map<Time, Candlestick> Chart;
    
private:

};


#endif