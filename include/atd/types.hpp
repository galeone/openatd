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

#ifndef ATD_TYPES_H_
#define ATD_TYPES_H_

#include <at/market.hpp>
#include <at/types.hpp>
#include <atd/channel.hpp>

namespace atd {

typedef struct {
    double balance_percentage;
    double fixed_amount;
} quantity_t;

typedef struct {
    quantity_t base, quote;
} budget_t;

typedef struct {
    std::shared_ptr<at::Market> market;
    at::order_t order;
} feedback_t;

typedef struct {
    at::order_t order;
    budget_t budget;
    std::shared_ptr<channel<feedback_t>> feedback;
} message_t;

}  // end namespace atd

#endif
