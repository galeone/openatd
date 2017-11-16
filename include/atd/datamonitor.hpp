/* Copyright 2017 Paolo Galeone <nessuno@nerdz.eu>. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.*/

#ifndef ATD_DATAMONITOR_H_
#define ATD_DATAMONITOR_H_

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>
#include <at/coinmarketcap.hpp>
#include <at/namespace.hpp>
#include <chrono>
#include <ctime>
#include <set>
#include <thread>

namespace atd {

using namespace at;

class DataMonitor {
private:
    SQLite::Database* _db;
    std::chrono::seconds _period;
    CoinMarketCap* _cmc;

public:
    ~DataMonitor() { delete _cmc; };
    DataMonitor(SQLite::Database* db, const std::chrono::seconds& period);
    // currencies monitor function
    void currencies(const std::vector<std::string>& currencies);
    // pairs monitor function
    void pairs(const std::vector<currency_pair_t>& pairs);

    // an ordered vector of cm_market_t from the beginning of monitoring to the
    // last saved
    std::vector<cm_market_t> pairHistory(const currency_pair_t& pair);
    // an ordered vector of cm_market_t from "after" time to the last saved
    std::vector<cm_market_t> pairHistory(const currency_pair_t& pair,
                                         const std::time_t& after);

    // an ordered vector of cm_ticker_t from the beginning of monitoring to the
    // last saved
    std::vector<cm_ticker_t> currencyHistory(const std::string& currency);
    // an ordered vector of cm_ticker_t from "after" time to the last saved
    std::vector<cm_ticker_t> currencyHistory(const std::string& currency,
                                             const std::time_t& after);
};
}  // end namespace atd

#endif  // ATD_DATAMONITOR_H_
