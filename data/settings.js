/**
 * ----------------------------------------------------------------------------
 * ESP32 Remote Control with WebSocket
 * ----------------------------------------------------------------------------
 * © 2020 Stéphane Calderoni
 * ----------------------------------------------------------------------------
 */

 var gateway = `ws://${window.location.hostname}/ws`;
 var websocket;
 var pname_empty = true;
 var only_once = true;
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
     initButtons();
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

    var pname = document.getElementById('pilot_name');
    if ((!pname.value) && pname_empty){
        pname.value = data.pilot_name;
        pname_empty = false;
    }

    //document.getElementById('spk_vol').innerHTML = data.spk_vol;
    var spkvol = document.querySelectorAll('#volume option');
    spkvol.forEach(function(element, index) {
    // current DOM element
        if(element.value == data.spk_vol){
            element.selected = true;
        }else{
            element.selected = false;
        }
    });   
    
    //document.getElementById('gps_hz').innerHTML = data.gps_hz;
    var gpsfrq = document.querySelectorAll('#gps_frq option');
    gpsfrq.forEach(function(element, index) {
    // current DOM element
        if(element.value == data.gps_hz){
            element.selected = true;
        }else{
            element.selected = false;
        }
    });   

    var wifiTimers = document.querySelectorAll('#wifi_timer option');
    wifiTimers.forEach(function(element, index) {
    // current DOM element
        if(element.value == data.wifi_timer){
            element.selected = true;
        }else{
            element.selected = false;
        }
    });    

    var epaper = document.querySelectorAll('#epaper option');
    epaper.forEach(function(element, index) {
    // current DOM element
        if(element.value == data.epaper){
            element.selected = true;
        }else{
            element.selected = false;
        }
    });    

    var rotation = document.querySelectorAll('#rotation option');
    rotation.forEach(function(element, index) {
    // current DOM element
        if(element.value == data.rotation){
            element.selected = true;
        }else{
            element.selected = false;
        }
    });    

    if (data.wifi_timer != 0){
        var wifi_countdown_min = document.getElementById('wifi_countdown_min');
        wifi_countdown_min.innerHTML = Math.floor(data.wifi_countdown/60);
        var wifi_countdown_sec = document.getElementById('wifi_countdown_sec');
        wifi_countdown_sec.innerHTML = Math.round(data.wifi_countdown - Math.floor(data.wifi_countdown/60)*60);    
    }

    // if (data.wifi_countdown < 10){
    //     alert("Shut Down Wifi and Restarting in 10 sec.");
    // }

    var rfmode = document.querySelectorAll('#rfmode option');
    rfmode.forEach(function(element, index) {
    // current DOM element
        if(element.value == data.rfmode){
            element.selected = true;
        }else{
            element.selected = false;
        }
    });   
    

    if (only_once){
        document.getElementById('beep_when_f').checked = data.beep_when_f;
//        document.getElementById('auto_switch_page').checked = data.auto_switch_page;
//        document.getElementById('t_refresh').value = data.t_refresh;
        document.getElementById('fly_min_v').value = data.fly_min_v;
        document.getElementById('fly_min_t').value = data.fly_min_t;
        document.getElementById('fly_stop_t').value = data.fly_stop_t;
        document.getElementById('min_sat_av').value = data.min_sat_av;
        document.getElementById('utc_offset').value = data.utc_offset;
        document.getElementById('AircraftType').value = data.AircraftType;

        only_once = false;
    }

    var loader = document.getElementById('loader_s');
    loader.style.display = "none"; 

}

function initButtons() {
    document.getElementById('volume').addEventListener('change', onVolumeSpeaker);
    document.getElementById('AircraftType').addEventListener('change', onAircraftType);
    //document.getElementById('test_speaker').addEventListener('click', onTestSpeaker);
    document.getElementById('wifi_timer').addEventListener('change', onWifiOff);
    document.getElementById('restart').addEventListener('click', onRestartTzI);
    //document.getElementById('wifi_psw').addEventListener('keyup', checkInput);
    //document.getElementById('save_wifi_psw').addEventListener('click', onSaveWifiPsw);
    document.getElementById('save_pilot_name').addEventListener('click', onSavePilot);
    document.getElementById('beep_when_f').addEventListener('change',onBeepWhenF);
    //document.getElementById('auto_switch_page').addEventListener('change',onAutoSwitchPage);
    //document.getElementById('t_refresh').addEventListener('change', onDisplayTimer);
    //document.getElementById('epaper').addEventListener('change', onEpaper);
    document.getElementById('rotation').addEventListener('change', onRotation);
    document.getElementById('fly_min_v').addEventListener('change', onFlyMinV);
    document.getElementById('fly_min_t').addEventListener('change', onFlyMinT);
    document.getElementById('fly_stop_t').addEventListener('change', onFlyStopT);
    document.getElementById('restart_gps').addEventListener('click', onGPSrestart);
    //document.getElementById('gps_frq').addEventListener('change',onGpsChangeFrq);
    document.getElementById('min_sat_av').addEventListener('change',onMinSatAv);
    document.getElementById('utc_offset').addEventListener('change',onUTCoffset);
    document.getElementById('rfmode').addEventListener('change', onRfmode);

}

function showLoader(){
    var loader = document.getElementById('loader_s');
    loader.style.display = "flex"; 
}

function onEpaper(event){
    var select = document.getElementById('epaper');
    var value = select.options[select.selectedIndex].value.toString();
    showLoader();
    websocket.send(JSON.stringify({'epaper':value,'update':true}));
    console.log("New display type:"+value);
    alert("Need a restart of the device to apply new Epaper Type. Please restart.");
}

function onRotation(event){
    var select = document.getElementById('rotation');
    var value = select.options[select.selectedIndex].value.toString();
    showLoader();
    websocket.send(JSON.stringify({'rotation':value,'update':true}));
    console.log("New display rotation:"+value);
}

function onUTCoffset(event){
    var inp = document.getElementById('utc_offset');
    var value = inp.value;
    showLoader();
    websocket.send(JSON.stringify({'utc_offset':value,'update':true}));
    console.log("New UTC offset:"+value);
}

function onMinSatAv(event){
    var inp = document.getElementById('min_sat_av');
    var value = inp.value;
    showLoader();
    websocket.send(JSON.stringify({'min_sat_av':value,'update':true}));
    console.log("New Min Sat Available:"+value);
}

function onAutoSwitchPage(event){
    var chk = document.getElementById('auto_switch_page');
    var value = chk.checked;
    showLoader();
    websocket.send(JSON.stringify({'auto_switch_page':value,'update':true}));
    console.log("auto Switch Page Data when flying: "+value);
}

function onDisplayTimer(event){
    var inp = document.getElementById('t_refresh');
    var value = inp.value;
    showLoader();
    websocket.send(JSON.stringify({'t_refresh':value,'update':true}));
    console.log("New Display refresh timer (min):"+value);
}

function onGPSrestart(event){
    if (confirm("Force GPS Restart, next Fix can take up to 10 minutes.") == true) {
        websocket.send(JSON.stringify({'restart_gps':true}));
        showLoader();
    } 
}

function onGpsChangeFrq(event){
    var select = document.getElementById('gps_frq');
    var value = select.options[select.selectedIndex].value.toString();

    showLoader();
    websocket.send(JSON.stringify({'gps_hz':value,'update':true}));
    console.log("New gps Frq Hz:"+value);
}

function onFlyMinV(event){
    var inp = document.getElementById('fly_min_v');
    var value = inp.value;
    showLoader();
    websocket.send(JSON.stringify({'fly_min_v':value,'update':true}));
    console.log("New Fly Min Speed:"+value);
};

function onFlyMinT(event){
    var inp = document.getElementById('fly_min_t');
    var value = inp.value;
    showLoader();
    websocket.send(JSON.stringify({'fly_min_t':value,'update':true}));
    console.log("New Fly Min Time:"+value);
};

function onFlyStopT(event){
    var inp = document.getElementById('fly_stop_t');
    var value = inp.value;
    showLoader();
    websocket.send(JSON.stringify({'fly_stop_t':value,'update':true}));
    console.log("New Fly Stop Time:"+value);
};

function onBeepWhenF(event){
    var chk = document.getElementById('beep_when_f');
    var value = chk.checked;
    showLoader();
    websocket.send(JSON.stringify({'beep_when_f':value,'update':true}));
    console.log("Beep only when flying: "+value);
}

function checkInput(event){
    var inp = document.getElementById('wifi_psw');
    var value = inp.value.toString();
    var inp_btn = document.getElementById('save_wifi_psw')
    if(value.length >=8 && value.length <=12){
        inp_btn.disabled = false;
    }else{
        inp_btn.disabled = true;
    }

}

function onSavePilot(event){
    var inp = document.getElementById('pilot_name');
    var value = inp.value.toString();
    showLoader();
    websocket.send(JSON.stringify({'pilot_name':value,'update':true}));
    console.log("New Pilot Name:"+value);
}

function onRestartTzI(event){
    if (confirm("Force Restart, then enable WiFi and refresh to reconnect.") == true) {
        websocket.send(JSON.stringify({'restart':true}));
        showLoader();
    } 
}

function onSaveWifiPsw(event){
    var inp = document.getElementById('wifi_psw');
    var value = inp.value.toString();

    showLoader();
    websocket.send(JSON.stringify({'wifi_psw':value,'update':true}));
}

function onVolumeSpeaker(event) {
    var select = document.getElementById('volume');
    var value = select.options[select.selectedIndex].value.toString();

    showLoader();
    websocket.send(JSON.stringify({'speaker':'test','volume':value,'update':true}));
}

function onAircraftType(event) {
    var select = document.getElementById('AircraftType');
    var value = select.options[select.selectedIndex].value.toString();

    showLoader();
    websocket.send(JSON.stringify({'AircraftType':value,'update':true}));
}

function onTestSpeaker(event) {
    var select = document.getElementById('volume');
    var value = select.options[select.selectedIndex].value.toString();
    showLoader();
    websocket.send(JSON.stringify({'speaker':'test','volume':value}));
}

function onWifiOff(event) {
    var select = document.getElementById('wifi_timer');
    var value = select.options[select.selectedIndex].value.toString();
    showLoader();
    websocket.send(JSON.stringify({'wifi_timer':value,'update':true}));
}

function onRfmode(event) {
    var select = document.getElementById('rfmode');
    var value = select.options[select.selectedIndex].value.toString();
    showLoader();
    websocket.send(JSON.stringify({'rfmode':value,'update':true}));
}