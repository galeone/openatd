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

#ifndef ATD_CHAN_H_
#define ATD_CHAN_H_

#include <condition_variable>
#include <list>
#include <mutex>
#include <thread>

namespace atd {

template <class item>
class channel {
private:
    std::list<item> _queue;
    std::mutex _m;
    std::condition_variable _cv;
    bool _closed;

public:
    channel() : _closed(false) {}
    void close()
    {
        std::unique_lock<std::mutex> lock(_m);
        _closed = true;
        _cv.notify_all();
    }
    bool is_closed()
    {
        std::unique_lock<std::mutex> lock(_m);
        return _closed;
    }
    void put(const item &i)
    {
        std::unique_lock<std::mutex> lock(_m);
        if (_closed) {
            throw std::logic_error("put to _closed channel");
        }
        _queue.push_back(i);
        _cv.notify_one();
    }
    bool get(item &out, bool wait = true)
    {
        std::unique_lock<std::mutex> lock(_m);
        if (wait) {
            _cv.wait(lock, [&]() { return _closed || !_queue.empty(); });
        }
        if (_queue.empty()) {
            return false;
        }
        out = _queue.front();
        _queue.pop_front();
        return true;
    }
};
}  // end namespace atd

#endif
