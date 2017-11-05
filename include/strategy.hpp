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

// Strategy is an interface representing a buy/sell/hodl
// strategy. If buy = sell = false -> hodl strategy
// Every implementation can decide if working with a single
// or multiple currencies and which market to use.
// Anything different from buy/sell methods can be added in the
// implementation, but it's better to let this methods the only one implemented
// and pass any other parameter in the constructor.
// This allow using the interface without dealing with the particular
// implementations
class Strategy {
public:
    virtual ~Strategy() {}

    // buy returns true when the strategy decides
    // that's time to buy
    virtual bool buy() = 0;
    // sell returns true when the strategy decides
    // that's time to sell
    virtual bool sell() = 0;
};
}  // end namespace atd
