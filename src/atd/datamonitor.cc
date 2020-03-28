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

#include <atd/datamonitor.hpp>

namespace atd {

DataMonitor::DataMonitor(SQLite::Database* db,
                         const std::chrono::seconds& period)
    : _db(db), _period(period)
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

    _cmc = new CoinMarketCap();
}

// begin currencies monitor function
void DataMonitor::currencies(const std::vector<std::string>& currencies)
{
    SQLite::Statement query(
        *_db,
        "INSERT INTO monitored_currencies"
        "(currency,time,price_btc,price_usd,"
        "day_volume_usd,market_cap_usd,percent_change_1h,"
        "percent_change_24h,percent_change_7d) VALUES ("
        "?, datetime(?, 'unixepoch'), ?, ?, ?, ?, ?, ?, ?)");

    while (true) {
        auto i = 0;
        for (const auto& currency : currencies) {
            auto tick = _cmc->ticker(currency);
            SQLite::bind(query,
                         tick.symbol,  // currency
                         static_cast<long long int>(
                             tick.last_updated),  // time, requires
                                                  // a well known
                                                  // type
                         tick.price_btc, tick.price_usd, tick.day_volume_usd,
                         tick.market_cap_usd, tick.percent_change_1h,
                         tick.percent_change_24h, tick.percent_change_7d);
            query.exec();
            // Reset prepared statement, so it's ready to be
            // re-executed
            query.reset();

            i++;
            // required because of cmc api limits
            if ((i % 10) == 0) {
                i = 0;
                std::this_thread::sleep_for(std::chrono::minutes(1));
            }
        }
        std::this_thread::sleep_for(_period);
    }
}
// end currencies monitor function

// begin pairs monitor function
void DataMonitor::pairs(const std::vector<currency_pair_t>& pairs)
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

    while (true) {
        for (const auto& [base, quotes] : aggregator) {
            auto markets = _cmc->markets(base);
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
            // required because of cmc "api" limits
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        std::this_thread::sleep_for(_period);
    }
}
// end pairs monitor function

// an ordered vector of cm_market_t from "after" time to the last saved
std::vector<cm_market_t> DataMonitor::pairHistory(const currency_pair_t& pair,
                                                  const std::time_t& after)
{
    SQLite::Statement query(
        *_db,
        "SELECT market,day_volume_usd,day_volume_btc,"
        "price_usd,price_btc,percent_volume, strftime('%s', time) as timestamp "
        "FROM monitored_pairs "
        "WHERE base = ? AND quote = ? AND time >= datetime(?, 'unixepoch') "
        "ORDER BY timestamp ASC");
    SQLite::bind(query, pair.first, pair.second,
                 static_cast<long long int>(after));
    std::vector<cm_market_t> ret;
    while (query.executeStep()) {
        ret.push_back(cm_market_t{
            .name = query.getColumn("market"),
            .pair = pair,
            .day_volume_usd = query.getColumn("day_volume_usd"),
            .day_volume_btc = query.getColumn("day_volume_btc"),
            .price_usd = query.getColumn("price_usd"),
            .price_btc = query.getColumn("price_btc"),
            .percent_volume = static_cast<float>(
                query.getColumn("percent_volume").getDouble()),
            .last_updated = static_cast<std::time_t>(
                query.getColumn("timestamp").getInt64()),
        });
    }
    return ret;
}

// an ordered vector of cm_market_t from the beginning of monitoring to the
// last seved
std::vector<cm_market_t> DataMonitor::pairHistory(const currency_pair_t& pair)
{
    return pairHistory(pair, 0);
}

// an ordered vector of cm_ticker_t from "after" time to the last saved
// id, name, rank available_supply and total_supply field are unset, DO NOT use
// them
std::vector<cm_ticker_t> DataMonitor::currencyHistory(
    const std::string& currency, const std::time_t& after)
{
    SQLite::Statement query(
        *_db,
        "SELECT strftime('%s', time) as timestamp,price_btc,price_usd,"
        "day_volume_usd,market_cap_usd,percent_change_1h,"
        "percent_change_24h,percent_change_7d "
        "FROM monitored_currencies "
        "WHERE lower(currency) = lower(?) AND time >= datetime(?, 'unixepoch') "
        "ORDER BY timestamp ASC");
    SQLite::bind(query, currency, static_cast<long long int>(after));
    std::vector<cm_ticker_t> ret;
    while (query.executeStep()) {
        ret.push_back(cm_ticker_t{
            .id = "",
            .name = "",
            .symbol = currency,
            .rank = 0,
            .price_usd = query.getColumn("price_usd"),
            .price_btc = query.getColumn("price_btc"),
            .day_volume_usd = query.getColumn("day_volume_usd"),
            .market_cap_usd = query.getColumn("market_cap_usd"),
            .available_supply = 0,
            .total_supply = 0,
            .percent_change_1h = static_cast<float>(
                query.getColumn("percent_change_1h").getDouble()),
            .percent_change_24h = static_cast<float>(
                query.getColumn("percent_change_24h").getDouble()),
            .percent_change_7d = static_cast<float>(
                query.getColumn("percent_change_7d").getDouble()),
            .last_updated = static_cast<std::time_t>(
                query.getColumn("timestamp").getInt64()),
        });
    }
    return ret;
}

// an ordered vector of cm_ticker_t from the beginning of monitoring to the
// last seved
std::vector<cm_ticker_t> DataMonitor::currencyHistory(
    const std::string& currency)
{
    return currencyHistory(currency, 0);
}

}  // namespace atd
