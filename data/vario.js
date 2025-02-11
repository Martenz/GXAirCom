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

 const minf = 0;
const maxf = 2000;

const mint = 0;
const maxt = 2000;

var playing = false;

const graph = document.getElementById('graph');

const g_height = document.getElementById('graph').clientHeight;
console.log(g_height);

var audioContext = null;
var oscillatorNode = null;
var stopTime = 0;

var default_curve={0:[
    {"vval":10,"frq":2000,"ton":100,"toff":100},
    {"vval":5,"frq":1500,"ton":200,"toff":100},
    {"vval":2.5,"frq":1250,"ton":200,"toff":150},
    {"vval":0.5,"frq":1000,"ton":200,"toff":200},
    {"vval":0,"frq":0,"ton":0,"toff":50},
    {"vval":-2.5,"frq":450,"ton":250,"toff":750},
    {"vval":-5,"frq":250,"ton":500,"toff":1000},
    {"vval":-10,"frq":200,"ton":500,"toff":1000}
    ],
    1:[
    {"vval":10,"frq":2000,"ton":100,"toff":100},
    {"vval":5,"frq":1500,"ton":200,"toff":100},
    {"vval":0.55,"frq":400,"ton":150,"toff":350},
    {"vval":0.5,"frq":120,"ton":50,"toff":140},
    {"vval":-0.2,"frq":80,"ton":50,"toff":140},
    {"vval":-0.25,"frq":80,"ton":20,"toff":20},
    {"vval":-2.5,"frq":250,"ton":500,"toff":1000},
    {"vval":-10,"frq":200,"ton":500,"toff":1000}
    ],
    2:[
    {"vval":10,"frq":2000,"ton":100,"toff":100},
    {"vval":5,"frq":1500,"ton":200,"toff":100},
    {"vval":0.2,"frq":400,"ton":150,"toff":400},
    {"vval":0.1,"frq":120,"ton":50,"toff":140},
    {"vval":-0.200000003,"frq":80,"ton":50,"toff":140},
    {"vval":-0.25,"frq":80,"ton":20,"toff":20},
    {"vval":-2.5,"frq":250,"ton":500,"toff":1000},
    {"vval":-10,"frq":200,"ton":500,"toff":1000}
    ],
    3:[
    {"vval":10,"frq":2000,"ton":100,"toff":100},
    {"vval":5,"frq":1500,"ton":200,"toff":100},
    {"vval":0.55,"frq":610,"ton":200,"toff":410},
    {"vval":0.35,"frq":400,"ton":200,"toff":410},
    {"vval":-0.2,"frq":80,"ton":50,"toff":1500},
    {"vval":-0.25,"frq":80,"ton":20,"toff":20},
    {"vval":-2.5,"frq":250,"ton":600,"toff":1000},
    {"vval":-10,"frq":500,"ton":600,"toff":1000}
    ],
}

 // ----------------------------------------------------------------------------
 // Initialization
 // ----------------------------------------------------------------------------
 
 window.addEventListener('load', onLoad);
 
 window.addEventListener("unload", function () {
    if(websocket.readyState == WebSocket.OPEN)
    websocket.close();
});
 
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

    if (only_once){
        document.getElementById('sink_on').value = data.sink_on;
        document.getElementById('lift_on').value = data.lift_on;

        document.getElementById('kalman_e_mea').value = data.kalman_e_mea;
        document.getElementById('kalman_e_est').value = data.kalman_e_est;
        document.getElementById('kalman_q').value = data.kalman_q;
        document.getElementById('vario_avg_ms').value = data.vario_avg_ms;
        document.getElementById('vario_avg_ms_b').value = data.vario_avg_ms_b;

        document.getElementById('thermal_detect').checked = data.thermal_detect;
        document.getElementById('thermal_avg').value = data.thermal_avg;

        only_once = false;
        hideLoader();
    }

 }

function interpolate(vval,curve){
    var vv1 = 0;
    var vv2 = 0;
    var pv1 = 0;
    var pv2 = 0;
    var to1 = 0;
    var to2 = 0;
    var tf1 = 0;
    var tf2 = 0;

  var sorted_curve = Object.create(curve);
  sorted_curve.sort((a, b) => a.vval - b.vval);
  console.log(sorted_curve);

      let frq = 0,
      ton  = 0,
      toff = 0;

    for (var prop in sorted_curve) {
        if (sorted_curve[prop]["vval"] < vval){
            vv1 = sorted_curve[prop]["vval"];  
            pv1 = sorted_curve[prop]["frq"];  
            to1 = sorted_curve[prop]["ton"];            
            tf1 = sorted_curve[prop]["toff"];            
        }else if(sorted_curve[prop]["vval"] > vval){  
            vv2 = sorted_curve[prop]["vval"]; 
            pv2 = sorted_curve[prop]["frq"]; 
            to2 = sorted_curve[prop]["ton"];            
            tf2 = sorted_curve[prop]["toff"];  
            break;
        }else if (sorted_curve[prop]["vval"] == vval){
            frq =  sorted_curve[prop]["frq"];
            ton =  sorted_curve[prop]["ton"];
            toff =  sorted_curve[prop]["toff"];
            return { frq, ton, toff };
        }
    }
    const ratio  = (vval - vv1)/(vv2 - vv1); 
    frq =  pv1 + ratio * (pv2 - pv1);  
    ton =  to1 + ratio * (to2 - to1);
    toff =  tf1 + ratio * (tf2 - tf1);

    console.log(vval);
    console.log(vv1);
    console.log(vv2);
    console.log(ratio);
    console.log(pv1);
    console.log(pv2);

    return { frq, ton, toff };
}

function showLoader(){
    var loader = document.getElementById('loader_s');
    loader.style.display = "flex"; 
}
function hideLoader(){
    var loader = document.getElementById('loader_s');
    loader.style.display = "none"; 
}


function sendTestJson(){
    console.log("vval input changed");
    var vval = document.getElementById('vval_demo').value;
    let interpolated = interpolate(vval,curve);
    frq = interpolated.frq;
    ton = interpolated.ton;
    toff = interpolated.toff;

    console.log(JSON.stringify(curve));
    console.log("vval: "+ vval +" frq: "+frq+" ton: "+ton+" toff: "+toff);
    websocket.send(JSON.stringify({'tv_vval':vval,'tv_frq':frq, 'tv_ton':ton, 'tv_toff':toff}));    
}

function exportCurve(){
    const filename = 'vario_curve.json';
    const jsonStr = JSON.stringify(curve);
    let element = document.createElement('a');
    element.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(jsonStr));
    element.setAttribute('download', filename);
    element.style.display = 'none';
    document.body.appendChild(element);
    element.click();
    document.body.removeChild(element);
}

function drawVarioCurve(){
    for (var prop in curve) {
        var divrow_inpt = document.createElement("tr");
        var td = document.createElement("td")

        var vval = document.createElement("input")
        vval.classList.add('inpt');
        vval.classList.add('inpt_vval');
        vval.classList.add('vval');
        vval.value = curve[prop]["vval"];
        vval.type = "number";
        vval.min =-20.0;
        vval.max = +20.0;
//        vval.lang = "en";
        vval.step = 0.05;
        vval.setAttribute('idx',prop);
        vval.setAttribute('field',"vval");
        vval.setAttribute("onchange",`validity.valid||(value='${vval.value}')`);
        td.appendChild(vval);
        divrow_inpt.appendChild(td);

        var td = document.createElement("td")
        var frq = document.createElement("input")
        frq.id = "frq_"+prop;
        frq.classList.add('inpt');
        frq.classList.add('inline-block');
        frq.classList.add('sl_inp');
        frq.classList.add('frq');
        frq.type = "range";
        frq.min = 0;
        frq.max = 2000;
        frq.step = 10;
        frq.setAttribute('idx',prop);
        frq.setAttribute('field',"frq");
        frq.setAttribute('oninput',"ofrq_"+prop+".value = frq_"+prop+".value");
        frq_out = document.createElement("output");
        frq_out.id = "ofrq_"+prop;
        td.appendChild(frq_out);
        td.appendChild(frq);
        divrow_inpt.appendChild(td);
        frq.value = curve[prop]["frq"];
        frq_out.value = curve[prop]["frq"];

        var td = document.createElement("td")
        var ton = document.createElement("input")
        ton.id = "ton_"+prop;
        ton.classList.add('inpt');
        ton.classList.add('inline-block');
        ton.classList.add('sl_inp');
        ton.classList.add('ton');
        ton.type = "range";
        ton.min = 0;
        ton.max = 2000;
        ton.step = 10;
        ton.setAttribute('idx',prop);
        ton.setAttribute('field',"ton");
        ton.setAttribute('oninput',"oton_"+prop+".value = ton_"+prop+".value");
        ton_out = document.createElement("output");
        ton_out.id = "oton_"+prop;
        td.appendChild(ton_out);
        td.appendChild(ton);
        divrow_inpt.appendChild(td);
        ton.value = curve[prop]["ton"];
        ton_out.value = curve[prop]["ton"];

        var td = document.createElement("td")
        var toff = document.createElement("input")
        toff.id = "toff_"+prop;
        toff.classList.add('inpt');
        toff.classList.add('inline-block');
        toff.classList.add('sl_inp');
        toff.classList.add('toff');
        toff.type = "range";
        toff.min = 0;
        toff.max = 2000;
        toff.step = 10;
        toff.setAttribute('idx',prop);
        toff.setAttribute('field',"toff");
        toff.setAttribute('oninput',"otoff_"+prop+".value = toff_"+prop+".value");
        toff_out = document.createElement("output");
        toff_out.id = "otoff_"+prop;
        td.appendChild(toff_out);
        td.appendChild(toff);
        divrow_inpt.appendChild(td);
        toff.value = curve[prop]["toff"];
        toff_out.value = curve[prop]["toff"];

        divrow_inpt.appendChild(td);

        graph.appendChild(divrow_inpt);
    }

    document.querySelectorAll('.inpt').forEach(item => {
        if (item.getAttribute('field') != 'vval'){

            item.addEventListener('change', event => {
                if ( item.validity.rangeUnderflow){
                    item.value = 0;
                }
                if ( item.validity.rangeOverflow){
                    item.value = 2000;
                }

                const idx = item.getAttribute('idx');
                const field = item.getAttribute('field');
                curve[idx][field] = parseInt(item.value); 

                console.log(JSON.stringify(curve));
                sendTestJson();
            })
        }else{
            item.addEventListener('change', event => {
                if ( item.validity.rangeUnderflow || item.validity.rangeOverflow){
                    item.value = 0;
                }

                const idx = item.getAttribute('idx');
                const field = item.getAttribute('field');
                curve[idx][field] = parseFloat(item.value); 

                console.log(JSON.stringify(curve));
                sendTestJson();
            })            
        }
      })

    const colors = {"frq":"#71ae1e","ton":"#8e9388","toff":"black"}

    document.querySelectorAll('.sl_inp').forEach(item => {
        var value = (item.value-item.min)/(item.max-item.min)*100.0;
        var color = colors[item.getAttribute("field")];
        item.style.background = 'linear-gradient(to right, '+color+' 0%, '+color+' ' + value + '%, transparent ' + value + '%, transparent 100%)'            
        item.addEventListener('input',event => {
            var value = (item.value-item.min)/(item.max-item.min)*100;
            item.style.background = 'linear-gradient(to right, '+color+' 0%, '+color+' ' + value + '%, transparent ' + value + '%, transparent 100%)'            
        })
    });      
}

function setDefaultCurve(){
    showLoader();
    var select = document.getElementById('default_curves');
    var value = select.options[select.selectedIndex].value.toString();
    curve = default_curve[value];

    graph.innerHTML = "";
    setTimeout(function(){
        drawVarioCurve();
        hideLoader();
    },500);

    hideLoader();
}

function importCurve(){
    var files = document.getElementById('selectFiles').files;
    console.log(files);
    if (files.length <= 0) {
        return false;
    }

    var fr = new FileReader();

    fr.onload = function(e) { 
    console.log(e);
        curve = JSON.parse(e.target.result);
        console.log(curve);
    }
    fr.readAsText(files.item(0));

    showLoader();
    graph.innerHTML = "";
    setTimeout(function(){
        drawVarioCurve();
        hideLoader();
    },500);
    
}
 
function onLoad(event) {

    initWebSocket();

    document.getElementById("export_vario").addEventListener('click',exportCurve);
    document.getElementById("import_vario").addEventListener('click',importCurve);
    document.getElementById("default_curves").addEventListener('change',setDefaultCurve);
    document.getElementById("selectFiles").addEventListener('change',function(){
        var import_b = document.getElementById("import_vario");
        import_b.disabled = false;
    })

    document.getElementById("slider_vario_dw").addEventListener('click',function(){
        var rslider = document.getElementById("vval_demo");
        rslider.stepDown();
        document.getElementById("ageOutputId").value = rslider.value;
        sendTestJson();
    });

    document.getElementById("slider_vario_up").addEventListener('click',function(){
        var rslider = document.getElementById("vval_demo")
        rslider.stepUp();
        document.getElementById("ageOutputId").value = rslider.value;
        sendTestJson();
    });

    document.getElementById("save_vario").addEventListener('click',function(event){
        showLoader();
        console.log("Saving curve:");
        console.log(JSON.stringify({'vario_curve':curve, 'update':true}));

        //save also sink lift values
        var sink_on = document.getElementById('sink_on').value;
        var lift_on = document.getElementById('lift_on').value;

        //save Kalman filter parameters and vario averaging factor
        const kalman_e_mea = document.getElementById('kalman_e_mea').value;
        const kalman_e_est = document.getElementById('kalman_e_est').value;
        const kalman_q = document.getElementById('kalman_q').value;
        const vario_avg_ms = document.getElementById('vario_avg_ms').value;
        const vario_avg_ms_b = document.getElementById('vario_avg_ms_b').value;
        const thermal_detect = document.getElementById('thermal_detect').value;
        const thermal_avg = document.getElementById('thermal_avg').value;

        websocket.send(JSON.stringify({
            'vario_curve':curve,
            'sink_on':sink_on,
            'lift_on':lift_on,
            'kalman_e_mea':kalman_e_mea,
            'kalman_e_est':kalman_e_est,
            'kalman_q':kalman_q,
            'vario_avg_ms':vario_avg_ms,
            'vario_avg_ms_b':vario_avg_ms_b,
            'thermal_detect':thermal_detect,
            'thermal_avg':thermal_avg,
            'update':true,
        }));  
        alert("Vario settings saved. Restrating to apply.");  
        websocket.send(JSON.stringify({
            'restart':true
        }));  
        hideLoader();
        alert("TzI Restarted. To enter Settings again enable Wifi first and reload this page.");  
    });

    drawVarioCurve();

    document.getElementById('play').addEventListener('click',function(event){
        playing = playing ? false : true;
        console.log(playing);

        if (playing){
            document.getElementById('play').innerHTML = 'Stop';    
            websocket.send(JSON.stringify({'test_vario':true}));    

        }else{
            document.getElementById('play').innerHTML = 'Play';
            websocket.send(JSON.stringify({'test_vario':false}));    
        }
        
    });

    document.getElementById('vval_demo').addEventListener('change',sendTestJson);

    document.getElementById('thermal_detect').addEventListener('change', (event) => {
        if (event.currentTarget.checked) {
          document.getElementById('thermal_avg').removeAttribute("disabled");
        } else {
          document.getElementById('thermal_avg').setAttribute("disabled", true);
        }
    });

    console.log("Init Done.");

    hideLoader();

}