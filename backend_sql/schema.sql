CREATE TABLE IF NOT EXISTS sessions (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL,
  started_at_us INTEGER NOT NULL,
  ended_at_us INTEGER
);
CREATE TABLE IF NOT EXISTS devices (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  session_id INTEGER NOT NULL,
  name TEXT NOT NULL,
  protocol TEXT NOT NULL,
  FOREIGN KEY(session_id) REFERENCES sessions(id)
);
CREATE TABLE IF NOT EXISTS telemetry_frames (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  session_id INTEGER NOT NULL,
  device_id INTEGER,
  bus_protocol TEXT NOT NULL,
  timestamp_us INTEGER NOT NULL,
  raw_hex TEXT NOT NULL,
  decoded TEXT,
  crc_ok INTEGER NOT NULL DEFAULT 1,
  FOREIGN KEY(session_id) REFERENCES sessions(id)
);
CREATE TABLE IF NOT EXISTS protocol_errors (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  session_id INTEGER,
  bus_protocol TEXT,
  timestamp_us INTEGER,
  error_type TEXT,
  detail TEXT
);
CREATE TABLE IF NOT EXISTS fault_events (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  session_id INTEGER,
  timestamp_us INTEGER,
  fault_mode TEXT,
  detail TEXT
);
CREATE INDEX IF NOT EXISTS idx_frames_session_proto_ts ON telemetry_frames(session_id, bus_protocol, timestamp_us);
CREATE INDEX IF NOT EXISTS idx_errors_session_proto_ts ON protocol_errors(session_id, bus_protocol, timestamp_us);
CREATE INDEX IF NOT EXISTS idx_faults_session_ts ON fault_events(session_id, timestamp_us);
