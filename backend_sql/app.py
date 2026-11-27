import os, sys, sqlite3, time, json, asyncio
from fastapi import FastAPI, WebSocket, Query
from fastapi.staticfiles import StaticFiles
from fastapi.responses import JSONResponse
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "backend_sql"))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "simulators"))
import database as dbmod
from bus_simulator import Simulator, set_mode, get_mode, generate_frame

dbmod.init_db()
app = FastAPI()
sim = Simulator(100)
latest = {"frames":{}, "estop":False, "kpi":{}}
def on_frame(frames, estop, joints, torque, imu, force, hb):
    latest["frames"]=frames
    latest["estop"]=estop
    latest["kpi"]={"throughput":100,"p50":hb,"p95":hb*2,"p99":hb*3,"integrity":99.0,"ring":0,"watchdog":"OK" if not estop else "ESTOP"}
sim.subscribe(on_frame)
sim.start()

@app.get("/api/metrics")
def metrics():
    return latest["kpi"]

@app.get("/api/frames")
def frames(protocol: str=None, limit: int=50, offset: int=0):
    c = dbmod.get_conn()
    q = "SELECT * FROM telemetry_frames"
    params=[]
    if protocol:
        q += " WHERE bus_protocol=?"
        params.append(protocol)
    q += " LIMIT ? OFFSET ?"
    params += [limit, offset]
    rows = [dict(r) for r in c.execute(q, params).fetchall()]
    c.close()
    return rows

@app.get("/api/faults")
def faults():
    c = dbmod.get_conn()
    rows = [dict(r) for r in c.execute("SELECT * FROM fault_events ORDER BY timestamp_us").fetchall()]
    c.close()
    return rows

@app.post("/api/inject-fault")
def inject(payload: dict):
    m = payload.get("mode","NORMAL")
    set_mode(m)
    c = dbmod.get_conn()
    c.execute("INSERT INTO fault_events(session_id,timestamp_us,fault_mode,detail) VALUES (?,?,?,?)",(1,int(time.time()*1e6),m,"injected"))
    c.commit(); c.close()
    return {"mode":m}

@app.get("/api/replay/{session_id}")
def replay(session_id: int):
    c = dbmod.get_conn()
    rows = [dict(r) for r in c.execute("SELECT * FROM telemetry_frames WHERE session_id=? ORDER BY timestamp_us",(session_id,)).fetchall()]
    c.close()
    return rows

@app.post("/api/sql-query")
def sql_query(payload: dict):
    q = payload.get("query","").strip()
    if not q.lower().startswith("select"):
        return JSONResponse({"error":"only SELECT allowed"}, status_code=400)
    c = dbmod.get_conn()
    try:
        rows = [dict(r) for r in c.execute(q).fetchall()]
    except Exception as e:
        c.close()
        return JSONResponse({"error":str(e)}, status_code=400)
    c.close()
    return rows

@app.websocket("/ws/telemetry")
async def ws_telemetry(ws: WebSocket):
    await ws.accept()
    while True:
        await ws.send_json({"frames":latest["frames"],"estop":latest["estop"],"kpi":latest["kpi"]})
        await asyncio.sleep(0.1)

frontend_dir = os.path.join(os.path.dirname(__file__), "..", "frontend")
if os.path.isdir(frontend_dir):
    app.mount("/", StaticFiles(directory=frontend_dir, html=True), name="frontend")
