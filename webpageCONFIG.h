const char CONF[] PROGMEM = R"CONF(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Raumlicht — Config</title>
<style>
/* ── base — matches main page exactly ── */
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:Arial,sans-serif;background:#0b0f14;color:#e6edf3;min-height:100vh}

.header{text-align:center;padding:14px;font-size:18px;background:#111826;margin-bottom:0}
.sub{text-align:center;font-size:11px;color:#64748b;background:#111826;padding:0 0 10px;letter-spacing:.06em}

/* status dot row */
.topbar{display:flex;align-items:center;gap:10px;padding:10px 14px;background:#111826;border-bottom:1px solid #1e293b}
.dot{width:8px;height:8px;border-radius:50%;background:#374151;flex-shrink:0;transition:background .4s}
.dot.on{background:#1D9E75}
.topbar-label{font-size:11px;color:#64748b;flex:1}
#ipLabel{font-size:10px;color:#64748b}

/* cards — same as main page */
.card{margin:12px;padding:14px;background:#111826;border-radius:12px}
.card-title{font-size:12px;color:#64748b;margin-bottom:12px;letter-spacing:.05em;text-transform:uppercase}

/* section toggle */
.sec-head{display:flex;align-items:center;gap:8px;cursor:pointer;user-select:none;margin-bottom:0}
.sec-head .card-title{margin-bottom:0;flex:1}
.chevron{font-size:13px;color:#64748b;transition:transform .2s;display:inline-block}
.sec-body{margin-top:12px}
.sec-body.hidden{display:none}

/* field rows */
.field-row{display:flex;align-items:center;gap:10px;margin-bottom:10px}
.field-row:last-child{margin-bottom:0}
.field-label{font-size:11px;color:#64748b;width:110px;flex-shrink:0;text-align:right}
.range-wrap{display:flex;align-items:center;gap:8px;flex:1}
.range-wrap input[type=range]{flex:1;accent-color:#38bdf8}
.range-val{font-size:12px;min-width:48px;text-align:right;color:#e6edf3;font-variant-numeric:tabular-nums}

/* LUT tabs */
.lut-tabs{display:flex;gap:0;border-bottom:1px solid #1e293b;margin:-14px -14px 14px}
.lut-tab{font-family:Arial,sans-serif;font-size:11px;padding:9px 16px;background:none;border:none;
  border-right:1px solid #1e293b;cursor:pointer;color:#64748b;letter-spacing:.04em;transition:background .15s}
.lut-tab:last-child{border-right:none}
.lut-tab.active{color:#e6edf3;background:#1f2a37}

.lut-pane{display:none}
.lut-pane.active{display:block}

/* LUT table */
.lut-wrap{overflow-x:auto}
table.lut{width:100%;border-collapse:collapse;font-size:11px}
table.lut th{text-align:center;padding:5px 6px;color:#64748b;font-weight:400;
  border-bottom:1px solid #1e293b;font-size:10px;letter-spacing:.04em}
table.lut td{padding:3px 4px;text-align:center}
table.lut td input{width:72px;font-family:Arial,sans-serif;font-size:11px;padding:4px 6px;
  text-align:center;background:#0b0f14;border:1px solid #1e293b;border-radius:6px;
  color:#e6edf3;outline:none;transition:border-color .15s}
table.lut td input:focus{border-color:#38bdf8}
.row-idx{color:#374151;font-size:10px;width:24px}

.lut-actions{display:flex;gap:6px;margin-top:10px;flex-wrap:wrap}
.btn-sm{font-family:Arial,sans-serif;font-size:10px;padding:5px 12px;
  border:1px solid #1e293b;border-radius:8px;background:#0b0f14;
  cursor:pointer;color:#94a3b8;transition:background .15s,color .15s;letter-spacing:.03em}
.btn-sm:hover{background:#1f2a37;color:#e6edf3}

/* curve canvas */
.curve-box{margin-top:12px;background:#0b0f14;border-radius:10px;padding:10px}
canvas.curve{width:100%;display:block}

/* auto sampler */
.auto-banner{background:#0b0f14;border-radius:10px;padding:12px 14px;margin-bottom:12px;
  display:flex;align-items:center;gap:14px}
.lux-big{font-size:28px;font-weight:500;min-width:90px;color:#38bdf8}
.lux-unit{font-size:10px;color:#64748b;margin-top:2px;letter-spacing:.05em}
.auto-controls{display:flex;flex-direction:column;gap:8px;flex:1}
.sample-row{display:flex;gap:8px;align-items:center;flex-wrap:wrap}
.sample-row label{font-size:10px;color:#64748b}
.sample-row input[type=number]{width:56px;font-family:Arial,sans-serif;font-size:11px;
  padding:4px 6px;background:#111826;border:1px solid #1e293b;border-radius:6px;
  color:#e6edf3;outline:none}
.progress-bar{height:3px;background:#1e293b;border-radius:2px;overflow:hidden}
.progress-fill{height:100%;width:0%;background:#1D9E75;transition:width .15s}

/* wifi */
.ssid-block{background:#0b0f14;border-radius:10px;padding:12px;margin-bottom:8px}
.ssid-block:last-child{margin-bottom:0}
.ssid-tag{font-size:9px;letter-spacing:.08em;text-transform:uppercase;color:#64748b;margin-bottom:8px}
.field-row input[type=text],
.field-row input[type=password]{flex:1;font-family:Arial,sans-serif;font-size:12px;
  padding:6px 8px;background:#0b0f14;border:1px solid #1e293b;border-radius:8px;
  color:#e6edf3;outline:none;transition:border-color .15s}
.field-row input[type=text]:focus,
.field-row input[type=password]:focus{border-color:#38bdf8}
.eye-btn{background:none;border:none;cursor:pointer;padding:2px 6px;color:#64748b;font-size:14px;line-height:1}
.ap-row{display:flex;align-items:center;gap:10px;padding:8px 0 0}
.ap-row label{font-size:11px;color:#64748b;flex:1}
input[type=checkbox]{accent-color:#38bdf8;width:14px;height:14px}

/* save bar */
.save-bar{display:flex;gap:8px;margin:12px}
.btn-save{flex:1;font-family:Arial,sans-serif;font-size:12px;padding:12px;
  border:1px solid #1e293b;border-radius:10px;background:#1f2a37;
  cursor:pointer;color:#94a3b8;transition:background .15s,color .15s}
.btn-save:hover{background:#263548;color:#e6edf3}
.btn-save.primary{background:#38bdf8;color:#0b0f14;border-color:#38bdf8;font-weight:600}
.btn-save.primary:hover{opacity:.85}

/* toast */
.toast{position:fixed;bottom:20px;left:50%;transform:translateX(-50%) translateY(60px);
  background:#e6edf3;color:#0b0f14;font-family:Arial,sans-serif;font-size:11px;
  padding:8px 18px;border-radius:10px;opacity:0;transition:all .3s;pointer-events:none;z-index:99}
.toast.show{opacity:1;transform:translateX(-50%) translateY(0)}

/* loading overlay */
.loading{display:flex;align-items:center;gap:8px;padding:20px 0;color:#64748b;font-size:12px;justify-content:center}
.spinner{width:14px;height:14px;border:2px solid #1e293b;border-top-color:#38bdf8;
  border-radius:50%;animation:spin .7s linear infinite}
@keyframes spin{to{transform:rotate(360deg)}}
</style>
</head>
<body>

<div class="header">Raumlicht</div>
<div class="sub">config</div>

<div class="topbar">
  <div class="dot" id="statusDot"></div>
  <span class="topbar-label" id="statusLabel">connecting…</span>
  <span id="ipLabel">—</span>
</div>

<!-- LUT TABLES -->
<div class="card">
  <div class="sec-head" onclick="toggleSec('lut')">
    <span class="card-title">lut tables</span>
    <span class="chevron" id="chev-lut">▾</span>
  </div>
  <div class="sec-body" id="sec-lut">
    <div class="lut-tabs">
      <button class="lut-tab active" onclick="switchTab('D')">D — dunkel</button>
      <button class="lut-tab"        onclick="switchTab('H')">H — hell</button>
      <button class="lut-tab"        onclick="switchTab('A')">auto — veml7700</button>
    </div>

    <!-- D pane -->
    <div class="lut-pane active" id="pane-D">
      <div class="lut-wrap">
        <table class="lut">
          <thead><tr><th>#</th><th>lux (in)</th><th>pwm (out)</th></tr></thead>
          <tbody id="lutBody-D"></tbody>
        </table>
      </div>
      <div class="lut-actions">
        <button class="btn-sm" onclick="addRow('D')">+ add</button>
        <button class="btn-sm" onclick="removeRow('D')">− remove</button>
        <button class="btn-sm" onclick="drawCurve('D')">↺ preview</button>
      </div>
      <div class="curve-box"><canvas class="curve" id="curve-D" height="130"></canvas></div>
    </div>

    <!-- H pane -->
    <div class="lut-pane" id="pane-H">
      <div class="lut-wrap">
        <table class="lut">
          <thead><tr><th>#</th><th>lux (in)</th><th>pwm (out)</th></tr></thead>
          <tbody id="lutBody-H"></tbody>
        </table>
      </div>
      <div class="lut-actions">
        <button class="btn-sm" onclick="addRow('H')">+ add</button>
        <button class="btn-sm" onclick="removeRow('H')">− remove</button>
        <button class="btn-sm" onclick="drawCurve('H')">↺ preview</button>
      </div>
      <div class="curve-box"><canvas class="curve" id="curve-H" height="130"></canvas></div>
    </div>

    <!-- A pane -->
    <div class="lut-pane" id="pane-A">
      <div class="auto-banner">
        <div>
          <div class="lux-big" id="liveLux">—</div>
          <div class="lux-unit">lux · veml7700</div>
        </div>
        <div class="auto-controls">
          <div class="sample-row">
            <label>samples</label>
            <input type="number" id="sampleCount" value="8" min="2" max="20" step="1">
            <label>interval (s)</label>
            <input type="number" id="sampleInterval" value="3" min="1" max="60" step="1">
            <button class="btn-sm" id="btnSample" onclick="startSampling()">▶ sample</button>
          </div>
          <div class="progress-bar"><div class="progress-fill" id="sampleProgress"></div></div>
        </div>
      </div>
      <div class="lut-wrap">
        <table class="lut">
          <thead><tr><th>#</th><th>lux (measured)</th><th>pwm (target)</th></tr></thead>
          <tbody id="lutBody-A"></tbody>
        </table>
      </div>
      <div class="lut-actions">
        <button class="btn-sm" onclick="addRow('A')">+ add</button>
        <button class="btn-sm" onclick="removeRow('A')">− remove</button>
        <button class="btn-sm" onclick="pushAutoToD()">→ copy to D</button>
        <button class="btn-sm" onclick="pushAutoToH()">→ copy to H</button>
        <button class="btn-sm" onclick="drawCurve('A')">↺ preview</button>
      </div>
      <div class="curve-box"><canvas class="curve" id="curve-A" height="130"></canvas></div>
    </div>
  </div>
</div>

<!-- CONTROL PARAMETERS -->
<div class="card">
  <div class="sec-head" onclick="toggleSec('cfg')">
    <span class="card-title">control parameters</span>
    <span class="chevron" id="chev-cfg">▾</span>
  </div>
  <div class="sec-body" id="sec-cfg">
    <div class="field-row">
      <span class="field-label">inRange</span>
      <div class="range-wrap">
        <input type="range" min="0" max="600" step="1" id="inRange"
               oninput="rv('rv-inRange', this.value)">
        <span class="range-val" id="rv-inRange">50</span>
      </div>
    </div>
    <div class="field-row">
      <span class="field-label">outRange</span>
      <div class="range-wrap">
        <input type="range" min="0" max="700" step="1" id="outRange"
               oninput="rv('rv-outRange', this.value)">
        <span class="range-val" id="rv-outRange">350</span>
      </div>
    </div>
    <div class="field-row">
      <span class="field-label">zeroRange</span>
      <div class="range-wrap">
        <input type="range" min="0" max="100" step="1" id="zeroRange"
               oninput="rv('rv-zeroRange', this.value)">
        <span class="range-val" id="rv-zeroRange">0</span>
      </div>
    </div>
    <div class="field-row">
      <span class="field-label">velocityConst</span>
      <div class="range-wrap">
        <input type="range" min="0" max="200" step="0.5" id="velocityConst"
               oninput="rv('rv-velocityConst', parseFloat(this.value).toFixed(1))">
        <span class="range-val" id="rv-velocityConst">68.0</span>
      </div>
    </div>
    <div class="field-row">
      <span class="field-label">targetLux</span>
      <div class="range-wrap">
        <input type="range" min="0" max="250" step="0.5" id="targetLux"
               oninput="rv('rv-targetLux', parseFloat(this.value).toFixed(1))">
        <span class="range-val" id="rv-targetLux">50.0</span>
      </div>
    </div>
  </div>
</div>

<!-- WIFI -->
<div class="card">
  <div class="sec-head" onclick="toggleSec('wifi')">
    <span class="card-title">wifi credentials</span>
    <span class="chevron" id="chev-wifi">▾</span>
  </div>
  <div class="sec-body hidden" id="sec-wifi">
    <div class="ssid-block">
      <div class="ssid-tag">network · AP fallback activates if unreachable</div>
      <div class="field-row">
        <span class="field-label">ssid</span>
        <input type="text" id="wifiSsid" placeholder="(default)" autocomplete="off">
      </div>
      <div class="field-row">
        <span class="field-label">password</span>
        <input type="password" id="wifiPass" placeholder="leave blank to keep" autocomplete="new-password">
        <button class="eye-btn" onclick="togglePw('wifiPass',this)">👁</button>
      </div>
    </div>
  </div>
  </div>
</div>

<div class="save-bar">
  <button class="btn-save" onclick="loadFromDevice()">↓ reload from device</button>
  <button class="btn-save primary" onclick="saveToDevice()">↑ save to device</button>
</div>

<div class="toast" id="toast"></div>

<script>
// ── constants ──────────────────────────────────────────────
const COLORS = {D:'#38bdf8', H:'#4ade80', A:'#fb923c'};

const DEFAULTS = {
  D:{lux:[0,0.01,0.03,0.05,0.1,0.25,0.5,1,5,20,50,500],
     pwm:[10,12,15,20,30,45,65,90,130,170,200,500]},
  H:{lux:[0,0.5,1,2,5,10,25,50,100,200,350,500],
     pwm:[100,120,140,180,250,350,550,800,1100,1500,1800,2000]},
  A:{lux:[],pwm:[]}
};

let activeTab = 'D';
let samplingTimer = null;

// ── section toggle ─────────────────────────────────────────
function toggleSec(id) {
  const body = document.getElementById('sec-' + id);
  const chev  = document.getElementById('chev-' + id);
  const hidden = body.classList.toggle('hidden');
  chev.textContent = hidden ? '▸' : '▾';
}

// ── display value next to slider ──────────────────────────
function rv(id, val) { document.getElementById(id).textContent = val; }

// ── tab switching ─────────────────────────────────────────
function switchTab(t) {
  activeTab = t;
  document.querySelectorAll('.lut-tab').forEach((b, i) => {
    b.classList.toggle('active', ['D','H','A'][i] === t);
  });
  document.querySelectorAll('.lut-pane').forEach(p => p.classList.remove('active'));
  document.getElementById('pane-' + t).classList.add('active');
  drawCurve(t);
}

// ── build LUT table from arrays ───────────────────────────
function buildTable(t, lux, pwm) {
  const tb = document.getElementById('lutBody-' + t);
  tb.innerHTML = '';
  for (let i = 0; i < lux.length; i++) {
    const tr = document.createElement('tr');
    tr.innerHTML =
      `<td class="row-idx">${i}</td>
       <td><input type="number" step="any"   class="lux-in-${t}"  value="${lux[i]}" oninput="drawCurve('${t}')"></td>
       <td><input type="number" step="1" min="0" max="4095" class="pwm-out-${t}" value="${pwm[i]}" oninput="drawCurve('${t}')"></td>`;
    tb.appendChild(tr);
  }
  drawCurve(t);
}

// ── read current table values ─────────────────────────────
function getLUT(t) {
  const lux = [...document.querySelectorAll('.lux-in-'  + t)].map(i => parseFloat(i.value) || 0);
  const pwm = [...document.querySelectorAll('.pwm-out-' + t)].map(i => parseInt(i.value)   || 0);
  return {lux, pwm};
}

function addRow(t) {
  const {lux, pwm} = getLUT(t);
  lux.push(lux[lux.length-1] + 1);
  pwm.push(pwm[pwm.length-1]);
  buildTable(t, lux, pwm);
}

function removeRow(t) {
  const {lux, pwm} = getLUT(t);
  if (lux.length > 2) { lux.pop(); pwm.pop(); buildTable(t, lux, pwm); }
}

function pushAutoToD() {
  const {lux, pwm} = getLUT('A');
  if (lux.length > 1) { buildTable('D', lux, pwm); showToast('copied auto → D'); }
}
function pushAutoToH() {
  const {lux, pwm} = getLUT('A');
  if (lux.length > 1) { buildTable('H', lux, pwm); showToast('copied auto → H'); }
}

// ── curve canvas ──────────────────────────────────────────
function drawCurve(t) {
  const c = document.getElementById('curve-' + t);
  if (!c) return;
  // sync canvas pixel size to display size
  const dpr = window.devicePixelRatio || 1;
  const dispW = c.clientWidth;
  if (c.width !== Math.round(dispW * dpr)) {
    c.width  = Math.round(dispW * dpr);
    c.height = Math.round(130  * dpr);
    c.getContext('2d').scale(dpr, dpr);
  }
  const ctx = c.getContext('2d');
  const W = dispW, H = 130;
  ctx.clearRect(0, 0, W, H);

  const {lux, pwm} = getLUT(t);
  if (lux.length < 2) return;

  const maxLux = Math.max(...lux) || 1;
  const maxPwm = Math.max(...pwm) || 1;
  const pad = {l:38, r:12, t:8, b:26};
  const w = W - pad.l - pad.r, h = H - pad.t - pad.b;

  // grid
  ctx.strokeStyle = 'rgba(255,255,255,0.06)'; ctx.lineWidth = 0.5;
  ctx.fillStyle   = '#475569'; ctx.font = '9px Arial'; ctx.textAlign = 'right';
  for (let i = 0; i <= 4; i++) {
    const y = pad.t + h * (1 - i/4);
    ctx.beginPath(); ctx.moveTo(pad.l, y); ctx.lineTo(pad.l+w, y); ctx.stroke();
    ctx.fillText(Math.round(maxPwm * i/4), pad.l - 4, y + 3);
  }
  ctx.fillStyle = '#475569'; ctx.textAlign = 'left';
  ctx.fillText('pwm', 2, pad.t + 7);
  ctx.fillText('lux →', pad.l, H - 4);

  const xp = v => pad.l + w * (v / maxLux);
  const yp = v => pad.t + h * (1 - v / maxPwm);

  // line
  ctx.strokeStyle = COLORS[t]; ctx.lineWidth = 1.5;
  ctx.beginPath(); ctx.moveTo(xp(lux[0]), yp(pwm[0]));
  for (let i = 1; i < lux.length; i++) ctx.lineTo(xp(lux[i]), yp(pwm[i]));
  ctx.stroke();

  // dots
  ctx.fillStyle = COLORS[t];
  for (let i = 0; i < lux.length; i++) {
    ctx.beginPath(); ctx.arc(xp(lux[i]), yp(pwm[i]), 3, 0, Math.PI*2); ctx.fill();
  }
}

// ── lux polling (auto tab only) ───────────────────────────
async function readLux() {
  try {
    const r = await fetch('/lux/read');
    if (!r.ok) return null;
    const d = await r.json();
    return d.lux ?? null;
  } catch { return null; }
}

async function pollLux() {
  const v = await readLux();
  const el = document.getElementById('liveLux');
  el.textContent = v !== null
    ? (v < 10 ? v.toFixed(2) : v < 100 ? v.toFixed(1) : Math.round(v))
    : '—';
}

// ── sampler ───────────────────────────────────────────────
async function startSampling() {
  if (samplingTimer) return;
  const n  = parseInt(document.getElementById('sampleCount').value);
  const iv = parseFloat(document.getElementById('sampleInterval').value) * 1000;
  let sampledPoints = [];
  const btn  = document.getElementById('btnSample');
  const fill = document.getElementById('sampleProgress');
  btn.textContent = '◼ stop'; btn.onclick = stopSampling;
  let done = 0;

  const tick = async () => {
    const v = await readLux();
    document.getElementById('liveLux').textContent =
      v !== null ? (v < 10 ? v.toFixed(2) : v < 100 ? v.toFixed(1) : Math.round(v)) : '—';
    if (v !== null) {
      sampledPoints.push({lux: v, pwm: 0});
      done++;
      fill.style.width = (done / n * 100) + '%';
      const sorted = [...sampledPoints].sort((a,b) => a.lux - b.lux);
      buildTable('A', sorted.map(p => p.lux), sorted.map(p => p.pwm));
    }
    if (done >= n) stopSampling();
  };
  await tick();
  if (done < n) samplingTimer = setInterval(tick, iv);
}

function stopSampling() {
  clearInterval(samplingTimer); samplingTimer = null;
  const btn = document.getElementById('btnSample');
  btn.textContent = '▶ sample'; btn.onclick = startSampling;
}

// ── password toggle ───────────────────────────────────────
function togglePw(id, btn) {
  const el = document.getElementById(id);
  el.type = el.type === 'text' ? 'password' : 'text';
  btn.textContent = el.type === 'text' ? '🙈' : '👁';
}

// ── toast ─────────────────────────────────────────────────
function showToast(msg) {
  const t = document.getElementById('toast');
  t.textContent = msg; t.classList.add('show');
  setTimeout(() => t.classList.remove('show'), 2200);
}

// ── apply device payload to UI ────────────────────────────
function applyPayload(d) {
  if (d.lut_d) buildTable('D', d.lut_d.lux, d.lut_d.pwm);
  if (d.lut_h) buildTable('H', d.lut_h.lux, d.lut_h.pwm);

  const setSlider = (id, v, display) => {
    if (v == null) return;
    const el = document.getElementById(id);
    if (el) el.value = v;
    rv('rv-' + id, display !== undefined ? display : v);
  };

  setSlider('inRange',       d.inRange,       d.inRange);
  setSlider('outRange',      d.outRange,      d.outRange);
  setSlider('zeroRange',     d.zeroRange,     d.zeroRange);
  setSlider('velocityConst', d.velocityConst, parseFloat(d.velocityConst ?? 68).toFixed(1));
  setSlider('targetLux',     d.targetLux,     parseFloat(d.targetLux    ?? 50).toFixed(1));

  if (d.wifi) {
      const ssidEl = document.getElementById('wifiSsid');
      ssidEl.value       = d.wifi.source === 'saved' ? d.wifi.ssid : '';
      ssidEl.placeholder = d.wifi.source === 'saved' ? d.wifi.ssid : '(default)';
      document.getElementById('wifiPass').value = '';
  }
}

// ── build POST payload ────────────────────────────────────
function getPayload() {
  return {
    lut_d: getLUT('D'),
    lut_h: getLUT('H'),
    inRange:       parseInt(document.getElementById('inRange').value),
    outRange:      parseInt(document.getElementById('outRange').value),
    zeroRange:     parseInt(document.getElementById('zeroRange').value),
    velocityConst: parseFloat(document.getElementById('velocityConst').value),
    targetLux:     parseFloat(document.getElementById('targetLux').value),
    wifi: {
        ssid: document.getElementById('wifiSsid').value.trim(),
        pass: document.getElementById('wifiPass').value
    }
  };
}

// ── load from device ──────────────────────────────────────
async function loadFromDevice() {
  const dot   = document.getElementById('statusDot');
  const label = document.getElementById('statusLabel');
  const ip    = document.getElementById('ipLabel');
  label.textContent = 'loading…';
  try {
    const r = await fetch('/config/get');
    if (!r.ok) throw new Error('http ' + r.status);
    const d = await r.json();
    applyPayload(d);
    dot.classList.add('on');
    label.textContent = 'connected';
    ip.textContent    = location.host || 'device';
    showToast('loaded from device');
  } catch (e) {
    dot.classList.remove('on');
    label.textContent = 'load failed';
    showToast('load failed — check connection');
  }
}

// ── save to device ────────────────────────────────────────
async function saveToDevice() {
  try {
    const r = await fetch('/config/set', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify(getPayload())
    });
    if (!r.ok) throw new Error('http ' + r.status);
    showToast('saved ✓');
  } catch {
    showToast('save failed — check connection');
  }
}

// ── init ──────────────────────────────────────────────────
// Start with hardcoded defaults so tables are never empty
buildTable('D', DEFAULTS.D.lux, DEFAULTS.D.pwm);
buildTable('H', DEFAULTS.H.lux, DEFAULTS.H.pwm);

// Auto-fetch from device immediately on page open
loadFromDevice();

// Poll lux only when auto tab is active
setInterval(() => { if (activeTab === 'A') pollLux(); }, 2000);
</script>
</body>
</html>
)CONF";