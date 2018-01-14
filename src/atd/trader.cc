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

void Trader::intramarket(
    at::Market& market,
    const std::map<currency_pair_t, std::unique_ptr<Strategy>>& strategies)

{
    auto decisor = [&]() {
        message_t message = {};
        _console_logger->info("Trader::intramarket: waiting");
        while (_chan->get(message)) {
            auto order = message.order;
            auto quantity = message.quantity;
            bool retry = true;
            while (retry) {
                try {
                    // handle buy orders
                    if (order.action == at::order_action_t::buy) {
                        // can I buy pair.first with pair.second?
                        // eg: LTC/EUR
                        auto balance =
                            market.balance(order.pair.second);  // 100 EUR

                        // Use a percentage of the available balance
                        double trade_balance = 0;
                        if (quantity.balance_percentage > 0) {
                            trade_balance = quantity.balance_percentage *
                                            balance;  // 0.2 * 100 = 20 EUR
                        }
                        else {
                            // Use a fixed amount, if avaiable, otherwise 0
                            if (quantity.fixed_amount <= balance) {
                                trade_balance =
                                    quantity.fixed_amount;  // 20 EUR
                            }
                        }

                        // check if order can be fulfilled (in limits, hence
                        // positive) and there's no other order for the same
                        // pair and the same action trade if it's present,
                        // remove it and place the new order
                        auto info = market.info(order.pair);
                        double fee = 0;
                        // info.{maker,taker}_fee are a percentage. eg. 0.16
                        // means 0.16% of the cost
                        if (order.type == at::order_type_t::limit) {
                            // In case of limit order, I can estimate the order
                            // cost and the volume of first I want to buy

                            // How many items of first can I buy with this
                            // balance given the requested price?
                            // 20(EUR) / 60(EUR/LTC) = 0.33 LTC
                            order.volume = trade_balance / order.price;

                            // keep track of the cost
                            order.cost =
                                order.volume *
                                order.price;  // 0.33 *60 = 19.8 EUR/LTC
                            fee = order.cost * info.maker_fee /
                                  100;  // 19.8 * 0.0016 = 0.3164
                        }
                        else if (order.type == at::order_type_t::market) {
                            // In case of market order, I don't fix the price
                            // but I can get an estimate of the price I will pay
                            // looking at the ticker
                            order.price = market.ticker(order.pair).ask.price;
                            order.volume = trade_balance / order.price;
                            order.cost = order.volume * order.price;
                            fee = order.cost * info.taker_fee / 100;
                        }

                        if (order.volume >= info.limit.min &&
                            order.volume <= info.limit.max &&
                            balance - trade_balance - fee >= 0) {
                            _console_logger->info(
                                "Trader::intramarket. BUY "
                                "Pair: {} "
                                "Balance: {} "
                                "Trade balanace: {} "
                                "Volume: {} "
                                "Price: {} "
                                "Estimated cost: {} "
                                "Estimated fees: {}",
                                order.pair, balance, trade_balance,
                                order.volume, order.price, order.cost, fee);

                            // market.place(order);
                        }
                    }
                    else {
                        // handle sell order
                        // can I sell pair.first(LTC) for pair.second(EUR)?
                        auto balance =
                            market.balance(order.pair.first);  // 10 LTC

                        // The percentage of pair.first of the balance to sell
                        double trade_balance = 0;
                        if (quantity.balance_percentage > 0) {
                            // 0.2 * 10 = 2 LTC
                            trade_balance =
                                quantity.balance_percentage * balance;
                        }
                        else {
                            if (balance - quantity.fixed_amount >= 0) {
                                // 10 - 2 = 8 => trade_balance = 2
                                trade_balance = quantity.fixed_amount;
                            }
                        }

                        // How many items of pair.first can I sell
                        // given the specified trade balance?
                        order.volume = trade_balance;

                        auto info = market.info(order.pair);
                        double fee = 0;
                        // info.{maker,taker}_fee are a percentage. eg. 0.16
                        // means 0.16% of the cost
                        if (order.type == at::order_type_t::limit) {
                            // In limit order I set the price, hence
                            // the cost is what I expect to earn
                            // 60 EUR/LTC (second) * 2 LTC = 120 EUR
                            order.cost = order.price * order.volume;
                            fee = order.cost * info.maker_fee /
                                  100;  // 60 * 0.0016 = 0.096
                        }
                        else if (order.type == at::order_type_t::market) {
                            // In market order I dont't fix the price but I can
                            // get and estimate of the applied sell price using
                            // the ticker
                            order.price = market.ticker(order.pair).bid.price;
                            order.cost = order.price * order.volume;
                            fee = order.cost * info.taker_fee / 100;
                        }

                        if (order.volume >= info.limit.min &&
                            order.volume <= info.limit.max &&
                            balance - trade_balance - fee >= 0) {
                            _console_logger->info(
                                "Trader::intramarket. SELL "
                                "Pair: {} "
                                "Balance: {} "
                                "Trade balanace: {} "
                                "Volume: {} "
                                "Price: {} "
                                "Estimated return: {} "
                                "Estimated fees: {}",
                                order.pair, balance, trade_balance,
                                order.volume, order.price, order.cost, fee);

                            // market.place(order);
                        }
                    }

                    _console_logger->info("Trader::intramarket: waiting");
                    message = {};
                    retry = false;
                }
                catch (const at::server_error& e) {
                    // catch only server error.
                    // response_erro should make the process die
                    // because it's a malformed request that have to be fixed
                    // instead, a server error is usually an overloaded server
                    // error
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
    for (const auto & [ pair, strategy ] : strategies) {
        sell_strategies.push_back(
            std::thread(&Strategy::sell, strategy.get(), std::ref(pair)));
        buy_strategies.push_back(
            std::thread(&Strategy::buy, strategy.get(), std::ref(pair)));
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
