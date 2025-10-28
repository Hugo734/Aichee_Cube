#pragma once
#include <pgmspace.h>

static const char AETHER_html[] PROGMEM = R"====AETHER====(
<!doctype html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>AETHER</title>
  <style>
    body{font-family:system-ui,Arial,sans-serif;margin:10px;line-height:1.4;background:#f5f5f5}
    h1{font-size:1.4rem;margin-bottom:0.25rem;color:#333}
    h2{font-size:1.1rem;margin-bottom:8px;color:#555}
    #status{font-weight:700;padding:3px 6px;border-radius:4px}
    #status.connected{background:#d4edda;color:#155724}
    #status.disconnected{background:#f8d7da;color:#721c24}
    .card{padding:12px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);max-width:800px;margin:0 auto 12px;background:white}
    .info{margin-top:6px;color:#666;font-size:0.85rem}
    hr{border:none;border-top:1px solid #e0e0e0;margin:10px 0}
    .btn{padding:8px 16px;background:#4CAF50;color:white;border:none;border-radius:4px;cursor:pointer;font-size:13px;margin:3px;transition:background 0.3s}
    .btn:hover{background:#45a049}
    .btn-danger{background:#f44336}
    .btn-danger:hover{background:#da190b}
    .btn-info{background:#2196F3}
    .btn-info:hover{background:#0b7dda}
    .btn-group{display:flex;gap:8px;flex-wrap:wrap;margin-top:10px;justify-content:center}
    .stats{background:#f9f9f9;padding:8px;border-radius:4px;margin-top:8px;display:flex;flex-wrap:wrap;justify-content:center}
    .stat-item{margin:0 12px 4px}
    .stat-label{font-size:0.8rem;color:#666}
    .stat-value{font-weight:bold;color:#333;font-size:0.9rem}
    .mode-indicator{display:inline-block;padding:2px 8px;border-radius:3px;font-size:0.85rem;margin-left:10px}
    .mode-manual{background:#ffeb3b;color:#333}
    .mode-auto{background:#4caf50;color:white}
    .logging-status{display:inline-block;padding:2px 8px;border-radius:3px;font-size:0.85rem;margin-left:10px}
    .logging-active{background:#4caf50;color:white}
    .logging-paused{background:#ff9800;color:white}
    .gauges-container{display:grid;grid-template-columns:repeat(auto-fit,minmax(120px,1fr));gap:15px;justify-items:center;margin:12px 0}
    .gauge-wrapper{text-align:center;width:100%;max-width:140px}
    .gauge-canvas{border:1px solid #ddd;background:white;width:100%;max-width:120px;height:120px;display:block;margin:0 auto}
    .gauge-label{font-size:0.85rem;color:#555;margin-top:3px}
    .gauge-value{font-size:1rem;font-weight:bold;color:#333;margin-top:2px}
    canvas#co2Chart{border:1px solid #ddd;background:white;width:100%;max-width:100%;height:180px;display:block;margin-top:8px}
    @media (max-width: 640px) {
      body{margin:5px}
      .card{padding:10px;margin-bottom:10px}
      h1{font-size:1.2rem}
      h2{font-size:1rem}
      .btn{padding:6px 12px;font-size:12px}
      .gauge-canvas{max-width:100px;height:100px}
      canvas#co2Chart{height:150px}
    }
    @media (min-width: 1024px) {
      .gauges-container{grid-template-columns:repeat(4,1fr);max-width:600px;margin:12px auto}
    }
  </style>
</head>
<body>
  <div class="card">
    <h1>AETHER <span id="modeIndicator" class="mode-indicator"></span></h1>
    <p style="margin:5px 0">Status: <span id="status" class="disconnected">Disconnected</span>
       <span id="loggingStatus" class="logging-status"></span></p>
    
    <div class="stats" id="statsPanel">
      <div class="stat-item">
        <span class="stat-label">Records:</span>
        <span class="stat-value" id="recordCount">0</span>
      </div>
      <div class="stat-item">
        <span class="stat-label">Avg CO₂:</span>
        <span class="stat-value" id="avgCO2">—</span>
      </div>
      <div class="stat-item">
        <span class="stat-label">Avg Temp:</span>
        <span class="stat-value" id="avgTemp">—</span>
      </div>
      <div class="stat-item">
        <span class="stat-label">Session:</span>
        <span class="stat-value" id="sessionTime">00:00:00</span>
      </div>
    </div>
    
    <div class="btn-group">
      <a href="/download" target="_blank" rel="noopener" id="dlDirect" class="btn">Download CSV</a>  
      <button class="btn btn-info" onclick="startLogging()">Start</button>
      <button class="btn btn-danger" onclick="clearData()">Clear</button>
    </div>
  </div>

  <div class="card">
    <h2>Live Readings</h2>
    <div class="gauges-container">
      <div class="gauge-wrapper">
        <canvas id="co2Gauge" class="gauge-canvas"></canvas>
        <div class="gauge-label">CO₂</div>
        <div class="gauge-value" id="co2GaugeValue">— ppm</div>
      </div>
      <div class="gauge-wrapper">
        <canvas id="tempGauge" class="gauge-canvas"></canvas>
        <div class="gauge-label">Temperature</div>
        <div class="gauge-value" id="tempGaugeValue">— °C</div>
      </div>
      <div class="gauge-wrapper">
        <canvas id="humGauge" class="gauge-canvas"></canvas>
        <div class="gauge-label">Humidity</div>
        <div class="gauge-value" id="humGaugeValue">— %</div>
      </div>
      <div class="gauge-wrapper">
        <canvas id="lpmGauge" class="gauge-canvas"></canvas>
        <div class="gauge-label">Flow Rate</div>
        <div class="gauge-value" id="lpmGaugeValue">— LPM</div>
      </div>
    </div>
  </div>
  
  <div class="card">
    <h2>CO₂ History (60s)</h2>
    <canvas id="co2Chart"></canvas>
  </div>
   
<script>
  (function(){
    const MAX_POINTS = 2500;
    const STATS_POINTS = 2000;
    const hist = { co2: [], temp: [], hum: [], pwm: [] };
    const MAX_RECORDS = 2500;
    const dataRecords = [];

    function pushBounded(arr, v, cap) {
      arr.push(v);
      if (arr.length > cap) arr.shift();
    }
    function addDataRecord(entry) {
      dataRecords.push(entry);
      if (dataRecords.length > MAX_RECORDS) dataRecords.shift();
    }
    function formatNumber(value, digits) {
      return Number.isFinite(value) ? value.toFixed(digits) : '';
    }
    function generateCSV() {
      if (dataRecords.length === 0) return '';
      const lines = ['Captured At,CO2 (ppm),Temperature (C),Humidity (%),PWM'];
      for (const rec of dataRecords) {
        const capturedIso = new Date(rec.capturedAt).toISOString();
        const co2 = formatNumber(rec.co2, 0);
        const temp = formatNumber(rec.temp, 1);
        const hum = formatNumber(rec.hum, 1);
        const pwm = Number.isFinite(rec.pwm) ? rec.pwm : '';
        lines.push([capturedIso, co2, temp, hum, pwm].join(','));      
      }
      return lines.join('\n');
    }
    function triggerCSVDownload(csvText) {
      const blob = new Blob([csvText], { type: 'text/csv' });
      const url = URL.createObjectURL(blob);
      const link = document.createElement('a');
      link.href = url;
      const stamp = new Date().toISOString().replace(/[:.]/g, '-');
      link.download = `AETHER_data_${stamp}.csv`;
      document.body.appendChild(link);
      link.click();
      document.body.removeChild(link);
      URL.revokeObjectURL(url);
    }
    function statsOf(arr, window = STATS_POINTS) {
      const n = Math.min(arr.length, window);
      if (n === 0) return { avg: NaN, min: NaN, max: NaN, n: 0 };
      let sum = 0, mn = +Infinity, mx = -Infinity;
      for (let i = arr.length - n; i < arr.length; i++) {
        const v = arr[i];
        if (!Number.isFinite(v)) continue;
        sum += v;
        if (v < mn) mn = v;
        if (v > mx) mx = v;
      }
      return { avg: sum / n, min: mn, max: mx, n };
    }

    const statusEl = document.getElementById('status');
    const modeEl = document.getElementById('modeIndicator');
    const loggingStatusEl = document.getElementById('loggingStatus');
    const sessionTimeEl = document.getElementById('sessionTime');
    const recordCountEl = document.getElementById('recordCount');
    const avgCO2El = document.getElementById('avgCO2');
    const avgTempEl = document.getElementById('avgTemp');

    class Gauge {
      constructor(canvasId, minVal, maxVal, unit, color) {
        this.canvas = document.getElementById(canvasId);
        this.ctx = this.canvas.getContext('2d');
        this.minVal = minVal;
        this.maxVal = maxVal;
        this.unit = unit;
        this.color = color || '#4CAF50';
        this.value = 0;
        this.canvas.width = 120;
        this.canvas.height = 120;
        this.centerX = 60;
        this.centerY = 70;
        this.radius = 40;
      }
      draw(value) {
        this.value = value;
        const ctx = this.ctx;
        ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
        const range = this.maxVal - this.minVal;
        let normalizedValue = 0;
        if (range > 0) normalizedValue = Math.max(0, Math.min(1, (value - this.minVal) / range));
        const startAngle = (135 * Math.PI) / 180;
        const endAngle   = (405 * Math.PI) / 180;
        const angleRange = endAngle - startAngle;
        const currentAngle = startAngle + (angleRange * normalizedValue);

        ctx.strokeStyle = '#e0e0e0';
        ctx.lineWidth = 6;
        ctx.lineCap = 'round';
        ctx.beginPath();
        ctx.arc(this.centerX, this.centerY, this.radius, startAngle, endAngle, false);
        ctx.stroke();

        if (normalizedValue > 0) {
          ctx.strokeStyle = this.color;
          ctx.lineWidth = 6;
          ctx.beginPath();
          ctx.arc(this.centerX, this.centerY, this.radius, startAngle, currentAngle, false);
          ctx.stroke();
        }

        ctx.strokeStyle = '#666';
        ctx.lineWidth = 1;
        for (let i = 0; i <= 10; i++) {
          const tickAngle = startAngle + (angleRange * i / 10);
          const x1 = this.centerX + Math.cos(tickAngle) * (this.radius - 8);
          const y1 = this.centerY + Math.sin(tickAngle) * (this.radius - 8);
          const x2 = this.centerX + Math.cos(tickAngle) * (this.radius - 4);
          const y2 = this.centerY + Math.sin(tickAngle) * (this.radius - 4);
          ctx.beginPath(); ctx.moveTo(x1, y1); ctx.lineTo(x2, y2); ctx.stroke();
        }

        ctx.strokeStyle = '#333';
        ctx.lineWidth = 2;
        ctx.lineCap = 'round';
        const needleX = this.centerX + Math.cos(currentAngle) * (this.radius - 12);
        const needleY = this.centerY + Math.sin(currentAngle) * (this.radius - 12);
        ctx.beginPath(); ctx.moveTo(this.centerX, this.centerY); ctx.lineTo(needleX, needleY); ctx.stroke();

        ctx.fillStyle = '#333';
        ctx.beginPath(); ctx.arc(this.centerX, this.centerY, 3, 0, 2 * Math.PI); ctx.fill();

        ctx.fillStyle = '#666';
        ctx.font = '9px sans-serif';
        ctx.textAlign = 'center';
        const minX = this.centerX + Math.cos(startAngle) * (this.radius + 12);
        const minY = this.centerY + Math.sin(startAngle) * (this.radius + 12) + 3;
        ctx.fillText(this.minVal.toString(), minX, minY);
        const maxX = this.centerX + Math.cos(endAngle) * (this.radius + 12);
        const maxY = this.centerY + Math.sin(endAngle) * (this.radius + 12) + 3;
        ctx.fillText(this.maxVal.toString(), maxX, maxY);
      }
    }

    const co2Gauge  = new Gauge('co2Gauge',  0, 2000, 'ppm', '#FF6B6B');
    const tempGauge = new Gauge('tempGauge', 0,   50, '°C',  '#4ECDC4');
    const humGauge  = new Gauge('humGauge',  0,  100, '%',   '#45B7D1');
    const lpmGauge  = new Gauge('lpmGauge',  0,   12, 'LPM', '#FFA07A');

    co2Gauge.draw(0); tempGauge.draw(0); humGauge.draw(0); lpmGauge.draw(0);

    const canvas = document.getElementById('co2Chart');
    const ctx = canvas.getContext('2d');
    const maxDataPoints = 1000;
    const co2Data = [];
    const chartPadding = 30;
    const minCO2 = 0;
    const maxCO2 = 900;

    let sessionStartTime = Date.now();

    function updateSessionTime(uptime) {
      if (uptime !== undefined) {
        const hours = Math.floor(uptime / 3600);
        const minutes = Math.floor((uptime % 3600) / 60);
        const seconds = uptime % 60;
        sessionTimeEl.textContent =
          String(hours).padStart(2, '0') + ':' +
          String(minutes).padStart(2, '0') + ':' +
          String(seconds).padStart(2, '0');
      }
    }
    function resizeCanvas() {
      const rect = canvas.getBoundingClientRect();
      canvas.width = rect.width;
      canvas.height = rect.height;
    }
    function drawChart() {
      resizeCanvas();
      const width = canvas.width, height = canvas.height;
      ctx.clearRect(0, 0, width, height);

      ctx.strokeStyle = '#ccc'; ctx.lineWidth = 1;
      ctx.beginPath();
      ctx.moveTo(chartPadding, chartPadding);
      ctx.lineTo(chartPadding, height - chartPadding);
      ctx.lineTo(width - chartPadding, height - chartPadding);
      ctx.stroke();

      ctx.fillStyle = '#666'; ctx.font = '10px sans-serif';
      ctx.textAlign = 'right'; ctx.textBaseline = 'middle';
      const step = 150;
      for (let value = minCO2; value <= maxCO2; value += step) {
        const normalized = (value - minCO2) / (maxCO2 - minCO2);
        const y = height - chartPadding - (height - 2 * chartPadding) * normalized;
        ctx.fillText(value.toFixed(0), chartPadding - 5, y);
        if (value !== minCO2) {
          ctx.strokeStyle = '#f0f0f0';
          ctx.beginPath(); ctx.moveTo(chartPadding, y); ctx.lineTo(width - chartPadding, y); ctx.stroke();
        }
      }

      if (co2Data.length > 1) {
        ctx.strokeStyle = '#4CAF50'; ctx.lineWidth = 2;
        ctx.beginPath();
        const chartWidth = width - 2 * chartPadding;
        const chartHeight = height - 2 * chartPadding;
        for (let i = 0; i < co2Data.length; i++) {
          const x = chartPadding + (chartWidth * i / (maxDataPoints - 1));
          const normalizedValue = (co2Data[i] - minCO2) / (maxCO2 - minCO2);
          const y = chartPadding + chartHeight * (1 - normalizedValue);
          if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
        }
        ctx.stroke();

        ctx.fillStyle = '#4CAF50';
        const lastIdx = co2Data.length - 1;
        const x = chartPadding + (chartWidth * lastIdx / (maxDataPoints - 1));
        const normalizedValue = (co2Data[lastIdx] - minCO2) / (maxCO2 - minCO2);
        const y = chartPadding + chartHeight * (1 - normalizedValue);
        ctx.beginPath(); ctx.arc(x, y, 3, 0, 2 * Math.PI); ctx.fill();
      }

      if (co2Data.length === 0) {
        ctx.fillStyle = '#999'; ctx.font = '12px sans-serif';
        ctx.textAlign = 'center'; ctx.textBaseline = 'middle';
        ctx.fillText('Waiting for data...', width / 2, height / 2);
      }
    }
    drawChart();

    const wsUrl = 'ws://' + location.hostname + ':81/';
    let ws, reconnectTimer, reconnectAttempts = 0;
    const maxReconnectAttempts = 10, reconnectDelay = 3000;

    function updateStatsFromClient() {
      recordCountEl.textContent = `${dataRecords.length}/${MAX_RECORDS}`;

      const sCO2  = statsOf(hist.co2);
      const sTemp = statsOf(hist.temp);

      avgCO2El.textContent  = Number.isFinite(sCO2.avg)  ? sCO2.avg.toFixed(0) + ' ppm' : '—';
      avgTempEl.textContent = Number.isFinite(sTemp.avg) ? sTemp.avg.toFixed(1) + '°C'   : '—';

      const elapsedSec = Math.floor((Date.now() - sessionStartTime) / 1000);
      updateSessionTime(elapsedSec);

      if (ws && ws.readyState === WebSocket.OPEN) {
        loggingStatusEl.textContent = 'Recording';
        loggingStatusEl.className = 'logging-status logging-active';
      } else {
        loggingStatusEl.textContent = 'Paused';
        loggingStatusEl.className = 'logging-status logging-paused';
      }
    }

    function connect() {
      if (reconnectAttempts >= maxReconnectAttempts) {
        console.log('Max reconnection attempts reached');
        statusEl.textContent = 'Connection failed';
        statusEl.className = 'disconnected';
        return;
      }
      console.log('Connecting to WebSocket:', wsUrl, 'Attempt:', reconnectAttempts + 1);
      try {
        ws = new WebSocket(wsUrl);

        ws.addEventListener('open', () => {
          console.log('WebSocket connected');
          statusEl.textContent = 'Connected';
          statusEl.className = 'connected';
          reconnectAttempts = 0;
          if (reconnectTimer) clearTimeout(reconnectTimer);
          updateStatsFromClient();
        });

        ws.addEventListener('message', (evt) => {
          try {
            const d = JSON.parse(evt.data);
            const elapsedField = Number(d.t);
            const record = {
              capturedAt: Date.now(),
              elapsed: Number.isFinite(elapsedField) ? Math.round(elapsedField) : null,
              co2: null,
              temp: null,
              hum: null,
              pwm: null
            };

            if (d.co2 !== undefined) {
              const co2Value = parseFloat(d.co2);
              if (Number.isFinite(co2Value)) {
                co2Data.push(co2Value);
                if (co2Data.length > maxDataPoints) co2Data.shift();
                drawChart();
                co2Gauge.draw(co2Value);
                document.getElementById('co2GaugeValue').textContent = co2Value.toFixed(0) + ' ppm';
                pushBounded(hist.co2, co2Value, MAX_POINTS);
                record.co2 = co2Value;
              }
            }

            if (d.temp !== undefined) {
              const tempValue = parseFloat(d.temp);
              if (Number.isFinite(tempValue)) {
                tempGauge.draw(tempValue);
                document.getElementById('tempGaugeValue').textContent = tempValue.toFixed(1) + ' °C';
                pushBounded(hist.temp, tempValue, MAX_POINTS);
                record.temp = tempValue;
              }
            }

            if (d.hum !== undefined) {
              const humValue = parseFloat(d.hum);
              if (Number.isFinite(humValue)) {
                humGauge.draw(humValue);
                document.getElementById('humGaugeValue').textContent = humValue.toFixed(1) + ' %';
                pushBounded(hist.hum, humValue, MAX_POINTS);
                record.hum = humValue;
              }
            }

            if (d.lpm !== undefined) {
              const lpmValue = parseFloat(d.lpm);
              if (Number.isFinite(lpmValue)) {
                lpmGauge.draw(lpmValue);
                document.getElementById('lpmGaugeValue').textContent = lpmValue.toFixed(2) + ' LPM';
                pushBounded(hist.pwm, lpmValue, MAX_POINTS);
                record.pwm = lpmValue;  // Guardamos como pwm para mantener compatibilidad con CSV
              }
            }

            if (d.mode !== undefined) {
              modeEl.textContent = d.mode;
              modeEl.className = 'mode-indicator mode-' + d.mode.toLowerCase();
            }

            if (record.co2 !== null || record.temp !== null || record.hum !== null || record.pwm !== null) {
              addDataRecord(record);
            }

            updateStatsFromClient();

          } catch (e) {
            console.warn('Error parsing message:', e, evt.data);
          }
        });

        ws.addEventListener('close', () => {
          console.log('WebSocket disconnected');
          statusEl.textContent = 'Disconnected';
          statusEl.className = 'disconnected';
          reconnectAttempts++;
          reconnectTimer = setTimeout(connect, reconnectDelay);
        });

        ws.addEventListener('error', (err) => {
          console.error('WebSocket error:', err);
          ws.close();
        });

      } catch (e) {
        console.error('Failed to create WebSocket:', e);
        reconnectAttempts++;
        reconnectTimer = setTimeout(connect, reconnectDelay);
      }
    }

    window.downloadCSV = function() {
      if (/(iPhone|iPad|iPod|Android)/i.test(navigator.userAgent)) {
      const a = document.getElementById('dlDirect');
      if (a) {
        a.click();
        return;
      }

      window.open('/download', '_blank', 'noopener');
      return;
    }
      fetch('/download')
        .then((response) => {
          if (!response.ok) throw new Error('download_failed');
          return response.text();
        })
        .then((csv) => {
          if (!csv.trim()) throw new Error('empty');
          triggerCSVDownload(csv);
        })
        .catch(() => {
          const csv = generateCSV();
          if (!csv) {
            alert('No data recorded yet.');
            return;
          }
          triggerCSVDownload(csv);
        });
    };

    window.clearData = function() {
      if (confirm('Clear all data and reset timer?')) {
        fetch('/clear', { method: 'POST' })
          .then(r => r.text())
          .then(result => {
            alert(result);
            hist.co2.length = hist.temp.length = hist.hum.length = hist.pwm.length = 0;
            co2Data.length = 0;
            dataRecords.length = 0;
            sessionStartTime = Date.now();
            drawChart();
            updateStatsFromClient();
          })
          .catch(error => console.error('Error:', error));
      }
    };

    window.startLogging = function() {
      fetch('/startlog', { method: 'POST' })
        .then(r => r.text())
        .then(result => {
          console.log(result);
          updateStatsFromClient();
        })
        .catch(error => console.error('Error:', error));
    };

    connect();
    window.addEventListener('resize', drawChart);
    setInterval(updateStatsFromClient, 3000);
  })();
</script>

</body>
</html>
)====AETHER====";
