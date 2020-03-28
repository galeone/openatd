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

#ifndef ATD_STATS_H_
#define ATD_STATS_H_

#include <algorithm>
#include <cmath>
#include <numeric>
#include <tuple>
#include <vector>

namespace atd::stats {

/* First order exponential smoothing on vector ts, using the specified alpha
 * factor*/
template <typename T>
class ExponentialSmoothing {
private:
    // smoothing factor
    T _alpha1, _alpha2;
    std::vector<T> _ts, _first, _second;

public:
    ExponentialSmoothing(std::vector<T> const &ts) { _ts = ts; }

    /* Compute first order exponential smoothing. The result is stored
     * internally and a copy of the smoothed time series is returned. */
    const std::vector<T> first_order(T alpha1);

    /* Second order exponential smoothing on vector ts, using the specified
     * alpha factor. Brown's linear exponential smoothing (LES) aka Brown's
     * double exp smoothing. Returns the second order smoothed series.*/
    const inline std::pair<std::vector<T>, std::vector<T>> second_order(
        T alpha2);

    /* Given the resulting series of second_exp_smoothing, returns the series of
     * the estimate values from 0 to the given time step. If the input time
     * series have a size of N, then the result with t <= N will be the second
     * order exponential smoothing value at t. If t > N it will be the second
     * order forecast. In both cases the formula is: a_t = 2s'_t - s''t; =
     * estimated level at time t b_t = \frac{alpha}{1-alpha}(s't - s''t); =
     * estimated trend at time t F_{t+m} = a_t + m b_t, where m is != 0 only if
     * t > N (m = N - t) The return value is the pari a_t, b_t
     * TODO: sistemare questa funzione per farla andare. Muovere classe in
     * header+cc separati
     */
    const inline std::pair<T, T> second_order_forecast(size_t t);
};

}  // namespace atd::stats

#endif  // ATD_STATS_H_
