#include "ring_buffer.hpp"
#include "protocol_adapter.hpp"
#include "safety_watchdog.hpp"
#include <thread>
#include <fstream>
#include <iostream>
#include <chrono>
#include <atomic>

int main() {
    RingBuffer<TelemetryFrame, 1024> rb;
    std::atomic<bool> running{true};
    std::ofstream bin("telemetry.bin", std::ios::binary);
    std::thread reader([&]{
        while (running.load()) {
            TelemetryFrame f;
            f.jointVel = {0.1f,0.2f,0.3f,0.4f,0.5f,0.6f};
            f.gripperForce = 10.0f;
            rb.push(f);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    std::thread worker([&]{
        SafetyWatchdog wd;
        while (running.load()) {
            TelemetryFrame f;
            if (rb.pop(f)) {
                if (!wd.check(f)) {
                    std::cerr << "{\"alert\":\"ESTOP\"}\n";
                    running.store(false);
                    break;
                }
                bin.write(reinterpret_cast<const char*>(&f.gripperForce), sizeof(float));
                std::cout << "{\"jv\":[";
                for (size_t i=0;i<f.jointVel.size();++i){ if(i) std::cout<<","; std::cout<<f.jointVel[i]; }
                std::cout << "],\"gf\":" << f.gripperForce << "}\n";
            }
        }
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    running.store(false);
    reader.join();
    worker.join();
    return 0;
}
