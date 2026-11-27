import threading, time, random, struct

FAULT_MODES = ["NORMAL","CAN_CRC_CORRUPT","I2C_NACK_STORM","UART_FRAME_DRIFT","LATENCY_SPIKE_20MS","FORCE_OVERLOAD_ESTOP"]
_state = {"mode":"NORMAL"}
_lock = threading.Lock()

def set_mode(m):
    with _lock:
        if m in FAULT_MODES:
            _state["mode"]=m

def get_mode():
    with _lock:
        return _state["mode"]

def frame_uart(joints):
    payload = bytes([0xAA] + [int((j+3.14)*10) & 0xFF for j in joints] + [0x00, 0x55])
    return payload.hex().upper()

def frame_spi(torque):
    b = struct.pack(">f", torque)
    return b.hex().upper()

def frame_i2c(imu):
    return "".join(f"{int((x+180)*10)&0xFF:02X}" for x in imu)

def frame_can(force, hb):
    b = struct.pack(">fI", force, hb)
    return b.hex().upper()

def generate_frame():
    joints = [random.uniform(-3.14,3.14) for _ in range(6)]
    torque = random.uniform(0,5)
    imu = [random.uniform(-90,90) for _ in range(3)]
    force = random.uniform(0,60)
    hb = random.randint(100,5000)
    mode = get_mode()
    if mode == "FORCE_OVERLOAD_ESTOP":
        force = 80.0
    if mode == "LATENCY_SPIKE_20MS":
        time.sleep(0.02)
    frames = {
        "UART": frame_uart(joints),
        "SPI": frame_spi(torque),
        "I2C": frame_i2c(imu),
        "CAN": frame_can(force, hb),
    }
    # fault injections
    if mode == "CAN_CRC_CORRUPT":
        frames["CAN"] = frames["CAN"][:-2] + "00"
    if mode == "I2C_NACK_STORM":
        frames["I2C"] = frames["I2C"][:-2] + "01"
    if mode == "UART_FRAME_DRIFT":
        frames["UART"] = frames["UART"][2:]
    estop = (mode=="FORCE_OVERLOAD_ESTOP") or force>50
    return frames, estop, joints, torque, imu, force, hb

class Simulator:
    def __init__(self, hz=100):
        self.hz = hz
        self.running = False
        self.thread = None
        self.subscribers = []
    def subscribe(self, cb):
        self.subscribers.append(cb)
    def _loop(self):
        period = 1.0/self.hz
        while self.running:
            t0 = time.time()
            frames, estop, joints, torque, imu, force, hb = generate_frame()
            for cb in list(self.subscribers):
                try:
                    cb(frames, estop, joints, torque, imu, force, hb)
                except Exception:
                    pass
            elapsed = time.time()-t0
            if elapsed < period:
                time.sleep(period-elapsed)
    def start(self):
        self.running = True
        self.thread = threading.Thread(target=self._loop, daemon=True)
        self.thread.start()
    def stop(self):
        self.running = False
        if self.thread:
            self.thread.join(timeout=2)
