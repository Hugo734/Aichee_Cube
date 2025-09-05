(function(){
  const statusEl = document.getElementById('status');
  const co2El = document.getElementById('co2');
  const tempEl = document.getElementById('temp');
  const humEl = document.getElementById('hum');
  const pwmEl = document.getElementById('pwm');
  
  // Stats elements
  const recordCountEl = document.getElementById('recordCount');
  const avgCO2El = document.getElementById('avgCO2');
  const avgTempEl = document.getElementById('avgTemp');
  
  // Simple Chart Implementation
  const canvas = document.getElementById('co2Chart');
  const ctx = canvas.getContext('2d');
  const maxDataPoints = 60;
  const co2Data = [];
  
  // Chart settings
  const chartPadding = 40;
  const minCO2 = 400;
  const maxCO2 = 2000;
  
  function resizeCanvas() {
    const rect = canvas.getBoundingClientRect();
    canvas.width = rect.width;
    canvas.height = rect.height;
  }
  
  function drawChart() {
    resizeCanvas();
    const width = canvas.width;
    const height = canvas.height;
    
    // Clear canvas
    ctx.clearRect(0, 0, width, height);
    
    // Draw axes
    ctx.strokeStyle = '#ccc';
    ctx.lineWidth = 1;
    ctx.beginPath();
    // Y axis
    ctx.moveTo(chartPadding, chartPadding);
    ctx.lineTo(chartPadding, height - chartPadding);
    // X axis
    ctx.lineTo(width - chartPadding, height - chartPadding);
    ctx.stroke();
    
    // Draw Y axis labels
    ctx.fillStyle = '#666';
    ctx.font = '11px sans-serif';
    ctx.textAlign = 'right';
    ctx.textBaseline = 'middle';
    
    const ySteps = 5;
    for (let i = 0; i <= ySteps; i++) {
      const y = chartPadding + (height - 2 * chartPadding) * i / ySteps;
      const value = maxCO2 - (maxCO2 - minCO2) * i / ySteps;
      ctx.fillText(value.toFixed(0), chartPadding - 5, y);
      
      // Grid lines
      if (i > 0 && i < ySteps) {
        ctx.strokeStyle = '#f0f0f0';
        ctx.beginPath();
        ctx.moveTo(chartPadding, y);
        ctx.lineTo(width - chartPadding, y);
        ctx.stroke();
      }
    }
    
    // Draw data
    if (co2Data.length > 1) {
      ctx.strokeStyle = '#4CAF50';
      ctx.lineWidth = 2;
      ctx.beginPath();
      
      const chartWidth = width - 2 * chartPadding;
      const chartHeight = height - 2 * chartPadding;
      
      for (let i = 0; i < co2Data.length; i++) {
        const x = chartPadding + (chartWidth * i / (maxDataPoints - 1));
        const normalizedValue = (co2Data[i] - minCO2) / (maxCO2 - minCO2);
        const y = chartPadding + chartHeight * (1 - normalizedValue);
        
        if (i === 0) {
          ctx.moveTo(x, y);
        } else {
          ctx.lineTo(x, y);
        }
      }
      ctx.stroke();
      
      // Draw dots for last 5 points
      ctx.fillStyle = '#4CAF50';
      const startIdx = Math.max(0, co2Data.length - 5);
      for (let i = startIdx; i < co2Data.length; i++) {
        const x = chartPadding + (chartWidth * i / (maxDataPoints - 1));
        const normalizedValue = (co2Data[i] - minCO2) / (maxCO2 - minCO2);
        const y = chartPadding + chartHeight * (1 - normalizedValue);
        
        ctx.beginPath();
        ctx.arc(x, y, 3, 0, 2 * Math.PI);
        ctx.fill();
      }
    }
    
    // Draw "No data" message if empty
    if (co2Data.length === 0) {
      ctx.fillStyle = '#999';
      ctx.font = '14px sans-serif';
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      ctx.fillText('Waiting for data...', width / 2, height / 2);
    }
  }
  
  // Initial draw
  drawChart();
  
  // WebSocket connection
  const wsUrl = 'ws://' + location.hostname + ':81/';
  let ws;
  let reconnectTimer;
   
  function connect() {
    console.log('Connecting to WebSocket:', wsUrl);
    
    try {
      ws = new WebSocket(wsUrl);
      
      ws.addEventListener('open', () => {
        console.log('WebSocket connected');
        statusEl.textContent = 'Connected';
        statusEl.className = 'connected';
        
        if (reconnectTimer) {
          clearTimeout(reconnectTimer);
          reconnectTimer = null;
        }
        
        // Refresh stats on connection
        refreshStats();
      });
      
      ws.addEventListener('message', (evt) => {
        try {
          const d = JSON.parse(evt.data);
          
          // Update values
          if (d.co2 !== undefined && d.co2 !== null) {
            co2El.textContent = d.co2.toFixed(0);
            
            // Update chart data
            co2Data.push(parseFloat(d.co2));
            if (co2Data.length > maxDataPoints) {
              co2Data.shift();
            }
            drawChart();
          }
          if (d.temp !== undefined && d.temp !== null) {
            tempEl.textContent = d.temp.toFixed(1);
          }
          if (d.hum !== undefined && d.hum !== null) {
            humEl.textContent = d.hum.toFixed(1);
          }
          if (d.pwm !== undefined && d.pwm !== null) {
            pwmEl.textContent = d.pwm;
          }
        } catch (e) {
          console.warn('Error parsing message:', e, evt.data);
        }
      });
      
      ws.addEventListener('close', () => {
        console.log('WebSocket disconnected');
        statusEl.textContent = 'Disconnected';
        statusEl.className = 'disconnected';
        
        // Reconnect after 2 seconds
        reconnectTimer = setTimeout(connect, 2000);
      });
      
      ws.addEventListener('error', (err) => {
        console.error('WebSocket error:', err);
        ws.close();
      });
      
    } catch (e) {
      console.error('Failed to create WebSocket:', e);
      // Retry connection
      reconnectTimer = setTimeout(connect, 2000);
    }
  }
  
  // Excel/CSV functions
  window.downloadCSV = function() {
    const link = document.createElement('a');
    link.href = '/download';
    link.download = 'scd30_data.csv';
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
  };
  
  window.clearData = function() {
    if (confirm('Are you sure you want to clear all logged data?')) {
      fetch('/clear', { method: 'POST' })
        .then(response => response.text())
        .then(result => {
          alert(result);
          refreshStats();
        })
        .catch(error => console.error('Error:', error));
    }
  };
  
  window.refreshStats = function() {
    fetch('/stats')
      .then(response => response.json())
      .then(data => {
        recordCountEl.textContent = data.totalRecords + '/' + data.maxRecords;
        avgCO2El.textContent = data.avgCO2 ? data.avgCO2.toFixed(0) + ' ppm' : '—';
        avgTempEl.textContent = data.avgTemp ? data.avgTemp.toFixed(1) + '°C' : '—';
      })
      .catch(error => console.error('Error fetching stats:', error));
  };
   
  // Start connection
  connect();
  
  // Redraw chart on window resize
  window.addEventListener('resize', drawChart);
  
  // Auto-refresh stats every 10 seconds
  setInterval(refreshStats, 10000);
})();