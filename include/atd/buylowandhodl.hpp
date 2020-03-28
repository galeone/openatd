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

#ifndef ATD_BUY_LOW_AND_HODL_STRATEGY_H_
#define ATD_BUY_LOW_AND_HODL_STRATEGY_H_

#include <atd/hodl.hpp>
#include <atd/stats/namespace.hpp>

namespace atd {

using namespace at;
using namespace std::literals::chrono_literals;

class BuyLowAndHodl : public Hodl {
private:
    float _low, _balance_percentage;
    std::chrono::seconds _trade_period, _stats_period;

public:
    BuyLowAndHodl(std::shared_ptr<DataMonitor> monitors,
                  std::shared_ptr<channel<message_t>> chan, float low,
                  float balance_percentage, std::chrono::seconds trade_period,
                  std::chrono::seconds stats_period)
        : Hodl(monitors, chan),
          _low(low),
          _balance_percentage(balance_percentage),
          _trade_period(trade_period),
          _stats_period(stats_period)
    {
    }
    void buy(const currency_pair_t& pair) override;
};
}  // end namespace atd

#endif  // ATD_BUY_LOW_AND_HODL_STRATEGY_H_
