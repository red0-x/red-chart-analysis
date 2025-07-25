#include "main.h"

void printCfg(){
 cfg.load("red.cfg");
    std::string ticker = cfg.getStr("ticker");
    std::string tf = cfg.getStr("tf");
    std::string data = cfg.getStr("data");
    std::string results = cfg.getStr("results");
    
 transform(ticker.begin(), ticker.end(), ticker.begin(), ::toupper);
 
    std::cout << std::endl;
    std::cout << "Redscreen | CFG" << std::endl;
    std::cout << "____________________" << std::endl;
    std::cout << "Using Ticker: [" << ticker << "]." << std::endl;
    std::cout << "Using Timeframe: [" << tf << "m]." << std::endl;
    std::cout << "CSV Path: [" << data << "]." << std::endl;
    std::cout << "Output Path: [" << results << "]." << std::endl;
    std::cout << "____________________" << std::endl;
    std::cout << std::endl << "(this is an example of the config handling, adding more functionality soon!)" << std::endl;

};

void printCsv(){
   cfg.load("red.cfg");
   std::string ticker = cfg.getStr("ticker");
   std::string tf = cfg.getStr("tf");
   std::string data = cfg.getStr("data");
   std::string results = cfg.getStr("results");
    
   std::string csvPath; 
   csvPath = (data+ticker+tf+".csv");
   csv.load(csvPath);
};

int main(){
  printCfg();
  std::cout << std::endl << std::endl;
  printCsv();
 return 0;   
}