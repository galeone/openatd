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

#include <spdlog/spdlog.h>
#include <atd/channel.hpp>
#include <atd/config.hpp>
#include <atd/datamonitor.hpp>
#include <atd/trader.hpp>
#include <atd/types.hpp>
#include <condition_variable>
#include <stdexcept>
#include <thread>

using namespace atd;

std::mutex mux;
std::condition_variable thread_exception_condtion;

static std::exception_ptr currencies_monitor_thread_except;
static std::exception_ptr pairs_monitor_thread_except;
static std::exception_ptr intramarket_trader;

void exception_handler(std::shared_ptr<spdlog::logger> error_logger)
{
    while (true) {
        // wait until exception
        std::unique_lock<std::mutex> lock(mux);
        thread_exception_condtion.wait(lock);

        // if here, exception has been thrown
        // and this thread has the lock
        std::string tid;
        try {
            if (currencies_monitor_thread_except) {
                tid = "currencies monitor";
                std::rethrow_exception(currencies_monitor_thread_except);
            }
            if (pairs_monitor_thread_except) {
                tid = "pairs monitor";
                std::rethrow_exception(pairs_monitor_thread_except);
            }
            if (intramarket_trader) {
                tid = "intramarket profit maker";
                std::rethrow_exception(intramarket_trader);
            }
        }
        catch (const at::response_error& e) {
            // pair unavailable for trade, for instance
            error_logger->error("{}: at::response_error: {}", tid, e.what());
        }
        catch (const at::server_error& e) {
            error_logger->error("{}: at::server_error: {}", tid, e.what());
        }
        catch (const curlpp::RuntimeError& e) {
            error_logger->error("{}: curlpp::RuntimeError: {}", tid, e.what());
        }
        catch (const curlpp::LogicError& e) {
            error_logger->error("{}: curlpp::LogicError: {}", tid, e.what());
        }
        catch (const std::logic_error& e) {
            error_logger->error("{}: std::logic_error: {}", tid, e.what());
        }
        catch (const std::runtime_error& e) {
            error_logger->error("{}: std::runtime_error {}", tid, e.what());
        }
        catch (...) {
            error_logger->error("{}: {}: unknown failure", tid,
                                typeid(std::current_exception()).name());
        }

        // discard lock
        lock.unlock();
    }
}

int main()
{
    // If we cant' create table, let the process die brutally
    SQLite::Database db("db.db3", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    // Same future for the configuration file
    Config config("config.json");
    // From config, instantiate configured markets and exchanges
    auto markets = config.markets();
    auto exchanges = config.exchanges();

    // Create monitor object, used by the monitor threads
    auto monitors = std::make_shared<DataMonitor>(&db, config.monitorPeriod());

    // Create channel of message_t
    std::shared_ptr<channel<atd::message_t>> chan =
        std::make_shared<channel<atd::message_t>>();

    // Create the trading strategies
    // pass the monitor and the shared channel.
    auto strategies = config.strategies(monitors, chan);

    // If, instead, we're here, we handle the execution of everything,
    // logging every exception to the error_logger. When an exceptin
    // occurs, log it on the error_logger, wait for 2 seconds and retry.
    auto error_logger = spdlog::rotating_logger_mt(
        "file_error_logger", "error.log", 1024 * 1024 * 5, 3);

    // console logger is used to show messages, instead of cout
    auto console_logger = spdlog::stdout_color_mt("console");

    // Creater treader object
    Trader trader(monitors, chan, error_logger, console_logger);

    // Thread for exception logging
    std::thread handler([&]() {
        console_logger->info("Started exception handler thread");
        exception_handler(error_logger);
    });

    // Thread that monitors coinmarketcap and saves infos about
    // monitored pairs
    std::thread pairs_monitor_thread([&]() {
        auto pairs = config.monitorPairs();
        std::ostringstream oss;
        for (const auto& pair : pairs) {
            oss << pair;
            oss << " ";
        }
        console_logger->info("Started pairs_monitor thread\nMonitoring: {}",
                             oss.str());
        while (true) {
            try {
                // monitors.pairs is a never ending function. It ends
                // only when an exception is thrown
                monitors->pairs(pairs);
            }
            catch (...) {
                // acquire lock
                std::lock_guard<std::mutex> lock(mux);
                pairs_monitor_thread_except = std::current_exception();
                // wake up exception handler
                thread_exception_condtion.notify_one();
                // lock is released when lock goes out of scope
            }
        }
    });

    // Thread that monitors coinmarketcap and saves info about
    // monitored currencies
    std::thread currencies_monitor_thread([&]() {
        std::ostringstream oss;
        auto currencies = config.monitorCurrencies();
        for (const auto& currency : currencies) {
            oss << currency;
            oss << " ";
        }

        console_logger->info(
            "Started currencies_monitor thread. Monitoring: {}", oss.str());
        while (true) {
            try {
                // monitors.currencies is a never ending function. It
                // ends only when an exception is thrown
                monitors->currencies(currencies);
            }
            catch (...) {
                // acquire lock
                std::lock_guard<std::mutex> lock(mux);
                currencies_monitor_thread_except = std::current_exception();
                // wake up exception handler
                thread_exception_condtion.notify_one();
                // lock is released when lock goes out of scope
            }
        }
    });

    // Threads for treader.intramarket
    std::vector<std::thread> intramarket_trading_threads;
    for (const auto& market : markets) {
        intramarket_trading_threads.push_back(std::thread([&]() {
            console_logger->info("Started trader.intramareket for market: {}",
                                 market.first);
            while (true) {
                try {
                    trader.intramarket(market.second, strategies);
                }
                catch (...) {
                    // acquire lock
                    std::lock_guard<std::mutex> lock(mux);
                    intramarket_trader = std::current_exception();
                    // wake up exception handler
                    thread_exception_condtion.notify_one();
                    // lock is released when lock goes out of scope
                }
            }
        }));
    }

    // Daemonize: wait for endless threads
    currencies_monitor_thread.join();
    pairs_monitor_thread.join();
    for (auto& thread : intramarket_trading_threads) {
        thread.join();
    }

    // should be never reached
    return 0;
}
