#include "GeigerMath.h"

#include <algorithm>

namespace GeigerMath {

RollingWindow::RollingWindow(qint64 windowMs)
    : m_windowMs(windowMs)
{
}

void RollingWindow::add(qint64 timestampMs, double cps)
{
    m_entries.append({timestampMs, cps});
    const qint64 cutoff = timestampMs - m_windowMs;
    int firstKept = 0;
    while (firstKept < m_entries.size() && m_entries[firstKept].first < cutoff)
        ++firstKept;
    if (firstKept > 0)
        m_entries.remove(0, firstKept);
}

void RollingWindow::clear()
{
    m_entries.clear();
}

double RollingWindow::meanCps() const
{
    if (m_entries.isEmpty())
        return 0.0;
    double sum = 0.0;
    for (const auto& entry : m_entries)
        sum += entry.second;
    return sum / m_entries.size();
}

double RollingWindow::maxCps() const
{
    double maxValue = 0.0;
    for (const auto& entry : m_entries)
        maxValue = std::max(maxValue, entry.second);
    return maxValue;
}

double doseMicroSvPerHour(double cpm, double tubeFactor)
{
    return cpm * tubeFactor;
}

} // namespace GeigerMath
