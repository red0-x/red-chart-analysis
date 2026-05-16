#include "fetcher.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {

struct IntervalInfo {
    const char* name;
    long        seconds;
    double      buffer;
    long        maxWindowSec; // 0 == unlimited
};

const IntervalInfo kIntervals[] = {
    {"1m",  60,      3.0,  7L      * 86400},
    {"2m",  120,     3.0,  60L     * 86400},
    {"5m",  300,     3.0,  60L     * 86400},
    {"15m", 900,     3.0,  60L     * 86400},
    {"30m", 1800,    3.0,  60L     * 86400},
    {"60m", 3600,    3.0,  730L    * 86400},
    {"1h",  3600,    3.0,  730L    * 86400},
    {"90m", 5400,    3.0,  60L     * 86400},
    {"1d",  86400,   1.5,  0},
    {"5d",  432000,  1.3,  0},
    {"1wk", 604800,  1.3,  0},
    {"1mo", 2592000, 1.1,  0},
    {"3mo", 7776000, 1.1,  0},
};

const IntervalInfo* findInterval(const std::string& name) {
    for (const auto& iv : kIntervals) {
        if (name == iv.name) return &iv;
    }
    return nullptr;
}

size_t writeCb(void* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* buf = static_cast<std::string*>(userdata);
    buf->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

std::string upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return s;
}

std::string formatTs(long ts) {
    std::time_t t = static_cast<std::time_t>(ts);
    std::tm tm{};
    gmtime_r(&t, &tm);
    char buf[80];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min, tm.tm_sec);
    return buf;
}

} // namespace

FetchResult fetchTicker(const std::string& tickerIn,
                        const std::string& interval,
                        int                candleCount,
                        const std::string& outPath) {
    FetchResult r{false, 0, ""};

    if (tickerIn.empty()) { r.message = "Ticker empty"; return r; }
    if (candleCount <= 0) { r.message = "Candle count must be > 0"; return r; }

    const IntervalInfo* iv = findInterval(interval);
    if (!iv) { r.message = "Unknown interval: " + interval; return r; }

    std::string ticker = upper(tickerIn);

    long now = static_cast<long>(std::time(nullptr));
    long span = static_cast<long>(candleCount * iv->seconds * iv->buffer);
    if (iv->maxWindowSec > 0 && span > iv->maxWindowSec) {
        span = iv->maxWindowSec;
    }
    long period1 = now - span;
    long period2 = now;

    std::ostringstream url;
    url << "https://query1.finance.yahoo.com/v8/finance/chart/" << ticker
        << "?interval=" << interval
        << "&period1=" << period1
        << "&period2=" << period2
        << "&includePrePost=false";

    CURL* curl = curl_easy_init();
    if (!curl) { r.message = "curl init failed"; return r; }

    std::string body;
    curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode cc = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);

    if (cc != CURLE_OK) {
        r.message = std::string("curl error: ") + curl_easy_strerror(cc);
        return r;
    }
    if (httpCode != 200) {
        r.message = "HTTP " + std::to_string(httpCode);
        return r;
    }

    json j;
    try {
        j = json::parse(body);
    } catch (const std::exception& e) {
        r.message = std::string("JSON parse: ") + e.what();
        return r;
    }

    try {
        const auto& chart = j.at("chart");
        if (!chart.at("error").is_null()) {
            r.message = "Yahoo error: " + chart.at("error").dump();
            return r;
        }
        const auto& result0 = chart.at("result").at(0);
        const auto& ts      = result0.at("timestamp");
        const auto& quote0  = result0.at("indicators").at("quote").at(0);
        const auto& opens   = quote0.at("open");
        const auto& highs   = quote0.at("high");
        const auto& lows    = quote0.at("low");
        const auto& closes  = quote0.at("close");
        const auto& vols    = quote0.at("volume");

        struct Row { long t; double o,h,l,c,v; };
        std::vector<Row> rows;
        rows.reserve(ts.size());
        for (size_t i = 0; i < ts.size(); ++i) {
            if (opens[i].is_null() || highs[i].is_null() ||
                lows[i].is_null()  || closes[i].is_null() ||
                vols[i].is_null()) continue;
            rows.push_back({
                ts[i].get<long>(),
                opens[i].get<double>(),
                highs[i].get<double>(),
                lows[i].get<double>(),
                closes[i].get<double>(),
                vols[i].get<double>(),
            });
        }

        if (rows.empty()) {
            r.message = "No usable rows returned";
            return r;
        }

        if (static_cast<int>(rows.size()) > candleCount) {
            rows.erase(rows.begin(), rows.end() - candleCount);
        }

        std::ofstream out(outPath);
        if (!out.is_open()) {
            r.message = "Cannot open output: " + outPath;
            return r;
        }
        out << "Time\tOpen\tHigh\tLow\tClose\tVolume\n";
        out.setf(std::ios::fixed);
        for (const auto& row : rows) {
            out.precision(6);
            out << formatTs(row.t) << '\t'
                << row.o << '\t' << row.h << '\t'
                << row.l << '\t' << row.c << '\t';
            out.precision(0);
            out << row.v << '\n';
        }
        out.close();

        r.success        = true;
        r.candlesWritten = static_cast<int>(rows.size());
        r.message        = "Wrote " + std::to_string(r.candlesWritten)
                         + " candles -> " + outPath;
        return r;
    } catch (const std::exception& e) {
        r.message = std::string("JSON shape: ") + e.what();
        return r;
    }
}
