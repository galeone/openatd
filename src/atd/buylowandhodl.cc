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

#include <atd/buylowandhodl.hpp>

namespace atd {

using namespace std::chrono_literals;

void BuyLowAndHodl::buy(const currency_pair_t& pair)
{
    // This buy strategy should trigger a buy notification
    // when a dip is found in the specified temporal window (stats_period).
    // If a dip is found and its value is below the "low" variable, send the
    // notification. The percentage of variation is considered in the WHOLE
    // temporal window.
    while (true) {
        bool buy_opportunity = false;
        auto stats_period_ago = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now() - _stats_period);
        auto pair_history = _monitors->pairHistory(pair, stats_period_ago);
        auto currency_history =
            _monitors->currencyHistory(pair.first, stats_period_ago);

        if (currency_history.size() <= 2) {
            std::this_thread::sleep_for(_trade_period);
            continue;
        }

        std::vector<double> prices;
        std::vector<double> changes_1h, changes_24h;
        for (const auto& point : currency_history) {
            prices.push_back(point.price_usd);
            changes_1h.push_back(point.percent_change_1h);
            changes_24h.push_back(point.percent_change_24h);
        }

        double mean, stddev;
        std::tie(mean, stddev) = stats::mean_stdev(prices);

        // I save the price every 20 minutes
        // Hence I have 3 measurements per hour and 72 measurements
        // per day.
        auto kernel = stats::gaussian1d(71, std::sqrt(stddev));

        auto conv = stats::conv(prices, kernel);
        for (size_t i = 0; i < conv.size(); ++i) {
            std::cout << "" << prices[i] << "," << conv[i] << "\n";
        }
        std::cout << "in: " << prices.size() << " o: " << conv.size() << "\n";

        /*

        double slope, intercept;
        std::tie(slope, intercept) = stats::least_squares(changes_1h,
        prices); std::cout << "1h eq: Y = a + bX  = " << intercept << " + "
        << slope
                  << " X\n";
        std::tie(slope, intercept) = stats::least_squares(changes_24h,
        prices); std::cout << "24h eq: Y = a + bX  = " << intercept << " + "
        << slope
                  << " X\n";
        auto current_price = prices[prices.size() - 1];
        std::cout << "current_price: " << current_price << "\n";

        // relation between first half of the measurements and the second
        half
        //
        //
        if (!(size % 2 == 0)) {
            --size;
        }
        // test with interleave
        // try to estimate relation between adjuacent measurements

        std::vector<double> prev, post;
        for (size_t i = 0; i < size; ++i) {
            if (i % 2 == 0) {
                prev.push_back(prices[i]);
            }
            else {
                post.push_back(prices[i]);
            }
        }

        std::tie(slope, intercept) = stats::least_squares(post, prev);
        std::cout << "halfs eq: Y = a + bX  = " << intercept << " + " <<
        slope
                  << " X\n";
        */
        /*


                // check if in the specified _stats_period the price is gone
                // below the _low threshold percentage

                // TEST
                // */

        buy_opportunity = true;
        double price = 0.1;  // buy price for ltc in eur
        // end TEST

        if (buy_opportunity) {
            buy_opportunity = false;
            atd::message_t message;

            at::order_t order;
            order.action = at::order_action_t::buy;
            order.type = at::order_type_t::limit;
            order.pair = pair;
            order.price = price;
            // volume is set to balance percentage.
            // the receiver will multiply the volume for the balance
            // of the base or quote currency, depending on the action
            order.volume = _balance_percentage;

            message.order = order;
            _chan->put(message);
        }
        std::cout << "before sleep" << std::endl;
        std::this_thread::sleep_for(_trade_period);
        std::cout << "\nafter sleep" << std::endl;
    }
}  // namespace atd

}  // end namespace atd
