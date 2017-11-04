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

#include <config.hpp>

namespace atd {

// Stupid but fast hash function
inline constexpr int _hash(const char* value)
{
    if (value == NULL) {
        return 0;
    }
    int ret = 0, i = 0;
    while (value[i] != '\0') {
        ret |= value[i] << (i % 8);
        i++;
    }
    return ret;
}

Config::Config(const std::string& path)
{
    std::ifstream ifconf(path);
    ifconf >> _config;
}

// markets returns a map of initialized markets from the configuration file
std::map<std::string, std::unique_ptr<Market>> Config::markets()
{
    std::map<std::string, std::unique_ptr<Market>> ret;
    json markets = _config["markets"];

    for (json::iterator it = markets.begin(); it != markets.end(); ++it) {
        switch (_hash(it.key().c_str())) {
            case _hash("kraken"): {
                auto value = *it;
                auto otp = value.find("otp");
                if (otp != value.end() && *otp != "") {
                    ret.insert(std::pair(
                        "kraken", std::make_unique<Kraken>(value["apiKey"],
                                                           value["apiSecret"],
                                                           value["otp"])));
                }
                else {
                    ret.insert(std::pair(
                        "kraken", std::make_unique<Kraken>(
                                      value["apiKey"], value["apiSecret"])));
                }
            } break;
            default:
                std::stringstream ss;
                ss << it.key() << " is not a valid key";
                throw std::runtime_error(ss.str());
        }
    }

    return ret;
}

// exchanges returns a map of initialized exchanges from the configuration
// file
std::map<std::string, std::unique_ptr<Exchange>> Config::exchanges()
{
    std::map<std::string, std::unique_ptr<Exchange>> ret;
    json exchanges = _config["exchanges"];

    for (json::iterator it = exchanges.begin(); it != exchanges.end(); ++it) {
        switch (_hash(it.key().c_str())) {
            case _hash("shapeshift"): {
                auto value = *it;
                if (value.find("affiliatePrivateKey") != value.end()) {
                    ret.insert(std::pair(
                        "shapeshift",
                        std::make_unique<Shapeshift>(
                            value["affiliatePrivateKey"].get<std::string>())));
                }
            } break;
            default:
                std::stringstream ss;
                ss << it.key() << " is not a valid key";
                throw std::runtime_error(ss.str());
        }
    }

    // Shapeshift is always available because it does not require any
    // credentials
    if (ret.find("shapeshift") == ret.end()) {
        ret.insert(std::pair("shapeshift", std::make_unique<Shapeshift>()));
    }
    return ret;
};

std::vector<currency_pair_t> Config::monitorPairs()
{
    std::vector<currency_pair_t> pairs;
    for (const auto& pair : _config["monitor"]["pairs"]) {
        pairs.push_back(currency_pair_t(pair.at(0), pair.at(1)));
    }
    return pairs;
};

std::vector<std::string> Config::monitorCurrencies()
{
    return _config["monitor"]["currencies"];
}

}  // end namespace atd
