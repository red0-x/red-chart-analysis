#ifndef FETCHER_H
#define FETCHER_H

#include <string>

struct FetchResult {
    bool        success;
    int         candlesWritten;
    std::string message;
};

FetchResult fetchTicker(const std::string& ticker,
                        const std::string& interval,
                        int                candleCount,
                        const std::string& outPath);

#endif
