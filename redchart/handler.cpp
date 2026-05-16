#include "handler.h"

// Utils 

int saferStoi(const std::string& str, const std::string& label = "") {
    try {
        return std::stoi(str);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument for stoi (" << label << "): \"" << str << "\"\n";
        throw;
    } catch (const std::out_of_range& e) {
        std::cerr << "Out of range for stoi (" << label << "): \"" << str << "\"\n";
        throw;
    }
}

bool toBool(const std::string& str) {
    std::string s = str;
    for (auto& c : s) c = tolower(c);

    return (s == "true" || s == "True" || s == "1");
}


// Config Handling

bool CfgHandler::load(const std::string&cfgPath){
    std::ifstream red(cfgPath);

    if (!red.is_open()){
        std::cerr << "failed opening " << cfgPath << "." << std::endl;
        return false;
    }

    std::string lnn; 

    while(std::getline(red, lnn)){
        if (lnn.empty() || lnn[0] == '#') continue;
        
        std::istringstream iss(lnn);
        std::string key, vlu;
        if (std::getline(iss, key, '=') && std::getline(iss, vlu)){
            cfg[key] = vlu;
        }
    };
    red.close();
    return true; 
}

std::string CfgHandler::getStr(const std::string&ky, const std::string&fallback) const{
 auto it = cfg.find(ky);
//  val = it->second;
//  return it != cfg.end() ? val: fallback;

//handle safely 
 if (it != cfg.end()) {
    return it->second;
 }
 return fallback;
}

bool CfgHandler::getBool(const std::string&ky, const bool&fallback) const{
 auto it = cfg.find(ky);

 bool val; 
 //converts string value to bool
 if (it != cfg.end()) {
    val = (it->second == "true" || it->second == "1" || it->second == "yes");
    return val;
 }

 return fallback;

}

// Csv Handling

void parseLine(const std::string& line, std::unordered_map<Time, Candlestick>& Chart, int&lines) {
    // yes i vibecoded this beacuse it works.
    //thank you mr. gpt :)

    std::istringstream iss(line);

    // Parse date
    std::string datetime;
    iss >> datetime;  // 2025-04-15
    std::istringstream dateStream(datetime);
    std::string yearStr, monthStr, dayStr;
    std::getline(dateStream, yearStr, '-');
    std::getline(dateStream, monthStr, '-');
    std::getline(dateStream, dayStr, '-');
   
    int year = saferStoi(yearStr, "year");
    int month = saferStoi(monthStr, "month");
    int day = saferStoi(dayStr, "day");

    // Parse time
    std::string timeStr;
    iss >> timeStr;  // 04:56:00
    std::istringstream timeStream(timeStr);
    std::string hourStr, minStr, secStr;
    std::getline(timeStream, hourStr, ':');
    std::getline(timeStream, minStr, ':');
    std::getline(timeStream, secStr, ':');

    int hour = saferStoi(hourStr, "hour");
    int minute = saferStoi(minStr, "minute");
    int second = saferStoi(secStr, "second");

    // Parse OHLCV data
    double open, high, low, close;
    double volume; // Use double if needed
    iss >> open >> high >> low >> close >> volume;

    // Construct Time + Candlestick
    Time t { year, month, day, hour, minute, second };
    Candlestick c { open, high, low, close, volume };

    // Insert into chart
    std::cout << "#" << lines << " Parsed [O,M,D,Y]: " << "" << open << "|" << month  << "|" << day << "|" << year << "" << std::endl;
    Chart[t] = c;

}


bool CsvHandler::load(const std::string&csvPath){
    std::ifstream red0xx(csvPath);

    if (!red0xx.is_open()){
        std::cerr << "failed opening " << csvPath << "." << std::endl;
        return false;
    }

    std::string lnn; 
    
    std::getline(red0xx, lnn); // skip header
    int lines = 0;
    while(std::getline(red0xx, lnn)){
        if (lnn.empty()) continue;
        lines += 1;
        parseLine(lnn, Chart, lines);
    };
    std::cout << std::endl << "Successfully parsed CSV to hashmap!" << std::endl;
    red0xx.close();
    return true; 
}

