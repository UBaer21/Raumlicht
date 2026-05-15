const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Raumlicht</title>

<style>
body{
  margin:0;
  font-family:Arial;
  background:#0b0f14;
  color:#e6edf3;
}

.header{
  text-align:center;
  padding:14px;
  font-size:18px;
  background:#111826;
}

.card{
  margin:12px;
  padding:14px;
  background:#111826;
  border-radius:12px;
}

.toggle{
  display:flex;
  gap:6px;
  align-items:stretch;
}

.toggle input{
  display:none;
}

.toggle label{
  flex:1;
  display: flex;
  align-items: center;
  justify-content: center;
  text-align:center;
  background:#1f2a37;
  padding:14px;
  border-radius:10px;
  font-size:clamp(14px,2.5vw,16px);
  opacity:1;
}

.btn-off{
  flex:0 0 auto !important;
  padding:0 10px !important;
  font-size:11px;
  border-radius:8px;
  display:flex;
  align-items:center;
  justify-content:center;
  opacity:0.5;
  background:#1f2a37;
  color:#94a3b8;
}

.toggle input:checked + .btn-off{
  opacity:1;
  background:#3a1a1a;
  color:#f87171;
}


.badge {
  display:inline-block;
  margin-top:6px;
  padding:3px 10px;
  border-radius:6px;
  font-size:11px;
  font-weight:700;
  letter-spacing:0.5px;
  background:#1a3a2a;
  color:#4ade80;
}
.badge.entering { background:#1a2a3a; color:#38bdf8; }
.badge.leaving  { background:#3a2a1a; color:#fb923c; }
.badge.inside   { background:#1a3a2a; color:#4ade80; }
.badge.outside  { background:#1f2a37; color:#94a3b8; }
.badge.stale    { background:#2a1a1a; color:#f87171; }
.badge.none     { background:#1f2a37; color:#94a3b8; }

.toggle input:checked + label{
  background:#38bdf8;
  color:#0b0f14;
}

input[type=range]{
  width:100%;
}

.stat-row{
  display:flex;
  gap:10px;
  margin:12px;
  flex-wrap:wrap;
}

.stat-card{
  flex:1 1 140px;
  background:#111826;
  border-radius:12px;
  padding:12px;
}

.stat-label{
  font-size:12px;
  opacity:0.6;
}

.stat-val{
  font-size:clamp(16px,3vw,22px);
}

/* range bar */
.barWrap{
  position:relative;
  margin-top:8px;
  height:8px;
  border-radius:10px;
  overflow:visible;
}
.bar-bg{
  position:absolute;
  inset:0;
  border-radius:10px;
  overflow:hidden;
  display:flex;
  background:#1a2433;
}
.seg-green{ height:100%; background:#00ff88; flex-shrink:0; }
.seg-grad { height:100%; background:linear-gradient(90deg,#00ff88,#00d4ff); flex-shrink:0; }
.seg-grey { height:100%; background:#aaa; opacity:0.25; flex-shrink:0; }
.seg-empty{ height:100%; flex:1; }

.barWrap input[type=range]{
  position:absolute;
  width:100%;
  top:50%;
  transform:translateY(-50%);
  margin:0;
  appearance:none;
  background:transparent;
  pointer-events:none;
  z-index:3;
  height:20px;
}
.barWrap input[type=range]::-webkit-slider-thumb{
  appearance:none;
  pointer-events:all;
  width:14px; height:14px;
  border-radius:50%;
  background:transparent;
  cursor:grab;
}
.barWrap input[type=range]:active::-webkit-slider-thumb{ cursor:grabbing; }
.barWrap input[type=range]::-moz-range-thumb{
  pointer-events:all;
  width:12px; height:12px;
  border-radius:50%;
  background:transparent;
  cursor:grab;
  box-sizing:border-box;
}
#r-in::-webkit-slider-thumb { border:2px solid #00cc66; }
#r-out::-webkit-slider-thumb{ border:2px solid #00aacc; }
#r-in::-moz-range-thumb     { border:2px solid #00cc66; }
#r-out::-moz-range-thumb    { border:2px solid #00aacc; }

canvas{
  width:100%;
  display:block;
  background:#0f172a;
  border-radius:10px;
}

#g {
  height: calc((100vh - 520px - 1vw) * 0.7);
}

#pg {
  height: calc((100vh - 520px - 1vw) * 0.3);
}
:root {
  --lux-thumb-bg: #0b0f14;
}

#luxSlider {
  -webkit-appearance: none;
  appearance: none;
  height: 8px;
  border-radius: 10px;
  outline: none;
  background: linear-gradient(90deg, #38bdf8 50%, #1a2433 50%);
}

#luxSlider::-webkit-slider-thumb {
  -webkit-appearance: none;
  width: 14px; height: 14px;
  border-radius: 50%;
  background: var(--lux-thumb-bg);
  cursor: grab;
  border: 2px solid #e6edf3;
}
#luxSlider::-moz-range-thumb {
  width: 12px; height: 12px;
  border-radius: 50%;
  background: var(--lux-thumb-bg);
  cursor: grab;
  border: 2px solid #e6edf3;
}

label{
  font-size:13px;
  opacity:0.7;
}
</style>
</head>

<body>

<div class="header">Raumlicht</div>

<div class="card">
  <label>Mode</label><br>
  <div class="toggle">
    <input type="radio" id="mD" name="mode" value="DUNKEL" onchange="setMode(this.value)">
    <label for="mD">DUNKEL</label>

    <input type="radio" id="mH" name="mode" value="HELL" onchange="setMode(this.value)">
    <label for="mH">MOTION</label>

    <input type="radio" id="mL" name="mode" value="LUX" onchange="setMode(this.value)">
    <label for="mL">LUX</label>

    <input type="radio" id="mOff" name="mode" value="OFF" onchange="setMode(this.value)">
    <label for="mOff" class="btn-off">OFF</label>
  </div>
</div>

<div class="card">
  <label>Set lux: <span id="t">50</span></label>
  <input type="range" id="luxSlider" min="0" max="1000" step="1" value="500"
         oninput="setLuxMapped(this.value)">
</div>

<div class="stat-row">
  <div class="stat-card">
    <div class="stat-label">Lux</div>
    <div class="stat-val" id="l">0</div>
  </div>
  <div class="stat-card">
    <div class="stat-label">PWM</div>
    <div class="stat-val" id="p">0</div>
  </div>
</div>

<div class="stat-row">
  <div class="stat-card">
    <div class="stat-label">Presence</div>
    <div class="stat-val" id="presence">—</div>
    <div id="motionbadge" class="badge none">NONE</div>
  </div>
  <div class="stat-card">
      <div style="display:flex;justify-content:space-between;align-items:center;">
        <div class="stat-label">Range (cm)</div>

        <button id="lockBtn" onclick="toggleLock()"
          style="font-size:11px;padding:2px 8px;border-radius:6px;border:none;cursor:pointer;
                background:#1f2a37;color:#94a3b8;">
          🔓 Enter
        </button>
      </div>
    <div class="stat-val" id="range">—</div>

    <div class="barWrap">
      <div class="bar-bg">
        <div class="seg-green" id="seg-green"></div>
        <div class="seg-grad"  id="seg-grad"></div>
        <div class="seg-grey"  id="seg-grey"></div>
        <div class="seg-empty"></div>
      </div>
      <input type="range" id="r-in"  min="0" max="599" value="50"  step="1">
      <input type="range" id="r-out" min="0" max="599" value="350" step="1">
    </div>
  </div>
</div>

<div class="card">
  <label>Lux</label>
  <canvas id="g"></canvas>
</div>

<div class="card">
  <label>PWM</label>
  <canvas id="pg"></canvas>
</div>

<script>

let hist=[], phist=[];

let MAX_RANGE = 400;
let rangeLocked = true; // locked by default after first setup

function updateLuxSliderBg(val){
  const pct = val / 1000 * 100;

  const mode =
     document.querySelector('input[name="mode"]:checked')?.value;
 
  const activeColor =
    mode === 'LUX' ? '#38bdf8' : '#4b5563';
 
  document.getElementById('luxSlider').style.background =
    `linear-gradient(90deg, ${activeColor} ${pct}%, #1a2433 ${pct}%)`;

  document.documentElement.style.setProperty('--lux-thumb-bg', activeColor);

}

function applyLock(){
  const locked = rangeLocked;
  document.getElementById('r-in').disabled  = locked;
  document.getElementById('r-out').disabled = locked;
  document.getElementById('lockBtn').textContent = locked ? '🔒' : 'slide';
  document.getElementById('lockBtn').style.color  = locked ? '#007171' : '#4ade80';
}

function toggleLock(){
  rangeLocked = !rangeLocked;
  localStorage.setItem('rangeLocked', rangeLocked);
  applyLock();
}

// restore lock state on load
const saved = localStorage.getItem('rangeLocked');
rangeLocked = saved === null ? true : saved === 'true';
applyLock();

function rangePercent(r){
  return Math.min(r, MAX_RANGE) / MAX_RANGE * 100;
}

function updateBar(rv){
  const inn = parseInt(document.getElementById('r-in').value);
  const out = parseInt(document.getElementById('r-out').value);

  // no valid range -> empty bar
  if(rv < 0){
    document.getElementById('seg-green').style.width = '0%';
    document.getElementById('seg-grad').style.width  = '0%';
    document.getElementById('seg-grey').style.width  = '0%';
    return;
  }

  const pRv  = rangePercent(rv);
  const pIn  = inn / MAX_RANGE * 100;
  const pOut = out / MAX_RANGE * 100;

  document.getElementById('seg-green').style.width =
    Math.max(0, Math.min(pRv, pIn)) + '%';

  document.getElementById('seg-grad').style.width =
    Math.max(0, (pRv > pIn ? Math.min(pRv, pOut) - pIn : 0)) + '%';

  document.getElementById('seg-grey').style.width =
    Math.max(0, (pRv > pOut ? pRv - pOut : 0)) + '%';
}


function saveConfig(){
  const inn = parseInt(document.getElementById('r-in').value);
  const out = parseInt(document.getElementById('r-out').value);
  MAX_RANGE = Math.min(out + 100, 699);
  document.getElementById('r-in').setAttribute('max', MAX_RANGE);
  document.getElementById('r-out').setAttribute('max', MAX_RANGE);

  fetch('/config/set', {
    method:'POST',
    headers:{'Content-Type':'application/json'},
    body: JSON.stringify({inRange: inn, outRange: out})
  });
}

document.getElementById('r-in').addEventListener('change', function(){
  if(+this.value >= +document.getElementById('r-out').value)
    document.getElementById('r-out').value = +this.value + 1;
  saveConfig();
});
document.getElementById('r-out').addEventListener('change', function(){
  if(+this.value <= +document.getElementById('r-in').value)
    document.getElementById('r-in').value = +this.value - 1;
  saveConfig();
});

/* MODE */


function setMode(m){
  fetch('/mode?m='+m);

  // ensure UI reflects new mode immediately
  document.querySelector(`input[name="mode"][value="${m}"]`).checked = true;

  updateLuxSliderBg(
    document.getElementById('luxSlider').value
  );
}




/* LUX MAPPING */
function setLuxMapped(v){
  let val;

  if(v<=500){
    val=(v/500)*5;
    val=Math.round(val/0.01)*0.01;
  } else {
    val=5+((v-500)/500)*(200-5);
    val=Math.round(val);
  }

  document.getElementById('t').innerText = val<5 ? val.toFixed(2) : val;

  fetch('/target?v='+val);
  updateLuxSliderBg(v);

  document.getElementById('mL').checked = true;
  setMode('LUX');
}

function resizeCanvas(c){
  const rect = c.getBoundingClientRect();
  const dpr = window.devicePixelRatio || 1;
  const w = Math.round(rect.width * dpr);
  const h = Math.round(rect.height * dpr);

  if(c.width !== w || c.height !== h){
    c.width = w;
    c.height = h;
    c.getContext('2d').setTransform(dpr,0,0,dpr,0,0);
  }
}

const ro = new ResizeObserver(entries=>{
  for(const e of entries){
    resizeCanvas(e.target);
  }
});

function draw(arr,id){
  let c=document.getElementById(id);
  resizeCanvas(c);
  let ctx=c.getContext('2d');

  let w=c.clientWidth;
  let h=c.clientHeight;

  ctx.clearRect(0,0,w,h);

  if(arr.length<2) return;

  let max=Math.max(...arr);
  let min=Math.max(Math.min(...arr),0.001);

  if(max===min) max=min+1;

  let logMax=Math.log10(max);
  let logMin=Math.log10(min);

  ctx.strokeStyle='#1e293b';
  ctx.lineWidth=1;
  ctx.fillStyle='#94a3b8';
  ctx.font='10px Arial';

  for(let i=0;i<=4;i++){
    let ratio=i/4;
    let y=i*h/4;

    ctx.beginPath();
    ctx.moveTo(0,y);
    ctx.lineTo(w,y);
    ctx.stroke();

    let value=Math.pow(10,logMin+(logMax-logMin)*(1-ratio));
    ctx.fillText(value<1?value.toFixed(3):value.toFixed(1),2,y>10?y-2:10);
  }

  for(let i=0;i<=6;i++){
    let x=i*w/6;
    ctx.beginPath();
    ctx.moveTo(x,0);
    ctx.lineTo(x,h);
    ctx.stroke();
  }

  ctx.strokeStyle='#38bdf8';
  ctx.lineWidth=2;
  ctx.beginPath();

  for(let i=0;i<arr.length;i++){
    let x=i*(w/arr.length);
    let v=Math.log10(Math.max(arr[i],0.001));
    let y=h-((v-logMin)/(logMax-logMin))*h;

    if(i===0) ctx.moveTo(x,y);
    else ctx.lineTo(x,y);
  }

  ctx.stroke();
}

function setBadge(state) {
    const badge = document.getElementById('motionbadge');
    const valid = ['entering','inside','leaving','outside','stale'];
    const s = valid.includes(state.toLowerCase()) ? state.toLowerCase() : 'none';
    badge.className = 'badge ' + s;
    badge.innerText = s.toUpperCase();
}

function luxToSliderValue(lux){
  if(lux <= 5){
    return (lux / 5) * 500;
  } else {
    return 500 + ((lux - 5) / (200 - 5)) * 500;
  }
}

async function upd(){
  try{
    let r=await fetch('/state');
    let j=await r.json();

    if(j.mode){
      const radio = document.querySelector(
        `input[name="mode"][value="${j.mode}"]`
      );

      if(radio){
        radio.checked = true;
      }
    }

    if(typeof j.target === 'number'){

      // update text
      document.getElementById('t').innerText =
        j.target < 5
          ? j.target.toFixed(2)
          : j.target.toFixed(0);

      // update slider position
      const sliderVal = luxToSliderValue(j.target);
      const slider = document.getElementById('luxSlider');
      if(slider && document.activeElement !== slider){
        slider.value = sliderVal;
        updateLuxSliderBg(sliderVal);
      }
    }


    const lux = Number(j.lux);
    document.getElementById('l').innerText =
      lux>=0 ? (lux < 3 ? lux.toFixed(2) : lux.toFixed(1)) : '…';

    document.getElementById('p').innerText=j.pwm;
    document.getElementById('presence').innerText=j.presence||'—';

    setBadge(j.motion || 'none');

    let rv=parseInt(j.range);
    document.getElementById('range').innerText=(rv>0)?rv:'—';
    updateBar(rv);

    hist.push(j.lux);
    phist.push(j.pwm);

    if(hist.length>10800) hist.shift();
    if(phist.length>18000) phist.shift();

    draw(hist,'g');
    draw(phist,'pg');

  }catch(e){}
}

ro.observe(document.getElementById('g'));
ro.observe(document.getElementById('pg'));


fetch('/config/get').then(r=>r.json()).then(cfg=>{
  const inEl = document.getElementById('r-in');
  const outEl = document.getElementById('r-out');
  inEl.value  = cfg.inRange;
  outEl.value = cfg.outRange;
  MAX_RANGE = Math.min(cfg.outRange + 100, 699);
  inEl.setAttribute('max', MAX_RANGE);
  outEl.setAttribute('max', MAX_RANGE);
  updateBar(parseInt(inEl.value));
});

setInterval(upd,1000);
upd();

</script>

</body>
</html>
)rawliteral";