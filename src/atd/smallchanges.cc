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

#include <atd/smallchanges.hpp>

namespace atd {

void SmallChanges::buy(const currency_pair_t &pair)
{
    while (true) {
        auto stats_period_ago = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now() - _stats_period);

        auto currency_history =
            _monitors->currencyHistory(pair.first, stats_period_ago);

        std::size_t chsize = static_cast<std::size_t>(currency_history.size());

        if (chsize <= 8) {
            std::this_thread::sleep_for(_trade_period);
            continue;
        }

        // if the last quarter of measurement are "big dip", bargain => buy

        // BARGAIN
        bool bargain = true;
        std::size_t halfPeriod = chsize / 2;
        std::size_t quarter = chsize / 4;
        for (std::size_t i = halfPeriod + quarter; i < currency_history.size();
             ++i) {
            bargain = bargain && currency_history[i].percent_change_24h / 100 <=
                                     -_dip_percentage;  // negate the margin of
                                                        // profit makint it a
                                                        // negative threshold
        }

        bool longBearRun = false;
        if (!bargain) {
            // check if there's a downtrend in the overall window
            longBearRun = true;
            for (std::size_t i = 0; i < chsize; ++i) {
                longBearRun =
                    longBearRun && currency_history[i].percent_change_24h <= 0;
            }
        }

        if (longBearRun || bargain) {
            // auto price_usd = currency_history[chsize - 1].price_usd;
            // auto eur_usd_ratio = _fiat.rate(at::currency_pair_t("eur",
            // "usd"));  auto price_eur = price_usd * eur_usd_ratio;

            atd::message_t message = {};
            message.feedback = _feedback;

            at::order_t order = {};
            order.action = at::order_action_t::buy;
            // order.type = at::order_type_t::limit;
            order.type = at::order_type_t::market;
            order.pair = pair;
            // order.price = price_eur;
            message.budget.base = _buy_quantity;
            message.order = order;

            _chan->put(message);
            std::cout << "[BUY] " << pair;
            if (longBearRun) {
                std::cout << " long bear run";
            }
            if (bargain) {
                std::cout << " bargain";
            }
            std::cout << " messege putted. Waiting for feedback" << std::endl;

            feedback_t feedback;
            if (_feedback->get(feedback)) {
                order = feedback.order;
                auto market = feedback.market;
                std::cout << "[BUY] feedback: " << order.txid << std::endl;

                if (order.txid.length() > 0) {
                    bool closed = false;
                    while (!closed) {
                        std::cout << "[BUY] checking for order: " << order.txid
                                  << " fulfillment" << std::endl;
                        closed = true;
                        std::vector<at::order_t> openOrders;
                        auto retry = true;
                        while (retry) {
                            try {
                                openOrders = market->openOrders();
                                retry = false;
                            }
                            catch (const at::server_error &e) {
                                std::cout << "smallchanges: market->openOrders "
                                          << e.what() << "\n sleep and retry";
                                retry = true;
                                std::this_thread::sleep_for(1min);
                            }
                        }
                        for (const auto &openOrder : openOrders) {
                            if (openOrder.txid == order.txid) {
                                closed = false;
                                break;
                            }
                        }
                        if (!closed) {
                            std::cout << "[BUY] order not closed, "
                                         "sleepting for 1min\n";

                            std::this_thread::sleep_for(1min);
                        }
                    }

                    std::cout << "[BUY] order fulfilled!\n";
                    _bought = true;
                    _price = std::max(order.price, _price);

                    // when the order is filled, instead of restarting and
                    // trained after "trade_period" it's better to wait for
                    // some hours in order to measure a lot of snapshot of
                    // the market and do not place equals order in a short
                    // period of time
                    std::this_thread::sleep_for(12h);
                }
            }
        }

        std::this_thread::sleep_for(_trade_period);
    }
}

void SmallChanges::sell(const currency_pair_t &pair)
{
    while (true) {
        auto stats_period_ago = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now() - _stats_period);

        auto currency_history =
            _monitors->currencyHistory(pair.first, stats_period_ago);

        std::size_t chsize = static_cast<std::size_t>(currency_history.size());

        if (chsize <= 8) {
            std::this_thread::sleep_for(_trade_period);
            continue;
        }

        // BARGAIN
        bool spike = true;
        std::size_t halfPeriod = chsize / 2;
        std::size_t quarter = chsize / 4;
        for (std::size_t i = halfPeriod + quarter; i < currency_history.size();
             ++i) {
            spike = spike && currency_history[i].percent_change_24h / 100 >=
                                 _margin_profit_percentage;
        }

        bool longBullRun = false;

        if (!spike) {
            // check if there's a downtrend in the overall window
            longBullRun = true;
            for (std::size_t i = 0; i < chsize; ++i) {
                longBullRun =
                    longBullRun && currency_history[i].percent_change_24h > 0;
            }
        }

        bool canComparePrice = false;
        auto price_usd = currency_history[chsize - 1].price_usd;
        auto quote = pair.second;
        double quote_usd_ratio = 0;
        // is fiat
        try {
            quote_usd_ratio = _fiat.rate(at::currency_pair_t(quote, "usd"));
            canComparePrice = true;
        }
        catch (const std::out_of_range &) {
            // if here is not fiat, let's say is xrp/eth
            auto oneHourAgo = std::chrono::system_clock::to_time_t(
                std::chrono::system_clock::now() - 1h);
            auto ph = _monitors->pairHistory(at::currency_pair_t(quote, "usd"),
                                             oneHourAgo);
            if (ph.size() > 0) {
                // There's the pair quote, usd stored
                // find the market with the highest volume, pick the price in
                // usd from there
                size_t max_id = 0;
                long long max_volume = 0;
                for (size_t i = 0; i < ph.size(); ++i) {
                    if (ph[i].day_volume_usd > max_volume) {
                        max_volume = ph[i].day_volume_usd;
                        max_id = i;
                    }
                }
                if (max_volume > 0) {
                    // price usd = usd/eth
                    auto price_usd = ph[max_id].price_usd;
                    // hence eth/usd ratio = 1/price_usd
                    quote_usd_ratio = 1. / price_usd;
                    canComparePrice = true;
                }
            }
        }

        auto price_quote = price_usd * quote_usd_ratio;

        bool takeProfit =
            _bought && _price > 0 && canComparePrice &&
            price_quote >= _price * (1. + _margin_profit_percentage);

        if (takeProfit || spike || longBullRun) {
            std::cout << "[SELL]" << pair;
            if (takeProfit) {
                std::cout << " take profit";
            }
            if (spike) {
                std::cout << " spike";
            }
            if (longBullRun) {
                std::cout << " long bull run";
            }

            std::cout << std::endl;
            atd::message_t message = {};
            message.feedback = _feedback;

            at::order_t order = {};
            order.action = at::order_action_t::sell;
            order.pair = pair;
            if (takeProfit) {
                order.type = at::order_type_t::limit;
                order.price = price_quote;
            }
            else {
                order.type = at::order_type_t::market;
            }

            message.budget.base = _sell_quantity;
            message.order = order;

            _chan->put(message);

            feedback_t feedback;
            if (_feedback->get(feedback)) {
                order = feedback.order;
                auto market = feedback.market;
                std::cout << "[SELL] feedback: " << order.txid << std::endl;

                if (order.txid.length() > 0) {
                    bool closed = false;
                    while (!closed) {
                        std::cout << "[SELL] checking for order: " << order.txid
                                  << " fulfillment" << std::endl;

                        closed = true;
                        std::vector<at::order_t> openOrders;
                        auto retry = true;
                        while (retry) {
                            try {
                                openOrders = market->openOrders();
                                retry = false;
                            }
                            catch (const at::server_error &e) {
                                std::cout << "smallchanges: market->openOrders "
                                          << e.what() << "\n sleep and retry";
                                retry = true;
                                std::this_thread::sleep_for(1min);
                            }
                        }
                        for (const auto &openOrder : openOrders) {
                            if (openOrder.txid == order.txid) {
                                closed = false;
                                break;
                            }
                        }
                        if (!closed) {
                            std::cout << "[SELL] order not closed, "
                                         "sleepting for 1min"
                                      << std::endl;

                            std::this_thread::sleep_for(1min);
                        }
                    }

                    std::cout << "[SELL] order fulfilled!" << std::endl;
                    if (takeProfit) {
                        _bought = false;
                        _price = 0;
                    }

                    // when the order is filled, instead of restarting and
                    // trained after "trade_period" it's better to wait for
                    // some hours in order to measure a lot of snapshot of
                    // the market and do not place equals order in a short
                    // period of time
                    std::this_thread::sleep_for(spike || longBullRun ? 12h
                                                                     : 2h);
                }
            }
        }

        std::this_thread::sleep_for(_trade_period);
    }
}

}  // end namespace atd
