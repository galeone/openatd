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

#ifndef ATD_SMALL_CHANGES_STRATEGY_H_
#define ATD_SMALL_CHANGES_STRATEGY_H_

#include <at/fiat.hpp>
#include <atd/strategy.hpp>

namespace atd {

using namespace std::literals::chrono_literals;

class SmallChanges : public Strategy {
private:
    // the amount to buy & sell
    quantity_t _buy_quantity = {}, _sell_quantity = {};
    std::shared_ptr<channel<feedback_t>> _feedback;
    bool _bought = false;
    double _price = 0;
    std::chrono::minutes _trade_period;
    std::chrono::minutes _stats_period;
    double _margin_profit_percentage = 0;
    double _dip_percentage = 0;
    at::Fiat _fiat;

public:
    SmallChanges(std::shared_ptr<DataMonitor> monitors,
                 std::shared_ptr<channel<message_t>> chan,
                 quantity_t buy_quantity, quantity_t sell_quantity)
        : Strategy(monitors, chan)
    {
        _buy_quantity = buy_quantity;
        _sell_quantity = sell_quantity;
        _feedback = std::make_shared<channel<feedback_t>>();
        _stats_period = 96h;
        _trade_period = 90min;
        _margin_profit_percentage = 0.1;
        _dip_percentage = 0.1;
    }

    void buy(const currency_pair_t &pair) override;
    void sell(const currency_pair_t &pair) override;
};
}  // end namespace atd

#endif  // ATD_SMALL_CHANGES_STRATEGY_H_
