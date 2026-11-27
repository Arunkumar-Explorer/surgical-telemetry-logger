import sqlite3, os, time, json

DB_PATH = os.path.join(os.path.dirname(__file__), "telemetry.db")
SCHEMA = os.path.join(os.path.dirname(__file__), "schema.sql")

def get_conn():
    c = sqlite3.connect(DB_PATH)
    c.row_factory = sqlite3.Row
    return c

def init_db():
    c = get_conn()
    with open(SCHEMA) as f:
        c.executescript(f.read())
    # seed if empty
    if c.execute("SELECT COUNT(*) FROM sessions").fetchone()[0] == 0:
        now = int(time.time()*1e6)
        cur = c.execute("INSERT INTO sessions(name,started_at_us,ended_at_us) VALUES (?,?,?)", ("historical_session_1", now-1000000, now))
        sid = cur.lastrowid
        c.execute("INSERT INTO devices(session_id,name,protocol) VALUES (?,?,?)", (sid,"arm_uart","UART"))
        c.execute("INSERT INTO telemetry_frames(session_id,bus_protocol,timestamp_us,raw_hex,decoded,crc_ok) VALUES (?,?,?,?,?,?)",
                  (sid,"UART",now-500000,"AA0102030055","{}",1))
        c.execute("INSERT INTO protocol_errors(session_id,bus_protocol,timestamp_us,error_type,detail) VALUES (?,?,?,?,?)",
                  (sid,"CAN",now-400000,"CRC_MISMATCH","corrupt"))
        c.execute("INSERT INTO fault_events(session_id,timestamp_us,fault_mode,detail) VALUES (?,?,?,?)",
                  (sid,now-300000,"FORCE_OVERLOAD_ESTOP","triggered"))
        c.commit()
    c.close()

if __name__ == "__main__":
    init_db()
    print("db ready")
