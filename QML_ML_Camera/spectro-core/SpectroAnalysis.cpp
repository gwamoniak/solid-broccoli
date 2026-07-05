#include "SpectroAnalysis.h"

#include <algorithm>
#include <cmath>

namespace SpectroAnalysis {

namespace {

// Solve the small (n x n) system M * x = b in place, partial pivoting.
// Returns false if the matrix is singular.
bool solveLinearSystem(QVector<QVector<double>>& M, QVector<double>& b)
{
    const int n = b.size();
    for (int col = 0; col < n; ++col) {
        int pivot = col;
        for (int r = col + 1; r < n; ++r) {
            if (std::abs(M[r][col]) > std::abs(M[pivot][col]))
                pivot = r;
        }
        if (std::abs(M[pivot][col]) < 1e-12)
            return false;
        M.swapItemsAt(col, pivot);
        std::swap(b[col], b[pivot]);
        for (int r = col + 1; r < n; ++r) {
            const double f = M[r][col] / M[col][col];
            for (int c = col; c < n; ++c)
                M[r][c] -= f * M[col][c];
            b[r] -= f * b[col];
        }
    }
    for (int r = n - 1; r >= 0; --r) {
        double sum = b[r];
        for (int c = r + 1; c < n; ++c)
            sum -= M[r][c] * b[c];
        b[r] = sum / M[r][r];
    }
    return true;
}

double interpolate(const Spectrum& s, double xNm)
{
    const auto& w = s.wavelengthsNm;
    const auto it = std::lower_bound(w.cbegin(), w.cend(), xNm);
    if (it == w.cbegin())
        return s.counts.first();
    if (it == w.cend())
        return s.counts.last();
    const int hi = int(it - w.cbegin());
    const int lo = hi - 1;
    const double t = (xNm - w[lo]) / (w[hi] - w[lo]);
    return s.counts[lo] + t * (s.counts[hi] - s.counts[lo]);
}

} // namespace

QVector<double> savitzkyGolay(const QVector<double>& y, int window, int polyOrder)
{
    if (window < 5 || window > 25 || window % 2 == 0)
        return y;
    if (polyOrder < 2 || polyOrder > 3 || polyOrder >= window)
        return y;
    if (y.size() < window)
        return y;

    const int m = window / 2;
    const int terms = polyOrder + 1;

    // Normal equations for a least-squares polynomial fit over offsets
    // -m..m: S[k][l] = sum(i^(k+l)). The smoothing weights are the first row
    // of (S^-1 * A^T), i.e. w_i = sum_k a[k] * i^k with S * a = e0.
    QVector<QVector<double>> S(terms, QVector<double>(terms, 0.0));
    for (int k = 0; k < terms; ++k) {
        for (int l = 0; l < terms; ++l) {
            double sum = 0.0;
            for (int i = -m; i <= m; ++i)
                sum += std::pow(double(i), k + l);
            S[k][l] = sum;
        }
    }
    QVector<double> a(terms, 0.0);
    a[0] = 1.0;
    if (!solveLinearSystem(S, a))
        return y;

    QVector<double> weights(window);
    for (int i = -m; i <= m; ++i) {
        double w = 0.0;
        for (int k = 0; k < terms; ++k)
            w += a[k] * std::pow(double(i), k);
        weights[i + m] = w;
    }

    QVector<double> out(y);  // edges stay as the original samples
    for (int j = m; j < y.size() - m; ++j) {
        double v = 0.0;
        for (int i = -m; i <= m; ++i)
            v += weights[i + m] * y[j + i];
        out[j] = v;
    }
    return out;
}

QVector<Peak> findPeaks(const Spectrum& s, double minProminence, double minDistanceNm)
{
    if (!s.isValid())
        return {};
    const auto& c = s.counts;
    const int n = c.size();

    QVector<Peak> candidates;
    for (int i = 1; i < n - 1; ++i) {
        if (!(c[i] > c[i - 1] && c[i] >= c[i + 1]))
            continue;

        // Walk outward until higher ground (or the edge); the prominence base
        // is the higher of the two lowest points passed on the way.
        double leftMin = c[i];
        for (int j = i - 1; j >= 0; --j) {
            if (c[j] > c[i])
                break;
            leftMin = std::min(leftMin, c[j]);
        }
        double rightMin = c[i];
        for (int j = i + 1; j < n; ++j) {
            if (c[j] > c[i])
                break;
            rightMin = std::min(rightMin, c[j]);
        }
        const double prominence = c[i] - std::max(leftMin, rightMin);
        if (prominence >= minProminence)
            candidates.append({i, s.wavelengthsNm[i], c[i], prominence});
    }

    std::sort(candidates.begin(), candidates.end(),
              [](const Peak& a, const Peak& b) { return a.prominence > b.prominence; });

    // Greedy minimum-distance suppression: highest prominence claims its
    // neighborhood first.
    QVector<Peak> result;
    for (const Peak& p : candidates) {
        const bool tooClose = std::any_of(result.cbegin(), result.cend(),
            [&](const Peak& kept) {
                return std::abs(kept.wavelengthNm - p.wavelengthNm) < minDistanceNm;
            });
        if (!tooClose)
            result.append(p);
    }
    return result;
}

double integrate(const Spectrum& s, double fromNm, double toNm)
{
    if (!s.isValid())
        return 0.0;
    if (fromNm > toNm)
        std::swap(fromNm, toNm);

    const double a = std::max(fromNm, s.wavelengthsNm.first());
    const double b = std::min(toNm, s.wavelengthsNm.last());
    if (a >= b)
        return 0.0;

    // Trapezoids between knots inside (a, b), plus interpolated partial
    // segments at both boundaries.
    double integral = 0.0;
    double prevX = a;
    double prevY = interpolate(s, a);
    const auto& w = s.wavelengthsNm;
    const int n = w.size();
    for (int i = 0; i < n; ++i) {
        if (w[i] <= a)
            continue;
        if (w[i] >= b)
            break;
        integral += 0.5 * (prevY + s.counts[i]) * (w[i] - prevX);
        prevX = w[i];
        prevY = s.counts[i];
    }
    integral += 0.5 * (prevY + interpolate(s, b)) * (b - prevX);
    return integral;
}

QVector<double> transmittance(const Spectrum& sample, const Spectrum& dark,
                              const Spectrum& reference)
{
    const int n = sample.counts.size();
    if (n == 0 || dark.counts.size() != n || reference.counts.size() != n)
        return {};

    QVector<double> t(n);
    for (int i = 0; i < n; ++i) {
        const double denom = std::max(reference.counts[i] - dark.counts[i], 1e-9);
        t[i] = (sample.counts[i] - dark.counts[i]) / denom;
    }
    return t;
}

QVector<double> absorbance(const Spectrum& sample, const Spectrum& dark,
                           const Spectrum& reference)
{
    QVector<double> t = transmittance(sample, dark, reference);
    for (double& v : t) {
        v = std::clamp(v, 1e-6, 10.0);
        v = -std::log10(v);
    }
    return t;
}

} // namespace SpectroAnalysis
