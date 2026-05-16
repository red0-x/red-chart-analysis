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

void printMenu(){
    std::cout << std::endl;
    std::cout << "Redscreen | Menu" << std::endl;
    std::cout << "____________________" << std::endl;
    std::cout << "[1] Print Config" << std::endl;
    std::cout << "[2] Parse CSV" << std::endl;
    std::cout << "[3] Exit" << std::endl;
    std::cout << "____________________" << std::endl << std::endl;
   
}

void handleSelection() {
    std::cout << "Choose an option." << std::endl << ">";

    int usri = 0;
    std::string input;
    std::getline(std::cin, input);

    std::stringstream ss(input);
    if (!(ss >> usri) || (ss.peek() != EOF)) {
        std::cout << "\nChoose a valid Input. [int 1-3]" << std::endl;
        handleSelection();
        return;
    }

    switch(usri){
        case 1:
            printCfg();
            break;
        case 2: 
            printCsv();
            break;
        case 3:
            break;
        default:
            std::cout << "\nChoose a valid Input. [int 1-3]" << std::endl;
            handleSelection();
            break;
    }
}


void menu(){
 printMenu();
 handleSelection();
};

int main(){
  menu();
 return 0;   
}