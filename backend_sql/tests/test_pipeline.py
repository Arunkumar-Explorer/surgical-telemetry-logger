import sys, os
sys.path.insert(0, os.path.dirname(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "simulators"))
import database as dbmod
from bus_simulator import generate_frame, set_mode
from fastapi.testclient import TestClient
import app as appmod

def setup_module():
    dbmod.DB_PATH = os.path.join(os.path.dirname(__file__), "test.db")
    if os.path.exists(dbmod.DB_PATH):
        os.remove(dbmod.DB_PATH)
    dbmod.init_db()

def test_frames_have_protocols():
    frames, estop, *_ = generate_frame()
    assert set(frames.keys()) == {"UART","SPI","I2C","CAN"}
    for v in frames.values():
        assert isinstance(v, str)

def test_crc_corrupt_mode():
    set_mode("CAN_CRC_CORRUPT")
    frames, *_ = generate_frame()
    assert frames["CAN"].endswith("00")
    set_mode("NORMAL")

def test_estop_trigger():
    set_mode("FORCE_OVERLOAD_ESTOP")
    frames, estop, *_ = generate_frame()
    assert estop is True
    set_mode("NORMAL")

def test_api_metrics():
    c = TestClient(appmod.app)
    r = c.get("/api/metrics")
    assert r.status_code == 200

def test_frames_endpoint():
    c = TestClient(appmod.app)
    r = c.get("/api/frames")
    assert r.status_code == 200

def test_faults_endpoint():
    c = TestClient(appmod.app)
    r = c.get("/api/faults")
    assert r.status_code == 200

def test_inject_fault():
    c = TestClient(appmod.app)
    r = c.post("/api/inject-fault", json={"mode":"NORMAL"})
    assert r.status_code == 200

def test_replay():
    c = TestClient(appmod.app)
    r = c.get("/api/replay/1")
    assert r.status_code == 200

def test_sql_readonly():
    c = TestClient(appmod.app)
    r = c.post("/api/sql-query", json={"query":"DELETE FROM sessions"})
    assert r.status_code == 400
    r2 = c.post("/api/sql-query", json={"query":"SELECT 1"})
    assert r2.status_code == 200
