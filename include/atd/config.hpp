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

#ifndef ATD_CONFIG_H_
#define ATD_CONFIG_H_

#include <at/exchange.hpp>
#include <at/kraken.hpp>
#include <at/market.hpp>
#include <at/namespace.hpp>
#include <at/shapeshift.hpp>
#include <at/types.hpp>
#include <atd/buylowandhodl.hpp>
#include <atd/channel.hpp>
#include <atd/datamonitor.hpp>
#include <atd/dollarcostaveraging.hpp>
#include <atd/hodl.hpp>
#include <atd/smallchanges.hpp>
#include <atd/strategy.hpp>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <json.hpp>
#include <set>

namespace atd {

using namespace at;

class Config {
private:
    json _config;

public:
    ~Config() {}
    Config(const std::string& path);
    // markets returns a map of initialized markets from the configuration file
    std::map<std::string, std::shared_ptr<Market>> markets();
    // exchanges returns a map of initialized exchanges from the configuration
    // file
    std::map<std::string, std::shared_ptr<Exchange>> exchanges();
    // vector of currancy_pair_t to montor
    std::vector<currency_pair_t> monitorPairs();
    // vector of currencies identifier to monitor
    std::vector<std::string> monitorCurrencies();
    // returns the number of seconds to wait between snapshots
    std::chrono::seconds monitorPeriod();
    // returns the defined strategies per pair
    std::map<currency_pair_t, std::vector<std::shared_ptr<Strategy>>>
        strategies(std::shared_ptr<DataMonitor>,
                   std::shared_ptr<channel<message_t>>);
};
}  // end namespace atd

#endif  // ATD_CONFIG_H_
