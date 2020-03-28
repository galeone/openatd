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

#include <atd/stats/ExponentialSmoothing.hpp>

namespace atd::stats {

template <typename T>
const std::vector<T> ExponentialSmoothing<T>::first_order(T alpha1)
{
    auto size = _ts.size();
    if (size <= 2 || alpha1 == 1) {
        return _ts;
    }
    if (_first.size() != 0) {
        return _first;
    }

    _first.push_back(_ts[0]);
    for (size_t i = 1; i < size; ++i) {
        _first.push_back(alpha1 * _ts[i] + (1 - alpha1) * _first[i - 1]);
    }
    _alpha1 = alpha1;
    return _first;
}

template <typename T>
const std::pair<std::vector<T>, std::vector<T>>
ExponentialSmoothing<T>::second_order(T alpha2)
{
    auto size = _ts.size();
    if (size <= 2 || alpha2 == 1) {
        return _ts;
    }

    if (_first.size() == 0) {
        _first = first_order(alpha2);
    }
    _second.push_back(_ts[0]);
    for (size_t i = 1; i < size; ++i) {
        _second.push_back(alpha2 * _first[i] + (1 - _alpha2) * _second[i - 1]);
    }
    _alpha2 = alpha2;

    return std::pair(_first, _second);
}

template <typename T>
const std::pair<T, T> ExponentialSmoothing<T>::second_order_forecast(size_t t)
{
    auto m = t - _second.first.size();
    if (m < 0) {
        m = 0;
    }
    auto at = 2 * _second.first[t] - _second.second[t];
    // TODO
}

}  // end namespace atd::stats
