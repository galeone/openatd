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

#ifndef ATD_HODL_STRATEGY_H_
#define ATD_HODL_STRATEGY_H_

#include <atd/strategy.hpp>
#include <chrono>

namespace atd {

using namespace at;
using namespace std::literals::chrono_literals;

class Hodl : public Strategy {
public:
    Hodl(std::shared_ptr<DataMonitor> monitors,
         std::shared_ptr<channel<order_t>> chan)
        : Strategy(monitors, chan)
    {
    }
    void buy(const currency_pair_t&) override

    {
        while (true) {
            std::this_thread::sleep_for(5h);
        }
    }

    void sell(const currency_pair_t&) override

    {
        while (true) {
            std::this_thread::sleep_for(5h);
        }
    }
};
}  // end namespace atd

#endif  // ATD_HODL_STRATEGY_H_
