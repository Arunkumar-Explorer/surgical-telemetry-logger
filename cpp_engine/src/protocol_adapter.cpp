#include "protocol_adapter.hpp"
#include <numeric>

bool UARTAdapter::parseFrame(const std::vector<uint8_t>& raw, Frame& out) {
    if (raw.size() < 4 || raw.front() != 0xAA || raw.back() != 0x55) return false;
    out.bytes = std::vector<uint8_t>(raw.begin() + 1, raw.end() - 1);
    return true;
}
bool UARTAdapter::validateChecksum(const Frame& f) {
    if (f.bytes.empty()) return false;
    uint8_t x = 0;
    for (size_t i = 0; i + 1 < f.bytes.size(); ++i) x ^= f.bytes[i];
    return x == f.bytes.back();
}

bool SPIAdapter::parseFrame(const std::vector<uint8_t>& raw, Frame& out) {
    if (raw.size() < 2) return false;
    out.bytes = raw;
    return true;
}
bool SPIAdapter::validateChecksum(const Frame& f) {
    uint8_t p = 0;
    for (auto b : f.bytes) { uint8_t x = b; x ^= x >> 4; x ^= x >> 2; x ^= x >> 1; p ^= (x & 1); }
    return p == 0;
}

bool I2CAdapter::parseFrame(const std::vector<uint8_t>& raw, Frame& out) {
    if (raw.size() < 2) return false;
    uint8_t addr = raw[0] & 0x7F;
    if (addr == 0) return false;
    out.bytes = raw;
    return true;
}
bool I2CAdapter::validateChecksum(const Frame& f) {
    // last byte is ACK/NACK flag; must be ACK (0)
    if (f.bytes.empty()) return false;
    return f.bytes.back() == 0x00;
}

static uint16_t crc16(const std::vector<uint8_t>& d) {
    uint16_t crc = 0xFFFF;
    for (auto b : d) {
        crc ^= (uint16_t)b << 8;
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
            else crc <<= 1;
        }
    }
    return crc;
}
bool CANAdapter::parseFrame(const std::vector<uint8_t>& raw, Frame& out) {
    if (raw.size() < 12) return false;
    // 2 bytes id, 1 byte dlc, 8 bytes payload, 2 bytes crc = 13 min? allow 11.. 
    out.bytes = raw;
    return true;
}
bool CANAdapter::validateChecksum(const Frame& f) {
    if (f.bytes.size() < 4) return false;
    size_t n = f.bytes.size() - 2;
    uint16_t c = (f.bytes[n] << 8) | f.bytes[n+1];
    std::vector<uint8_t> d(f.bytes.begin(), f.bytes.begin() + n);
    return crc16(d) == c;
}
