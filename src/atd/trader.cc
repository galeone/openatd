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

#include <atd/trader.hpp>

namespace atd {

double Trader::_market_buy_price(std::shared_ptr<Market> market,
                                 const at::currency_pair_t& pair)
{
    return market->ticker(pair).bid.price;
}

double Trader::_market_sell_price(std::shared_ptr<Market> market,
                                  const at::currency_pair_t& pair)
{
    return market->ticker(pair).ask.price;
}

double Trader::_buy_trade_balance(std::shared_ptr<Market> market,
                                  const message_t& message)
{
    double trade_balance = 0;
    double quote_balance = market->balance(message.order.pair.second);

    if (quote_balance <= 0) {
        return trade_balance;
    }

    // QUOTE
    if (message.budget.quote.balance_percentage > 0 ||
        message.budget.quote.fixed_amount > 0) {
        auto budget = message.budget.quote;
        if (budget.fixed_amount > 0) {
            if (budget.fixed_amount <= quote_balance) {
                trade_balance = budget.fixed_amount;  // 20 EUR
            }
        }
        else {
            trade_balance = budget.balance_percentage *
                            quote_balance;  // 0.2 * 100 = 20 EUR
        }
    }
    // BASE
    else if (message.budget.base.balance_percentage > 0 ||
             message.budget.base.fixed_amount > 0) {
        auto budget = message.budget.base;
        auto price = message.order.price > 0
                         ? message.order.price
                         : _market_sell_price(market, message.order.pair);
        if (budget.fixed_amount > 0) {
            trade_balance = budget.fixed_amount * price;

            if (trade_balance > quote_balance) {
                trade_balance = 0;
            }
        }
        else {
            trade_balance = quote_balance * price;
        }
    }
    return trade_balance;
}

double Trader::_sell_trade_balance(std::shared_ptr<Market> market,
                                   const message_t& message)
{
    double trade_balance = 0;
    double base_balance = market->balance(message.order.pair.first);

    if (base_balance <= 0) {
        return trade_balance;
    }
    // BASE
    if (message.budget.base.balance_percentage > 0 ||
        message.budget.base.fixed_amount > 0) {
        auto budget = message.budget.base;
        if (budget.balance_percentage > 0) {
            // 0.2 * 10 = 2 LTC
            trade_balance = budget.balance_percentage * base_balance;
        }
        else {
            if (base_balance - budget.fixed_amount > 0) {
                // 10 - 2 = 8 => trade_balance = 2
                trade_balance = budget.fixed_amount;
            }
        }
    }
    // QUOTE
    else if (message.budget.quote.balance_percentage > 0 ||
             message.budget.quote.fixed_amount > 0) {
        auto budget = message.budget.quote;
        auto price = message.order.price > 0
                         ? message.order.price
                         : _market_buy_price(market, message.order.pair);

        if (budget.fixed_amount > 0) {
            trade_balance = budget.fixed_amount * price;

            if (trade_balance > base_balance) {
                trade_balance = 0;
            }
        }
        else {
            trade_balance = budget.balance_percentage * price;
        }
    }

    return trade_balance;
}

void Trader::intramarket(
    std::shared_ptr<Market> market,
    const std::map<currency_pair_t, std::vector<std::shared_ptr<Strategy>>>&
        strategies)

{
    auto decisor = [&]() {
        message_t message = {};
        _console_logger->info("Trader::intramarket: waiting");
        while (_chan->get(message)) {
            auto order = message.order;

            bool retry = true;
            _console_logger->info(
                "Trader::intramarket: received message. Order type: {}, "
                "Pair: "
                "{}",
                order.action == at::order_action_t::buy ? "BUY" : "SELL",
                order.pair);

            while (retry) {
                try {
                    // handle buy orders
                    if (order.action == at::order_action_t::buy) {
                        auto balance = market->balance(order.pair.second);
                        auto trade_balance =
                            _buy_trade_balance(market, message);

                        auto info = market->info(order.pair);
                        double fee = 0;
                        // info.{maker,taker}_fee are a percentage. eg. 0.16
                        // means 0.16% of the cost
                        if (order.type == at::order_type_t::limit) {
                            order.volume = trade_balance / order.price;

                            // keep track of the cost
                            order.cost =
                                order.volume *
                                order.price;  // 0.33 *60 = 19.8 EUR/LTC
                            fee = order.cost * info.maker_fee /
                                  100;  // 19.8 * 0.0016 = 0.3164
                        }
                        else if (order.type == at::order_type_t::market) {
                            order.price =
                                _market_sell_price(market, order.pair);
                            order.volume = trade_balance / order.price;

                            order.cost = order.volume * order.price;
                            fee = order.cost * info.taker_fee / 100;
                        }

                        if (order.volume >= info.limit.min &&
                            order.volume <= info.limit.max &&
                            balance - trade_balance - fee >= 0) {
                            _console_logger->info(
                                "Trader::intramarket-> BUY "
                                "Pair: {} "
                                "Balance: {} "
                                "Trade balanace: {} "
                                "Volume: {} "
                                "Price: {} "
                                "Estimated cost: {} "
                                "Estimated fees: {}",
                                order.pair, balance, trade_balance,
                                order.volume, order.price, order.cost, fee);

                            market->place(order);
                        }
                        else {
                            _console_logger->info(
                                "Trader::intramarket-> BUY [FAIL!] "
                                "Pair: {} "
                                "Balance: {} "
                                "Trade balanace: {} "
                                "Volume: {} "
                                "Price: {} "
                                "Estimated cost: {} "
                                "Estimated fees: {} "
                                "Min volume: {} "
                                "Max volume: {} "
                                "balance - trade_balance - fee: {}",
                                order.pair, balance, trade_balance,
                                order.volume, order.price, order.cost, fee,
                                info.limit.min, info.limit.max,
                                balance - trade_balance - fee);
                        }
                    }
                    else {
                        // handle sell order
                        // can I sell pair.first(LTC) for pair.second(EUR)?
                        auto balance =
                            market->balance(order.pair.first);  // 10 LTC
                        auto trade_balance =
                            _sell_trade_balance(market, message);

                        // How many items of pair.first can I sell
                        // given the specified trade balance?
                        order.volume = trade_balance;

                        auto info = market->info(order.pair);
                        double fee = 0;
                        // info.{maker,taker}_fee are a percentage. eg. 0.16
                        // means 0.16% of the cost
                        if (order.type == at::order_type_t::limit) {
                            order.cost = order.price * order.volume;
                            fee = order.cost * info.maker_fee /
                                  100;  // 60 * 0.0016 = 0.096
                        }
                        else if (order.type == at::order_type_t::market) {
                            order.price =
                                _market_sell_price(market, order.pair);
                            order.cost = order.price * order.volume;
                            fee = order.cost * info.taker_fee / 100;
                        }

                        if (order.volume >= info.limit.min &&
                            order.volume <= info.limit.max &&
                            balance - trade_balance - fee >= 0) {
                            _console_logger->info(
                                "Trader::intramarket-> SELL "
                                "Pair: {} "
                                "Balance: {} "
                                "Trade balanace: {} "
                                "Volume: {} "
                                "Price: {} "
                                "Estimated return: {} "
                                "Estimated fees: {}",
                                order.pair, balance, trade_balance,
                                order.volume, order.price, order.cost, fee);

                            market->place(order);
                        }
                        else {
                            _console_logger->info(
                                "Trader::intramarket-> SELL [FAIL!] "
                                "Pair: {} "
                                "Balance: {} "
                                "Trade balanace: {} "
                                "Volume: {} "
                                "Price: {} "
                                "Estimated return: {} "
                                "Estimated fees: {} "
                                "Min volume: {} "
                                "Max volume: {} "
                                "balance - trade_balance - fee: {}",
                                order.pair, balance, trade_balance,
                                order.volume, order.price, order.cost, fee,
                                info.limit.min, info.limit.max,
                                balance - trade_balance - fee);
                        }
                    }

                    // Give feedback to the strategy
                    if (message.feedback != nullptr) {
                        feedback_t feedback;
                        feedback.market = market;
                        feedback.order = order;
                        message.feedback->put(feedback);
                    }

                    _console_logger->info("Trader::intramarket: waiting");

                    message = {};
                    retry = false;
                }
                catch (const at::server_error& e) {
                    // catch only server error.
                    // response_error should make the process die
                    // because it's a malformed request that have to be
                    // fixed instead, a server error is usually an
                    // overloaded server error
                    _error_logger->error(
                        "Trader::intramarket: at::server_error: {}", e.what());
                    retry = true;
                }
            }  // end while retry

        }  // end while get chan
    };     // end decisor definition

    std::thread decisor_thread(decisor);
    std::vector<std::thread> buy_strategies;
    std::vector<std::thread> sell_strategies;
    for (const auto& [pair, strategy_vector] : strategies) {
        for (const auto strategy : strategy_vector) {
            sell_strategies.push_back(
                std::thread(&Strategy::sell, strategy.get(), std::ref(pair)));
            buy_strategies.push_back(
                std::thread(&Strategy::buy, strategy.get(), std::ref(pair)));
        }
    }

    // wait for endless threads
    for (auto& buyer : buy_strategies) {
        buyer.join();
    }
    for (auto& seller : sell_strategies) {
        seller.join();
    }
    decisor_thread.join();
}

}  // end namespace atd
