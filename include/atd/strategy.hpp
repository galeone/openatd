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

#ifndef ATD_STRATEGY_H_
#define ATD_STRATEGY_H_

#include <at/market.hpp>
#include <at/namespace.hpp>
#include <atd/channel.hpp>
#include <atd/datamonitor.hpp>
#include <atd/types.hpp>
#include <future>
#include <memory>
#include <thread>

namespace atd {

using namespace at;

// Strategy is an abstract class representing a trading strategy.
// Every derived class must implement the buy/sell methods
// and inherit the Strategy constructor, setting the monitors variable
// DataMonitor can be used to monitor the general markets + read the saved data
// in the db.
class Strategy {
protected:
    std::shared_ptr<DataMonitor> _monitors;
    std::shared_ptr<channel<message_t>> _chan;

public:
    Strategy(std::shared_ptr<DataMonitor> monitors,
             std::shared_ptr<channel<message_t>> chan)
        : _monitors(monitors), _chan(chan)
    {
    }
    virtual ~Strategy() {}
    virtual void buy(const currency_pair_t&) = 0;
    virtual void sell(const currency_pair_t&) = 0;
};
}  // end namespace atd

#endif  // ATD_STRATEGY_H_
