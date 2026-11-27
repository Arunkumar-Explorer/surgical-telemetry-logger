#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct Frame {
    std::vector<uint8_t> bytes;
};

class ProtocolAdapter {
public:
    virtual ~ProtocolAdapter() = default;
    virtual bool parseFrame(const std::vector<uint8_t>& raw, Frame& out) = 0;
    virtual bool validateChecksum(const Frame& f) = 0;
    virtual std::string protocolName() const = 0;
};

class UARTAdapter : public ProtocolAdapter {
public:
    bool parseFrame(const std::vector<uint8_t>& raw, Frame& out) override;
    bool validateChecksum(const Frame& f) override;
    std::string protocolName() const override { return "UART"; }
};

class SPIAdapter : public ProtocolAdapter {
public:
    bool parseFrame(const std::vector<uint8_t>& raw, Frame& out) override;
    bool validateChecksum(const Frame& f) override;
    std::string protocolName() const override { return "SPI"; }
};

class I2CAdapter : public ProtocolAdapter {
public:
    bool parseFrame(const std::vector<uint8_t>& raw, Frame& out) override;
    bool validateChecksum(const Frame& f) override;
    std::string protocolName() const override { return "I2C"; }
};

class CANAdapter : public ProtocolAdapter {
public:
    bool parseFrame(const std::vector<uint8_t>& raw, Frame& out) override;
    bool validateChecksum(const Frame& f) override;
    std::string protocolName() const override { return "CAN"; }
};
