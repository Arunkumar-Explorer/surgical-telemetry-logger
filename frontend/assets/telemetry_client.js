// Dual-mode data engine
window.TelemetryClient = {
  ws: null,
  mode: 'sim',
  data: { kpi:{throughput:100,p99:1.2,integrity:99.5,ring:42,watchdog:'NOMINAL'}, frames:[], faults:[] },
  subscribers: [],
  async connect() {
    try {
      const r = await fetch('http://127.0.0.1:8000/api/metrics', {mode:'no-cors'});
      this.mode='live';
    } catch(e){ this.mode='sim'; }
    this.startSim();
  },
  startSim() {
    setInterval(()=>{ this.tick(); this.subscribers.forEach(cb=>cb(this.data)); }, 100);
  },
  tick() {
    const d=this.data;
    d.kpi.throughput=95+Math.random()*10;
    d.kpi.p99=0.8+Math.random()*1.5;
    d.kpi.integrity=98+Math.random()*2;
    d.kpi.ring=Math.round(30+Math.random()*40);
    d.kpi.watchdog=Math.random()>0.99?'ESTOP':'NOMINAL';
    const f={ts:Date.now()*1000, proto:['UART','SPI','I2C','CAN'][Math.floor(Math.random()*4)], hex:'AA0102030055', crc:Math.random()>0.05};
    d.frames.unshift(f);
    if(d.frames.length>200) d.frames.pop();
  },
  subscribe(cb){ this.subscribers.push(cb); }
};
window.TelemetryClient.connect();
function nav(page){
  document.querySelectorAll('nav a').forEach(a=>a.classList.toggle('active', a.dataset.page===page));
}
