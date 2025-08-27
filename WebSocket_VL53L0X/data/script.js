// Use backticks ` ` for template literals! Your string was using single quotes ' ' which won't work for ${}
let gateway = `ws://${window.location.hostname}/ws`; 
let websocket;

window.addEventListener('load', onLoad);

function onLoad() {
    initWebSocket();  // You forgot parentheses here! You need to call the function.
}

function initWebSocket(){
    console.log('Trying to open a WebSocket connection...');

    // Fix typo: It should be WebSocket (with capital S, no extra s)
    websocket = new WebSocket(gateway);

    websocket.onopen = onOpen;
    websocket.onclose = onClose;

    // Fix typo: onmessage (not onmesage)
    websocket.onmessage = onMessage;
}

function onOpen(event){
    console.log('WebSocket connection opened');
    document.getElementById('status').innerHTML = 'Connected';
}

function onClose(event){
    console.log('WebSocket connection closed');
    document.getElementById('status').innerHTML = 'Disconnected';
    setTimeout(initWebSocket, 2000);  // retry connection after 2 seconds
}

function onMessage(event){
    console.log('Message received from ESP32:', event.data);
    document.getElementById('sensor').innerHTML = event.data;
}
