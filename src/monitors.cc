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

#include <monitors.hpp>

namespace atd {

Monitors::Monitors(SQLite::Database* db) : _db(db)
{
    _db->exec(
        "CREATE TABLE IF NOT EXISTS monitored_pairs("
        "market text,"
        "base text,"
        "quote text,"
        "day_volume_usd big int,"
        "day_volume_btc double,"
        "price_usd double,"
        "price_btc double,"
        "percent_volume real,"
        "time datetime default current_timestamp)");
    _db->exec(
        "CREATE TABLE IF NOT EXISTS monitored_currencies("
        "currency text,"
        "time datetime,"
        "price_btc double,"
        "price_usd double,"
        "day_volume_usd big int,"
        "market_cap_usd big int,"
        "percent_change_1h real,"
        "percent_change_24h real,"
        "percent_change_7d real)");
};

// begin currencies monitor function
void Monitors::currencies(const std::vector<std::string>& currencies)
{
    SQLite::Statement query(
        *_db,
        "INSERT INTO monitored_currencies"
        "(currency,time,price_btc,price_usd,"
        "day_volume_usd,market_cap_usd,percent_change_1h,"
        "percent_change_24h,percent_change_7d) VALUES ("
        "?, datetime(?, 'unixepoch'), ?, ?, ?, ?, ?, ?, ?)");
    auto cmc = CoinMarketCap();
    while (true) {
        auto i = 0;
        for (const auto& currency : currencies) {
            auto tick = cmc.ticker(currency);
            SQLite::bind(query,
                         tick.symbol,  // currency
                         static_cast<long long int>(
                             tick.last_updated),  // time, requires
                                                  // a well known
                                                  // type
                         tick.price_btc, tick.price_usd, tick.day_volume_usd,
                         tick.market_cap_usd, tick.percent_change_1h,
                         tick.percent_change_24h, tick.percent_change_24h);
            query.exec();
            // Reset prepared statement, so it's ready to be
            // re-executed
            query.reset();

            i++;
            if ((i % 10) == 0) {
                i = 0;
                std::this_thread::sleep_for(1min);
            }
        }
        std::this_thread::sleep_for(15min);
    }
};
// end currencies monitor function

// begin pairs monitor function
void Monitors::pairs(const std::vector<currency_pair_t>& pairs)
{
    std::map<std::string, std::set<std::string>> aggregator;
    for (const auto& pair : pairs) {
        aggregator[pair.first].insert(pair.second);
    }

    SQLite::Statement query(*_db,
                            "INSERT INTO monitored_pairs("
                            "market,base,quote,day_volume_usd, day_volume_btc,"
                            "price_usd,price_btc,percent_volume)"
                            "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    auto cmc = CoinMarketCap();
    while (true) {
        for (const auto & [ base, quotes ] : aggregator) {
            auto markets = cmc.markets(base);
            for (const auto& market : markets) {
                if (quotes.find(market.pair.second) != quotes.end()) {
                    SQLite::bind(query, market.name, market.pair.first,
                                 market.pair.second, market.day_volume_usd,
                                 market.day_volume_btc, market.price_usd,
                                 market.price_btc, market.percent_volume);
                    query.exec();
                    // Reset prepared statement, so it's ready to be
                    // re-executed
                    query.reset();
                }
            }
            std::this_thread::sleep_for(2s);
        }
        std::this_thread::sleep_for(15min);
    }
};
// end pairs monitor function
};  // namespace atd
