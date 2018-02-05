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

#ifndef ATD_TRADER_H_
#define ATD_TRADER_H_

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>
#include <at/market.hpp>
#include <at/types.hpp>
#include <atd/channel.hpp>
#include <atd/datamonitor.hpp>
#include <atd/strategy.hpp>
#include <map>
#include <memory>

namespace atd {

using namespace at;

class Trader {
private:
    std::shared_ptr<DataMonitor> _monitors;
    std::shared_ptr<channel<message_t>> _chan;
    std::shared_ptr<spdlog::logger> _error_logger;
    std::shared_ptr<spdlog::logger> _console_logger;

    double _market_buy_price(std::shared_ptr<Market>,
                             const at::currency_pair_t&);
    double _market_sell_price(std::shared_ptr<Market>,
                              const at::currency_pair_t&);
    double _buy_trade_balance(std::shared_ptr<Market>, const message_t&);
    double _sell_trade_balance(std::shared_ptr<Market>, const message_t&);

public:
    Trader(std::shared_ptr<DataMonitor> monitors,
           std::shared_ptr<channel<message_t>> chan,
           std::shared_ptr<spdlog::logger> error_logger,
           std::shared_ptr<spdlog::logger> console_logger)
        : _monitors(monitors),
          _chan(chan),
          _error_logger(error_logger),
          _console_logger(console_logger)
    {
    }
    ~Trader() {}
    // intramarket trading function, use it in a new thread
    void intramarket(
        std::shared_ptr<Market> market,
        const std::map<currency_pair_t, std::vector<std::shared_ptr<Strategy>>>&
            strategies);
};
}  // end namespace atd

#endif  // ATD_TRADER_H_
