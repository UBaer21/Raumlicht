#pragma once

const char PAGE[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">

<title>RAUMLICHT</title>

<style>
body {
    margin:0;
    font-family: Arial;
    background:#0b0f14;
    color:#e6edf3;
}

.header {
    text-align:center;
    padding:15px;
    font-size:20px;
    background:#111826;
}

.card {
    margin:15px;
    padding:15px;
    background:#111826;
    border-radius:12px;
    box-shadow:0 0 10px rgba(0,0,0,0.4);
}

/* -----------------------------
   TOGGLE GROUP
----------------------------- */
.toggle {
    display:flex;
    gap:5px;
}

.toggle input {
    display:none;
}

.toggle label {
    flex:1;
    text-align:center;
    background:#1f2a37;
    color:#e6edf3;
    padding:14px;
    border-radius:10px;
    font-size:16px;
    cursor:pointer;
    user-select:none;
}

.toggle input:checked + label {
    background:#38bdf8;
    color:#0b0f14;
    box-shadow:0 0 8px rgba(56,189,248,0.6);
}

.toggle label:active {
    background:#334155;
}

/* ----------------------------- */

input[type=range] {
    width:100%;
}

label {
    font-size:14px;
    opacity:0.8;
}

#g {
    height:35vh;
}

#pg {
    height:20vh;
}

canvas {
    width:100%;
    background:#0f172a;
    border-radius:10px;
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
        <label for="mH">HELL</label>

        <input type="radio" id="mL" name="mode" value="LUX" onchange="setMode(this.value)">
        <label for="mL">LUX</label>
    </div>
</div>

<div class="card">
    <label>Target Lux: <span id="t">50</span></label>
    <input type="range" min="0.1" max="200" step="0.1" value="50" oninput="setLux(this.value)">
</div>

<div class="card">
    <label>Live Lux</label>
    <div id="l">0</div> 
    <canvas id="g" width="1200" height="150"></canvas>
</div>

<div class="card">
    <label>PWM</label>
    <div id="p">0</div>
    <canvas id="pg" width="1200" height="100"></canvas>
</div>

<script>

let hist=[];
let phist=[];

function setMode(m){
    fetch('/mode?m='+m);
}

function setLux(v){
    document.getElementById('t').innerText=v;
    fetch('/target?v='+v);
    setMode('LUX');
}

function updateModeUI(mode){
    let el = document.querySelector(`input[name="mode"][value="${mode}"]`);
    if(el) el.checked = true;
}

async function upd(){
    let r=await fetch('/state');
    let j=await r.json();

    document.getElementById('t').innerText=j.target;
    document.getElementById('p').innerText=j.pwm;
    document.getElementById('l').innerText =
         j.lux < 3 ? j.lux.toFixed(2) : j.lux.toFixed(1);

    updateModeUI(j.mode);

    hist.push(j.lux);
    phist.push(j.pwm);

    if(hist.length>10800) hist.shift();
    if(phist.length>10800) phist.shift();

    draw(hist,"g");
    draw(phist,"pg");
}

function draw(arr,id){
    let c = document.getElementById(id);
    resizeCanvas(c);
    let ctx = c.getContext('2d');

    let w = c.width;
    let h = c.height;

    ctx.clearRect(0,0,w,h);

    ctx.fillStyle = "#0f172a";
    ctx.fillRect(0,0,w,h);

    if(arr.length < 2) return;

    let max = Math.max(...arr);
    let min = Math.min(...arr);

    if(max <= 0.0001) max = 0.0001;
    if(min <= 0.0001) min = 0.0001;

    let logMax = Math.log10(max);
    let logMin = Math.log10(min);

    ctx.strokeStyle = "#1e293b";
    ctx.lineWidth = 1;

    let rows = 4;
    let cols = 6;

    for(let i=0;i<=rows;i++){
        let y = i * h / rows;
        ctx.beginPath();
        ctx.moveTo(0,y);
        ctx.lineTo(w,y);
        ctx.stroke();
    }

    for(let i=0;i<=cols;i++){
        let x = i * w / cols;
        ctx.beginPath();
        ctx.moveTo(x,0);
        ctx.lineTo(x,h);
        ctx.stroke();
    }

    ctx.fillStyle = "#94a3b8";
    ctx.font = "10px Arial";

    for(let i=0;i<=rows;i++){
        let ratio = i / rows; 
        let value = Math.pow(10, logMin + (logMax - logMin) * (1 - ratio));
        let y = i * h / rows;
        ctx.fillText(
            value < 1 ? value.toFixed(3) : value.toFixed(1),
            2,
            y-2
        );
    }

    ctx.strokeStyle = "#38bdf8";
    ctx.lineWidth = 2;
    ctx.beginPath();

    for(let i=0;i<arr.length;i++){
        let x = i * (w / arr.length);

        let safe = Math.max(arr[i], 0.0001);
        let v = Math.log10(safe);
        let y = h - ((v - logMin) / (logMax - logMin)) * h;

        if(i==0) ctx.moveTo(x,y);
        else ctx.lineTo(x,y);
    }

    ctx.stroke();
}

function resizeCanvas(c){
    let rect = c.getBoundingClientRect();
    c.width = rect.width;
    c.height = rect.height;
}

setInterval(upd,1000);
upd();

</script>

</body>
</html>

)rawliteral";