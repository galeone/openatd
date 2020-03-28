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

#ifndef ATD_DOLLAR_COST_AVERAGING_STRATEGY_H_
#define ATD_DOLLAR_COST_AVERAGING_STRATEGY_H_

#include <atd/hodl.hpp>
#include <iomanip>

namespace atd {

using namespace at;
using namespace std::literals::chrono_literals;

class DollarCostAveraging : public Hodl {
private:
    // flag, if true montly buy, else daily
    bool _monthly;
    // date will only contain the (day,) hour and minutes specified
    std::tm _date;
    // the amount to buy
    atd::quantity_t _buy_quantity;

    // next_date returns the date in which the buy notification should be
    // triggered
    std::time_t _next_date();

    const char *_monthly_format = "%d %H:%M", *_daily_format = "%H:%M";

public:
    DollarCostAveraging(std::shared_ptr<DataMonitor> monitors,
                        std::shared_ptr<channel<message_t>> chan,
                        std::string date, atd::quantity_t buy_quantity)
        : Hodl(monitors, chan)
    {
        // support 2 formats: date is always UTC

        _monthly = false;
        auto now = std::time(nullptr);
        _date = *std::gmtime(&now);

        std::istringstream ss(date);
        ss >> std::get_time(&_date, _daily_format);
        if (ss.fail()) {
            std::istringstream ss(date);
            ss >> std::get_time(&_date, _monthly_format);
            if (ss.fail()) {
                std::ostringstream oss;
                oss << "Unsupported date format: ";
                oss << date;
                oss << " supported format are: ";
                oss << _monthly_format;
                oss << " & ";
                oss << _daily_format;
                throw std::runtime_error(oss.str());
            }
            else {
                _monthly = true;
            }
        }
        _buy_quantity = buy_quantity;

        // get UTC time
        auto time = timegm(&_date);
        _date = *std::gmtime(&time);
    }

    void buy(const currency_pair_t &pair) override;
};
}  // end namespace atd

#endif  // ATD_DOLLAR_COST_AVERAGING_STRATEGY_H_
