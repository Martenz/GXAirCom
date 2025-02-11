/**
 * ----------------------------------------------------------------------------
 * ESP32 Remote Control with WebSocket
 * ----------------------------------------------------------------------------
 * © 2020 Stéphane Calderoni
 * ----------------------------------------------------------------------------
 */

 var gateway = `ws://${window.location.hostname}/ws`;
 var websocket;
 
 // ----------------------------------------------------------------------------
 // Initialization
 // ----------------------------------------------------------------------------
 
 window.addEventListener('load', onLoad);
 
 window.addEventListener("unload", function () {
    if(websocket.readyState == WebSocket.OPEN)
    websocket.close();
});

 function onLoad(event) {
     initWebSocket();
 }
 
 // ----------------------------------------------------------------------------
 // WebSocket handling
 // ----------------------------------------------------------------------------
 
 function initWebSocket() {
     console.log('Trying to open a WebSocket connection...');
     websocket = new WebSocket(gateway);
     websocket.onopen  = onOpen;
     websocket.onclose = onClose;
     websocket.onmessage = onMessage; 
 }
 
 function onOpen(event) {
     console.log('Connection opened');
 }
 
 function onClose(event) {
     console.log('Connection closed');
     setTimeout(initWebSocket, 2000);
 }

 function onMessage(event) {
    console.log(`Received a notification from ${event.origin}`);
    console.log(event);
    let data = JSON.parse(event.data);
    
    document.getElementById('dev_id').innerHTML = data.dev_id;
    document.getElementById('run_time').innerHTML = data.run_time;
    document.getElementById('b_mv').innerHTML = data.b_mv.toFixed(2);
    document.getElementById('b_perc').innerHTML = data.b_perc;
    document.getElementById('gps_sat').innerHTML = data.gps_sat;
    document.getElementById('gps_fix').innerHTML = data.gps_fix;
    document.getElementById('temp_c').innerHTML = data.temp_c.toFixed(1);
    document.getElementById('spk_vol').innerHTML = data.spk_vol;
    document.getElementById('sd_size').innerHTML = data.sd_size;
    document.getElementById('pres_hpa').innerHTML = data.pres_hpa.toFixed(2);
    document.getElementById('gps_alt').innerHTML = data.gps_alt.toFixed();
    document.getElementById('baro_alt').innerHTML = data.baro_alt.toFixed();
    document.getElementById('gps_time').innerHTML = data.gps_time;
    
}

function initButtons() {
    document.getElementById('test_speaker').addEventListener('click', onTestSpeaker);
}

function onTestSpeaker(event) {
    websocket.send(JSON.stringify({'speaker':'test'}));
}