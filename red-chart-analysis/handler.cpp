#include "handler.h" 

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

    return true;
}

bool toBool(const std::string& str) {
    std::string s = str;
    for (auto& c : s) c = tolower(c);

    return (s == "true" || s == "True" || s == "1");
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
