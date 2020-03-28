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

/* Returns the mean and stddev of the values in vector */
template <typename T>
const inline std::pair<T, T> mean_stdev(const std::vector<T> &vector)
{
    T sum = std::accumulate(vector.begin(), vector.end(), 0.0);
    T mean = sum / vector.size();
    std::vector<T> diff(vector.size());
    std::transform(vector.begin(), vector.end(), diff.begin(),
                   [mean](T x) { return x - mean; });
    T squared_sum =
        std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    T stdev = std::sqrt(squared_sum / vector.size());
    return std::pair(mean, stdev);
}

/* Returns the pearson correlation coefficient between the values of X and Y */
template <typename T>
inline double pearson(const std::vector<T> &X, const std::vector<T> &Y)
{
    auto n = X.size();
    T prod_sum = 0, X_sum = 0, Y_sum = 0;

    for (size_t i = 0; i < n; ++i) {
        prod_sum += X[i] * X[i];
        X_sum += X[i];
        Y_sum += Y[i];
    }
    double numerator = prod_sum - X_sum * Y_sum / n;

    T X_sum_of_squares = std::inner_product(X.begin(), X.end(), X.begin(), 0.0);
    T Y_sum_of_squares = std::inner_product(Y.begin(), Y.end(), Y.begin(), 0.0);
    double denominator = std::sqrt((X_sum_of_squares - std::pow(X_sum, 2) / n) *
                                   (Y_sum_of_squares - std::pow(Y_sum, 2) / n));
    return numerator / denominator;
}

/* Returns the slope and the slope and the intercept of the equation
 * predicted Y = intercept + X * slope */
template <typename T>
const inline std::pair<double, double> least_squares(const std::vector<T> &X,
                                                     const std::vector<T> &Y)
{
    double Y_mean, Y_stdev;
    std::tie(Y_mean, Y_stdev) = mean_stdev(Y);
    double r = pearson(X, Y);
    double X_mean, X_stdev;
    std::tie(X_mean, X_stdev) = mean_stdev(X);
    double b = r * Y_stdev / X_stdev;
    double a = Y_mean - b * X_mean;
    return std::pair(b, a);
}

/* Returns the the result fo the discrete convolution, without padding, between
 * f and g. */
template <typename T>
const inline std::vector<T> conv(std::vector<T> const &f,
                                 std::vector<T> const &g)
{
    size_t nf = f.size();
    size_t ng = g.size();
    std::vector<T> const &min_v = (nf < ng) ? f : g;
    std::vector<T> const &max_v = (nf < ng) ? g : f;
    size_t n = std::max(nf, ng) - std::min(nf, ng) + 1;
    std::vector<T> out(n, T());
    for (size_t i = 0; i < n; ++i) {
        for (int j(min_v.size() - 1), k(i); j >= 0; --j) {
            out[i] += min_v[j] * max_v[k];
            ++k;
        }
    }
    return out;
}

/* Returns a discrete 1d gaussian filter with the specified "lenght" sampling
 * data from a gaussian distribution wiht zero mean and given sigma */
const inline std::vector<double> gaussian1d(size_t length, double sigma)
{
    if (length % 2 == 0) {
        throw std::runtime_error("atd::stats::gaussian1d define an odd kernel");
    }
    std::vector<double> out(length);
    auto center = std::round(length / 2.);
    double sum = 0;
    auto two_sigma_sq = 2 * std::pow(sigma, 2);

    // build right side of the kernel, copy it to the left side
    for (size_t i = 0; i < center; ++i) {
        auto right = center - 1 + i;
        auto left = center - 1 - i;
        out[left] = out[right] = std::exp(-(std::pow(i, 2) / two_sigma_sq));
        // l + r
        sum += out[left] + out[right];
    }
    std::transform(out.begin(), out.end(), out.begin(),
                   [sum](double x) { return x / sum; });
    return out;
}

}  // namespace atd::stats

#endif  // ATD_STATS_H_
