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

#include <atd/dollarcostaveraging.hpp>

namespace atd {

std::time_t DollarCostAveraging::_next_date()
{
    auto now = std::time(nullptr);
    auto now_tm = std::gmtime(&now);
    std::tm ret = _date;
    ret.tm_year = now_tm->tm_year;

    if (_monthly) {
        // if i've not reached the day of the month (and hour and minutes)
        // current month is the mont of the next date
        if (now_tm->tm_mday < _date.tm_mday) {
            ret.tm_mon = now_tm->tm_mon;
        }
        else if (now_tm->tm_mday == _date.tm_mday) {
            if (now_tm->tm_hour < _date.tm_hour) {
                ret.tm_mon = now_tm->tm_mon;
            }
            else if (now_tm->tm_hour == _date.tm_hour) {
                if (now_tm->tm_min < _date.tm_min) {
                    ret.tm_mon = now_tm->tm_mon;
                }
                else {
                    ret.tm_mon += 1;
                }
            }
            else {
                ret.tm_mon += 1;
            }
        }
        else {
            // else, the next month is the actual +1
            ret.tm_mon += 1;
        }
    }
    else {
        // the month of starting is the actual month
        ret.tm_mon = now_tm->tm_mon;
        // daily
        // if now <= specified hour
        // then today is the day and the hour is the specified one
        if (now_tm->tm_hour <= _date.tm_hour && now_tm->tm_min < _date.tm_min) {
            ret.tm_mday = now_tm->tm_mday;
        }
        else {
            // the specified date is expired, tomorrow will be the day
            ret.tm_mday = now_tm->tm_mday + 1;
        }
    }
    return timegm(&ret);
}

void DollarCostAveraging::buy(const currency_pair_t &pair)
{
    // This buy strategy should trigger a buy notification
    // when the specified day-hour-minute (UTC) of the month is reached.
    while (true) {
        auto date = _next_date();
        std::this_thread::sleep_until(
            std::chrono::system_clock::from_time_t(date));

        atd::message_t message = {};
        at::order_t order = {};
        order.action = at::order_action_t::buy;
        order.type = at::order_type_t::market;
        order.pair = pair;
        message.budget.quote = _buy_quantity;
        message.order = order;
        _chan->put(message);
        // Sleep for 1 second otherwise _next_date()
        // that has a second precision will trigger the
        // same date until a second is passed
        std::this_thread::sleep_for(1s);
    }
}

}  // end namespace atd
