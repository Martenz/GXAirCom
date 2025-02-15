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
    
    if (data.dev_id) document.getElementById('dev_id').innerHTML = data.dev_id;
    if (data.run_time) document.getElementById('run_time').innerHTML = data.run_time;
    if (data.b_mv) document.getElementById('b_mv').innerHTML = data.b_mv.toFixed(2);
    if (data.b_perc) document.getElementById('b_perc').innerHTML = data.b_perc;
    if (data.gps_sat) document.getElementById('gps_sat').innerHTML = data.gps_sat;
    if (data.gps_fix) document.getElementById('gps_fix').innerHTML = data.gps_fix;
    if (data.temp_c) document.getElementById('temp_c').innerHTML = data.temp_c.toFixed(1);
    if (data.spk_vol) document.getElementById('spk_vol').innerHTML = data.spk_vol;
    if (data.sd_size) document.getElementById('sd_size').innerHTML = data.sd_size;
    if (data.pres_hpa) document.getElementById('pres_hpa').innerHTML = data.pres_hpa.toFixed(2);
    if (data.gps_alt) document.getElementById('gps_alt').innerHTML = data.gps_alt.toFixed();
    if (data.baro_alt) document.getElementById('baro_alt').innerHTML = data.baro_alt.toFixed();
    if (data.gps_time) document.getElementById('gps_time').innerHTML = data.gps_time;
    if (data.fanet_rx) document.getElementById('fanet_rx').innerHTML = data.fanet_rx;
    if (data.fanet_tx) document.getElementById('fanet_tx').innerHTML = data.fanet_tx;
    if (data.leg_rx) document.getElementById('leg_rx').innerHTML = data.leg_rx;
    if (data.leg_tx) document.getElementById('leg_tx').innerHTML = data.leg_tx;    
}

function initButtons() {
    document.getElementById('test_speaker').addEventListener('click', onTestSpeaker);
}

function onTestSpeaker(event) {
    websocket.send(JSON.stringify({'speaker':'test'}));
}