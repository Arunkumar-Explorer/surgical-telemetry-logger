#include "ring_buffer.hpp"
#include "protocol_adapter.hpp"
#include "safety_watchdog.hpp"
#include <cassert>
#include <iostream>
#include <vector>

int main() {
    // SPSC wrap-around/overflow
    RingBuffer<int,4> rb;
    assert(rb.push(1) && rb.push(2) && rb.push(3));
    assert(!rb.push(4)); // full (capacity 3 usable)
    int x;
    assert(rb.pop(x) && x==1);
    assert(rb.push(4));
    assert(rb.pop(x)&&x==2);
    assert(rb.pop(x)&&x==3);
    assert(rb.pop(x)&&x==4);
    assert(rb.empty());

    // UART
    UARTAdapter ua;
    std::vector<uint8_t> raw={0xAA,0x01,0x02,0x03,0x00,0x55}; // checksum 01^02^03=00
    Frame f; assert(ua.parseFrame(raw,f)); assert(ua.validateChecksum(f));
    raw.back()=0x54; // corrupt delimiter? test checksum corrupt
    // corrupt checksum
    std::vector<uint8_t> raw2={0xAA,0x01,0x02,0x03,0xFF,0x55};
    Frame f2; ua.parseFrame(raw2,f2); assert(!ua.validateChecksum(f2));

    // SPI parity
    SPIAdapter sa;
    std::vector<uint8_t> sp={0x00}; // even parity ok
    Frame fs; sa.parseFrame(sp,fs); assert(sa.validateChecksum(fs));

    // I2C
    I2CAdapter ia;
    std::vector<uint8_t> ic={0x50, 0x00}; // addr + ACK
    Frame fi; ia.parseFrame(ic,fi); assert(ia.validateChecksum(fi));
    std::vector<uint8_t> ic2={0x50, 0x01};
    Frame fi2; ia.parseFrame(ic2,fi2); assert(!ia.validateChecksum(fi2));

    // CAN CRC
    CANAdapter ca;
    std::vector<uint8_t> payload(8,0x11);
    std::vector<uint8_t> can={0x01,0x02, 0x08};
    can.insert(can.end(), payload.begin(), payload.end());
    // compute crc
    std::vector<uint8_t> d(can.begin(), can.end());
    // append crc placeholder then fix
    uint16_t crc=0xFFFF;
    for (auto b: d){ crc ^= (uint16_t)b<<8; for(int i=0;i<8;++i){ if(crc&0x8000) crc=(crc<<1)^0x1021; else crc<<=1;} }
    can.push_back((crc>>8)&0xFF);
    can.push_back(crc&0xFF);
    Frame fc; assert(ca.parseFrame(can,fc)); assert(ca.validateChecksum(fc));
    // corrupt
    can[can.size()-1]^=0xFF;
    Frame fc2; ca.parseFrame(can,fc2); assert(!ca.validateChecksum(fc2));

    // ESTOP
    SafetyWatchdog wd;
    TelemetryFrame t; t.jointVel={0,0,0,0,0,3.0f}; t.gripperForce=10; assert(!wd.check(t));
    TelemetryFrame t2; t2.jointVel={0,0,0,0,0,0}; t2.gripperForce=60; assert(!wd.check(t2));
    TelemetryFrame t3; t3.jointVel={0,0,0,0,0,0}; t3.gripperForce=10; assert(wd.check(t3));

    std::cout << "ALL TESTS PASSED\n";
    return 0;
}
