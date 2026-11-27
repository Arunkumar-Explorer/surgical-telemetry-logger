#pragma once
#include <cstdint>
#include <vector>
#include <chrono>

struct TelemetryFrame {
    std::vector<float> jointVel; // 6
    float gripperForce;
};

class SafetyWatchdog {
public:
    bool check(const TelemetryFrame& f) const; // true = safe
    static constexpr float VEL_LIMIT = 2.0f;
    static constexpr float FORCE_LIMIT = 50.0f;
};
inline bool SafetyWatchdog::check(const TelemetryFrame& f) const {
    for (float v : f.jointVel) if (v > VEL_LIMIT || v < -VEL_LIMIT) return false;
    if (f.gripperForce > FORCE_LIMIT || f.gripperForce < 0) return false;
    return true;
}
