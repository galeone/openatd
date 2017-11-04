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

#include <at/exchange.hpp>
#include <at/kraken.hpp>
#include <at/market.hpp>
#include <at/namespace.hpp>
#include <at/shapeshift.hpp>
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
    std::map<std::string, std::unique_ptr<Market>> markets();
    // exchanges returns a map of initialized exchanges from the configuration
    // file
    std::map<std::string, std::unique_ptr<Exchange>> exchanges();
    // vector of currancy_pair_t to montor
    std::vector<currency_pair_t> monitorPairs();
    // vector of currencies identifier to monitor
    std::vector<std::string> monitorCurrencies();
};
}  // end namespace atd
