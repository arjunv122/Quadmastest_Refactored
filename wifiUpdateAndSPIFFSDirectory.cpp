#include "wifiUpdateAndSPIFFSDirectory.h"
#include "CSS.h"
#include <DNSServer.h>  // ← ADD THIS for Captive Portal

#ifndef ESP8266
  #include <WiFiManager.h>
#endif

String webpage = "";
bool SPIFFS_present = false;
String addIP = "";

// Global WiFiManager instance (needed for reset route)
WiFiManager wifiManager;

// ============================================================
// NEW: DNS Server for Captive Portal Auto-Redirect
// ============================================================
DNSServer dnsServer;
const byte DNS_PORT = 53;
bool captivePortalActive = false;

#ifdef ESP8266
  ESP8266WebServer server(80);
#else
  WebServer server(80);
#endif

// ============================================================
// HELPER FUNCTIONS
// ============================================================

void SendHTML_Header(){
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html; charset=UTF-8", "");
  append_page_header();
  server.sendContent(webpage);
  webpage = "";
}

void SendHTML_Content(){
  server.sendContent(webpage);
  webpage = "";
}

void SendHTML_Stop(){
  server.sendContent("");
  server.client().stop();
}

void ReportSPIFFSNotPresent(){
  SendHTML_Header();
  webpage += F("<h3>No SPIFFS present</h3>");
  webpage += F("<a href='/dir'><button class='btn-main'>Back</button></a>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

void ReportFileNotPresent(String target){
  SendHTML_Header();
  webpage += F("<h3>File not found</h3>");
  webpage += F("<a href='/dir'><button class='btn-main'>Back</button></a>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

String file_size(int bytes){
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < 1048576) return String(bytes/1024.0, 2) + " KB";
  else return String(bytes/1048576.0, 2) + " MB";
}

// ============================================================
// NEW:  CAPTIVE PORTAL HANDLERS - AUTO REDIRECT TO /dir
// ============================================================

// Handle all unknown URLs - redirect to main page
void handleNotFound() {
  String redirectUrl = "http://" + WiFi.localIP().toString() + "/dir";
  server.sendHeader("Location", redirectUrl, true);
  server.send(302, "text/plain", "");
}

// Captive portal redirect handler
void handleCaptivePortal() {
  String redirectUrl = "http://" + WiFi.localIP().toString() + "/dir";
  server.sendHeader("Location", redirectUrl, true);
  server.send(302, "text/plain", "");
}

// Apple/iOS captive portal detection - returns HTML that redirects
void handleHotspotDetect() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta http-equiv='refresh' content='0;url=http://" + WiFi.localIP().toString() + "/dir'>";
  html += "</head><body>Redirecting to QMT File Manager... </body></html>";
  server.send(200, "text/html", html);
}

// ============================================================
// WIFI RESET PAGE - Manual way to reset WiFi credentials
// ============================================================

void WiFi_Reset_Page() {
  String html = "";
  html += F("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
  html += F("<title>WiFi Reset</title>");
  html += F("<meta name='viewport' content='width=device-width,initial-scale=1.0'>");
  html += F("<style>");
  html += F("body{font-family: Arial;background:#f5f5f5;text-align:center;padding:50px 20px;}");
  html += F(".box{background:#fff;max-width:400px;margin:auto;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}");
  html += F("h2{color:#333;margin-bottom:20px;}");
  html += F("p{color:#666;margin-bottom:25px;}");
  html += F(".btn{padding:15px 30px;font-size:16px;border:none;border-radius:8px;cursor:pointer;margin:10px;}");
  html += F(".btn-danger{background:#e53935;color:#fff;}");
  html += F(".btn-danger:hover{background:#c62828;}");
  html += F(".btn-cancel{background:#9e9e9e;color:#fff;}");
  html += F(".btn-cancel:hover{background:#757575;}");
  html += F("</style></head><body>");
  html += F("<div class='box'>");
  html += F("<h2>Reset WiFi Settings? </h2>");
  html += F("<p>This will clear saved WiFi credentials.  The device will restart and open the WiFi configuration portal.</p>");
  html += F("<form action='/wifi-reset-confirm' method='POST'>");
  html += F("<button type='submit' class='btn btn-danger'>Yes, Reset WiFi</button>");
  html += F("</form>");
  html += F("<a href='/dir'><button class='btn btn-cancel'>Cancel</button></a>");
  html += F("</div></body></html>");
  
  server.send(200, "text/html; charset=UTF-8", html);
}

void WiFi_Reset_Confirm() {
  String html = "";
  html += F("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
  html += F("<title>WiFi Reset</title>");
  html += F("<meta http-equiv='refresh' content='5;url=http://192.168.4.1'>");
  html += F("<style>");
  html += F("body{font-family:Arial;background:#f5f5f5;text-align:center;padding:50px 20px;}");
  html += F(".box{background:#fff;max-width: 400px;margin:auto;padding: 30px;border-radius: 10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}");
  html += F("h2{color:#4CAF50;}");
  html += F("p{color:#666;}");
  html += F("</style></head><body>");
  html += F("<div class='box'>");
  html += F("<h2>WiFi Settings Cleared! </h2>");
  html += F("<p>Device will restart in 5 seconds... </p>");
  html += F("<p>Connect to the device's WiFi hotspot and go to <b>192.168.4.1</b> to configure.</p>");
  html += F("</div></body></html>");
  
  server.send(200, "text/html; charset=UTF-8", html);
  
  delay(1000);
  
  // Clear WiFi credentials
  wifiManager.resetSettings();
  
  delay(1000);
  
  // Restart ESP32
  ESP.restart();
}

// ============================================================
// WIFI INFO PAGE - Shows current connection info
// ============================================================

void WiFi_Info_Page() {
  SendHTML_Header();
  
  webpage += F("<h2>WiFi Information</h2>");
  webpage += F("<div style='background:#fff;max-width:500px;margin:20px auto;padding: 25px;border-radius:10px;text-align:left;'>");
  
  webpage += F("<p><b>Status:</b> ");
  if (WiFi.status() == WL_CONNECTED) {
    webpage += F("<span style='color:#4CAF50;'>Connected</span></p>");
  } else {
    webpage += F("<span style='color:#f44336;'>Disconnected</span></p>");
  }
  
  webpage += "<p><b>SSID:</b> " + WiFi.SSID() + "</p>";
  webpage += "<p><b>IP Address:</b> " + WiFi.localIP().toString() + "</p>";
  webpage += "<p><b>Signal Strength:</b> " + String(WiFi.RSSI()) + " dBm</p>";
  webpage += "<p><b>MAC Address:</b> " + WiFi.macAddress() + "</p>";
  webpage += "<p><b>Device ID:</b> " + String(uniqueChipID) + "</p>";
  
  webpage += F("</div>");
  
  webpage += F("<a href='/wifi-reset'><button class='btn-main btn-danger' style='background:#e53935;'>Reset WiFi Settings</button></a>");
  webpage += F("<br><br>");
  webpage += F("<a href='/dir'><button class='btn-main'>Back to Dashboard</button></a>");
  
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

// ============================================================
// API ENDPOINTS
// ============================================================

void List_Files_JSON() {
  String json = "[";
  bool first = true;
  File root = SPIFFS.open("/");
  if (root) {
    File file = root.openNextFile();
    while (file) {
      if (!file.isDirectory()) {
        if (!first) json += ",";
        String fname = String(file.name());
        if (fname.startsWith("/")) fname = fname.substring(1);
        json += "\"" + fname + "\"";
        first = false;
      }
      file = root.openNextFile();
    }
    root.close();
  }
  json += "]";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

void Get_File_Content() {
  if (!server.hasArg("name")) {
    server.send(400, "text/plain", "Missing filename");
    return;
  }
  String filename = server.arg("name");
  if (!filename.startsWith("/")) filename = "/" + filename;
  
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    server.send(404, "text/plain", "File not found: " + filename);
    return;
  }
  String content = file.readString();
  file.close();
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain; charset=UTF-8", content);
}

// ============================================================
// 7-DAY TREND PAGE
// ============================================================

void Trend_Page() {
  String html = "";
  html += F("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Mastitis Trend</title>");
  html += F("<meta name='viewport' content='width=device-width,initial-scale=1.0'>");
  html += F("<script src='https://cdn.jsdelivr.net/npm/chart.js@4.4.1/dist/chart.umd.min.js'></script>");
  
  // CSS
  html += F("<style>");
  html += F("*{box-sizing:border-box;margin:0;padding:0;}");
  html += F("body{background:#0d1f2d;color:#fff;font-family:Arial,sans-serif;min-height:100vh;overflow-x:hidden;}");
  html += F(".header{display:flex;justify-content:space-between;align-items: center;padding:15px 25px;}");
  html += F(".header h1{color:#4ecdc4;font-size:22px;font-style:italic;margin: 0;}");
  html += F(".header .date-range{color:#4ecdc4;font-size: 14px;}");
  html += F(".controls{padding:5px 25px 15px;display:flex;align-items:center;gap:12px;}");
  html += F(".controls label{color:#aaa;font-size: 14px;}");
  html += F(".controls select{padding:6px 12px;font-size:13px;border-radius:4px;border:1px solid #555;background:#fff;}");
  html += F(".chart-container{position: relative;width:100%;height: calc(100vh - 140px);padding:0 15px 15px;}");
  html += F(".chart-container canvas{width:100%!important;height:100%!important;}");
  html += F("#status{color:#4ecdc4;padding:5px 25px;font-size:13px;}");
  html += F(".back-btn{padding:8px 16px;background:#4CAF50;color:#fff;border:none;border-radius: 5px;text-decoration:none;font-size:13px;}");
  html += F(".back-btn:hover{background:#45a049;}");
  html += F("</style></head><body>");
  
  // Header
  html += F("<div class='header'>");
  html += F("<a href='/dir' class='back-btn'>&larr; Back</a>");
  html += F("<h1>Mastitis</h1>");
  html += F("<div class='date-range' id='dateRange'>--</div>");
  html += F("</div>");
  
  // Controls
  html += F("<div class='controls'>");
  html += F("<label>RFID: </label>");
  html += F("<select id='rfidSelect'><option value=''>Loading...</option></select>");
  html += F("</div>");
  
  // Status
  html += F("<div id='status'></div>");
  
  // Chart
  html += F("<div class='chart-container'><canvas id='trendChart'></canvas></div>");
  
  // JavaScript
  html += F("<script>");
  
  html += F("var chart=null;");
  html += F("var allData={};");
  html += F("var lineColors={FL:'#f5f5dc',FR:'#ff6b9d',BL:'#ffd93d',BR:'#6bcb77'};");
  
  // Severity bands plugin
  html += F("var severityPlugin={");
  html += F("id:'severityBands',");
  html += F("beforeDatasetsDraw:function(chart){");
  html += F("var ctx=chart.ctx;");
  html += F("var area=chart.chartArea;");
  html += F("var yAxis=chart.scales.y;");
  html += F("if(!area)return;");
  html += F("function drawBand(yMin,yMax,color){");
  html += F("var top=yAxis.getPixelForValue(yMax);");
  html += F("var bottom=yAxis.getPixelForValue(yMin);");
  html += F("ctx.fillStyle=color;");
  html += F("ctx.fillRect(area.left,top,area.right-area.left,bottom-top);}");
  html += F("drawBand(0,5.5,'rgba(13,90,80,0.85)');");
  html += F("drawBand(5.5,7.5,'rgba(128,128,50,0.85)');");
  html += F("drawBand(7.5,12,'rgba(100,40,50,0.85)');");
  html += F("}};");
  
  html += F("function setStatus(msg){document.getElementById('status').textContent=msg;}");
  
  html += F("function fnToDate(fn){");
  html += F("var s=fn.replace('.txt','').replace(/\\D/g,'');");
  html += F("if(s.length>=8)return s.substring(0,2)+'/'+s.substring(2,4)+'/'+s.substring(4,8);");
  html += F("return fn;}");
  
  html += F("function getVal(lines,key){");
  html += F("for(var i=0;i<lines.length;i++){");
  html += F("if(lines[i].indexOf(key)!==-1){");
  html += F("var p=lines[i].indexOf(': ');");
  html += F("if(p>-1)return lines[i].substring(p+1).trim();");
  html += F("}}return '';}");
  
  html += F("function parseFile(content,filename){");
  html += F("var dateStr=fnToDate(filename);");
  html += F("var blocks=content.split('File Name: ');");
  html += F("for(var i=1;i<blocks.length;i++){");
  html += F("var lines=blocks[i].split('\\n');");
  html += F("var rfid=getVal(lines,'RFID No').trim();");
  html += F("if(!rfid||rfid.length<2)rfid='UNKNOWN';");
  html += F("var ecStr=getVal(lines,'EC');");
  html += F("if(!ecStr)continue;");
  html += F("var parts=ecStr.split(',');");
  html += F("if(parts.length<4)continue;");
  html += F("var ec={FL: parseFloat(parts[0]),FR:parseFloat(parts[1]),BL:parseFloat(parts[2]),BR:parseFloat(parts[3])};");
  html += F("if(isNaN(ec.FL))continue;");
  html += F("if(!allData[rfid])allData[rfid]={};");
  html += F("if(!allData[rfid][dateStr]){");
  html += F("allData[rfid][dateStr]={FL:ec.FL,FR:ec.FR,BL:ec.BL,BR:ec.BR,n: 1};");
  html += F("}else{");
  html += F("var d=allData[rfid][dateStr];");
  html += F("d.FL=(d.FL*d.n+ec.FL)/(d.n+1);");
  html += F("d.FR=(d.FR*d.n+ec.FR)/(d.n+1);");
  html += F("d.BL=(d.BL*d.n+ec.BL)/(d.n+1);");
  html += F("d.BR=(d.BR*d.n+ec.BR)/(d.n+1);");
  html += F("d.n++;}}}");
  
  html += F("function sortDates(arr){");
  html += F("return arr.sort(function(a,b){");
  html += F("var pa=a.split('/'),pb=b.split('/');");
  html += F("return new Date(pa[2],pa[1]-1,pa[0])-new Date(pb[2],pb[1]-1,pb[0]);");
  html += F("});}");
  
  html += F("function buildDropdown(){");
  html += F("var sel=document.getElementById('rfidSelect');");
  html += F("sel.innerHTML='';");
  html += F("var keys=Object.keys(allData).sort();");
  html += F("if(!keys.length){");
  html += F("sel.innerHTML='<option>No data</option>';");
  html += F("setStatus('No test data found');return;}");
  html += F("keys.forEach(function(k){");
  html += F("var o=document.createElement('option');");
  html += F("o.value=k;o.textContent=k;sel.appendChild(o);});");
  html += F("sel.onchange=function(){renderChart(this.value);};");
  html += F("setStatus('');");
  html += F("renderChart(keys[0]);}");
  
  html += F("function renderChart(rfid){");
  html += F("var data=allData[rfid];");
  html += F("if(!data)return;");
  html += F("var dates=sortDates(Object.keys(data)).slice(-7);");
  html += F("if(!dates.length)return;");
  html += F("document.getElementById('dateRange').textContent=dates[0]+' - '+dates[dates.length-1];");
  html += F("var datasets=[];");
  html += F("['FL','FR','BL','BR'].forEach(function(q){");
  html += F("datasets.push({");
  html += F("label: q,");
  html += F("data: dates.map(function(d){return data[d]? parseFloat(data[d][q].toFixed(2)):null;}),");
  html += F("borderColor: lineColors[q],");
  html += F("backgroundColor: lineColors[q],");
  html += F("borderWidth: 3,");
  html += F("pointRadius:6,");
  html += F("pointHoverRadius:9,");
  html += F("pointBackgroundColor:lineColors[q],");
  html += F("tension:0.4,");
  html += F("fill: false");
  html += F("});});");
  html += F("if(chart){chart.destroy();chart=null;}");
  html += F("var ctx=document.getElementById('trendChart').getContext('2d');");
  html += F("chart=new Chart(ctx,{");
  html += F("type:'line',");
  html += F("data:{labels:dates,datasets: datasets},");
  html += F("options:{");
  html += F("responsive:true,");
  html += F("maintainAspectRatio: false,");
  html += F("interaction:{intersect:false,mode:'index'},");
  html += F("plugins:{");
  html += F("legend:{position:'bottom',labels:{color:'#fff',usePointStyle:false,boxWidth:20,padding:20,font:{size:13}}},");
  html += F("tooltip:{backgroundColor:'rgba(0,0,0,0.8)',titleColor:'#fff',bodyColor:'#fff',");
  html += F("callbacks:{label:function(ctx){");
  html += F("var v=ctx.parsed.y;");
  html += F("var status=v<=5.5? 'Normal': v<=7.4?'Warning':'Critical';");
  html += F("return ctx.dataset.label+': '+v.toFixed(2)+' ('+status+')';");
  html += F("}}}},");
  html += F("scales:{");
  html += F("y:{min:0,max: 12,");
  html += F("ticks:{color:'#aaa',stepSize:2,font:{size:12}},");
  html += F("grid:{color:'rgba(255,255,255,0.1)',drawBorder:false}},");
  html += F("x:{ticks:{color:'#aaa',font:{size:11},maxRotation:0},");
  html += F("grid:{color:'rgba(255,255,255,0.05)',drawBorder:false}}");
  html += F("}},");
  html += F("plugins:[severityPlugin]");
  html += F("});}");
  
  html += F("(function(){");
  html += F("setStatus('Loading files...');");
  html += F("fetch('/list-files')");
  html += F(".then(function(r){if(!r.ok)throw new Error(r.status);return r.json();})");
  html += F(".then(function(files){");
  html += F("if(!files||!files.length){setStatus('No files found');return Promise.resolve();}");
  html += F("setStatus('Loading '+files.length+' file(s)...');");
  html += F("var promises=files.map(function(f){");
  html += F("return fetch('/filecontent?name='+encodeURIComponent(f))");
  html += F(".then(function(r){return r.ok?r.text():null;})");
  html += F(".then(function(txt){if(txt)parseFile(txt,f);})");
  html += F(".catch(function(){});");
  html += F("});");
  html += F("return Promise.all(promises);})");
  html += F(".then(function(){buildDropdown();})");
  html += F(".catch(function(e){setStatus('Error: '+e);});");
  html += F("})();");
  
  html += F("</script></body></html>");
  
  server.send(200, "text/html; charset=UTF-8", html);
}

// ============================================================
// SPIFFS DIRECTORY PAGE WITH MODAL
// ============================================================

void SPIFFS_dir(){
  if (!SPIFFS_present) { ReportSPIFFSNotPresent(); return; }
  
  File root = SPIFFS.open("/");
  if (!root) { ReportFileNotPresent("dir"); return; }
  
  SendHTML_Header();
  
  // Page title and buttons
  webpage += F("<h2>SPIFFS File Manager</h2>");
  webpage += F("<a href='/filedownloadall'><button class='btn-main'>Download All</button></a>");
  webpage += F("<a href='/filedeleteall' onclick='return confirm(\"Delete ALL files? \")'><button class='btn-main btn-danger'>Delete All</button></a>");
  webpage += F("<a href='/trend'><button class='btn-main' style='background:#2196F3'>7-Day Trend</button></a>");
  webpage += F("<a href='/wifi-info'><button class='btn-main' style='background:#FF9800'>WiFi Settings</button></a>");
  
  // File table
  webpage += F("<table><tr><th>Filename</th><th>Size</th><th>Actions</th></tr>");
  
  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      String fname = String(file.name());
      if (fname.startsWith("/")) fname = fname.substring(1);
      
      webpage += "<tr><td>" + fname + "</td>";
      webpage += "<td>" + file_size(file.size()) + "</td>";
      webpage += "<td>";
      webpage += "<button class='btn-visualize' onclick=\"openModal('" + fname + "')\">Visualize</button>";
      webpage += "<a href='/filedownload?name=" + fname + "'><button class='btn'>Download</button></a> ";
      webpage += "<a href='/filedelete?name=" + fname + "' onclick='return confirm(\"Delete this file? \")'><button class='btn btn-danger'>Delete</button></a> ";
      webpage += "</td></tr>";
    }
    file = root.openNextFile();
  }
  webpage += F("</table>");
  root.close();
  
  // Modal styles
  webpage += F("<style>");
  // The container (dark overlay)
  webpage += F("#modal{display:none;position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,0.6);z-index:9999;justify-content:center;align-items:center;}");
  
  // NOTE: The space after #modal is CRITICAL in the lines below
  webpage += F("#modal .box{background:#fff;padding:25px 30px;border-radius:8px;width:95%;max-width:520px;position:relative;box-shadow:0 5px 30px rgba(0,0,0,0.3);}");
  webpage += F("#modal .close{position:absolute;right:15px;top:12px;font-size:24px;color:#333;cursor:pointer;font-weight:bold;}");
  webpage += F("#modal .close:hover{color:#000;}");
  webpage += F("#modal .title{text-align:center;font-size:20px;font-weight:bold;color:#333;margin-bottom:15px;}");
  webpage += F("#modal .select-row{text-align:center;margin-bottom:20px;}");
  webpage += F("#modal .select-row select{padding:5px 15px;font-size:14px;border: 1px solid #999;border-radius:4px;}");
  webpage += F("#modal .info{display:flex;justify-content:space-between;margin-bottom:15px;font-size:13px;color:#333;padding:0 10px;}");
  
  // Navigation container
  webpage += F("#modal .nav-container{display:flex;justify-content:center;align-items:center;gap:15px;margin-bottom:15px;}");
  
  // Arrow buttons
  webpage += F("#modal .arrow-btn{width:50px;height:80px;background:#1976d2;border:none;border-radius:8px;cursor:pointer;display:flex;align-items:center;justify-content:center;}");
  webpage += F("#modal .arrow-btn:hover{background:#1565c0;}");
  webpage += F("#modal .arrow-btn.disabled{background:#ccc;cursor:not-allowed;}");
  webpage += F("#modal .arrow-btn svg{width:24px;height: 24px;fill:#fff;}");
  
  // Circle visualization
  webpage += F("#modal .circle{width:200px;height:200px;border-radius:50%;margin:0;position:relative;overflow:hidden;border:3px solid #333333;}");
  webpage += F("#modal .quad{position:absolute;width:50%;height:50%;display:flex;align-items:center;justify-content:center;font-size:22px;font-weight:bold;color:#fff;}");
  webpage += F("#modal .q-fl{top:0;left:0;}");
  webpage += F("#modal .q-fr{top:0;left:50%;}");
  webpage += F("#modal .q-bl{top:50%;left:0;}");
  webpage += F("#modal .q-br{top:50%;left: 50%;}");
  webpage += F("#modal .green{background:#4CAF50;}");
  webpage += F("#modal .yellow{background:#FFA726;}");
  webpage += F("#modal .red{background:#EF5350;}");
  webpage += F("#modal .div-v{position:absolute;left:50%;top:0;width:2px;height:100%;background:#333333;transform:translateX(-50%);}");
  webpage += F("#modal .div-h{position:absolute;top:50%;left:0;height:2px;width:100%;background:#333333;transform:translateY(-50%);}");
  
  // Counter and Legend
  webpage += F("#modal .test-counter{text-align:center;font-size:14px;color:#666;margin-bottom:15px;font-weight:500;}");
  webpage += F("#modal .legend{text-align:center;font-size:14px;}");
  webpage += F("#modal .legend span{margin:0 15px;}");
  webpage += F("#modal .leg-g{color:#4CAF50;}");
  webpage += F("#modal .leg-y{color:#FFA726;}");
  webpage += F("#modal .leg-r{color:#EF5350;}");
  webpage += F("</style>");


  // Modal HTML - ORIGINAL structure with arrow buttons added
  webpage += F("<div id='modal'>");
  webpage += F("<div class='box'>");
  webpage += F("<span class='close' onclick='closeModal()'>&times;</span>");
  webpage += F("<div class='title'>Test Visualization</div>");
  webpage += F("<div class='select-row'><b>Select Test:</b> <select id='testSelect' onchange='showTest(this.value)'></select></div>");
  webpage += F("<div class='info'><span id='infoDate'>Date: --</span><span id='infoTime'>Time: --</span><span id='infoRfid'>RFID: --</span></div>");
  
  // NEW: Navigation container with arrows
  webpage += F("<div class='nav-container'>");
  // Previous Arrow Button
  webpage += F("<button class='arrow-btn' id='prevBtn' onclick='prevTest()' title='Previous Test'>");
  webpage += F("<svg viewBox='0 0 24 24'><path d='M15.41 7.41L14 6l-6 6 6 6 1.41-1.41L10.83 12z'/></svg>");
  webpage += F("</button>");
  
  // ORIGINAL Circle - preserved exactly
  webpage += F("<div class='circle'>");
  webpage += F("<div id='qFL' class='quad q-fl green'>FL</div>");
  webpage += F("<div id='qFR' class='quad q-fr green'>FR</div>");
  webpage += F("<div id='qBL' class='quad q-bl green'>BL</div>");
  webpage += F("<div id='qBR' class='quad q-br green'>BR</div>");
  webpage += F("<div class='div-v'></div><div class='div-h'></div>");
  webpage += F("</div>");
  
  // Next Arrow Button
  webpage += F("<button class='arrow-btn' id='nextBtn' onclick='nextTest()' title='Next Test'>");
  webpage += F("<svg viewBox='0 0 24 24'><path d='M8.59 16.59L10 18l6-6-6-6-1.41 1.41L13.17 12z'/></svg>");
  webpage += F("</button>");
  webpage += F("</div>"); // End nav-container
  
  // NEW: Test counter
  webpage += F("<div class='test-counter' id='testCounter'>-- of --</div>");
  
  // ORIGINAL Legend - preserved exactly
  webpage += F("<div class='legend'><span class='leg-g'>&#9679; Normal</span><span class='leg-y'>&#9679; Sub-Clinical</span><span class='leg-r'>&#9679; Clinical</span></div>");
  webpage += F("</div></div>");
  
  // JavaScript - ORIGINAL functions preserved + new arrow functions added
  webpage += F("<script>");
  webpage += F("var tests=[];var currentFile='';var currentTestIndex=0;");
  
  // ORIGINAL openModal
  webpage += F("function openModal(fname){currentFile=fname;fetch('/filecontent?name='+encodeURIComponent(fname)).then(function(r){return r.text();}).then(function(txt){parseTests(txt);fillDropdown();if(tests.length>0){currentTestIndex=0;showTest(0);}updateArrowButtons();document.getElementById('modal').style.display='flex';}).catch(function(e){alert('Error: '+e);});}");
  
  // ORIGINAL closeModal
  webpage += F("function closeModal(){document.getElementById('modal').style.display='none';}");
  
  // ORIGINAL click outside to close
  webpage += F("document.getElementById('modal').onclick=function(e){if(e.target.id==='modal')closeModal();};");
  
  // ORIGINAL parseTests
  webpage += F("function parseTests(txt){tests=[];var parts=txt.split('File Name: ');for(var i=1;i<parts.length;i++){tests.push(parts[i].trim().split('\\n'));}}");
  
  // ORIGINAL fillDropdown
  webpage += F("function fillDropdown(){var sel=document.getElementById('testSelect');sel.innerHTML='';for(var i=0;i<tests.length;i++){var o=document.createElement('option');o.value=i;o.text='Test '+(i+1);sel.appendChild(o);}}");
  
  // ORIGINAL getVal
  webpage += F("function getVal(lines,key){for(var i=0;i<lines.length;i++){if(lines[i].indexOf(key)!==-1){var p=lines[i].indexOf(': ');if(p>-1)return lines[i].substring(p+1).trim();}}return '';}");
  
  // ORIGINAL fmtDate
  webpage += F("function fmtDate(fn){var s=fn.replace('.txt','').replace(/\\D/g,'');if(s.length>=8)return s.slice(0,2)+'.'+s.slice(2,4)+'.'+s.slice(4,8);return fn;}");
  
  // ORIGINAL setQ
  webpage += F("function setQ(id,c){document.getElementById(id).className='quad q-'+id.slice(1).toLowerCase()+' '+c;}");
  
  // NEW: Update arrow buttons state
  webpage += F("function updateArrowButtons(){");
  webpage += F("var prevBtn=document.getElementById('prevBtn');");
  webpage += F("var nextBtn=document.getElementById('nextBtn');");
  webpage += F("if(currentTestIndex<=0){prevBtn.classList.add('disabled');}else{prevBtn.classList.remove('disabled');}");
  webpage += F("if(currentTestIndex>=tests.length-1){nextBtn.classList.add('disabled');}else{nextBtn.classList.remove('disabled');}");
  webpage += F("}");
  
  // NEW: Update test counter
  webpage += F("function updateTestCounter(){");
  webpage += F("var counter=document.getElementById('testCounter');");
  webpage += F("if(tests.length>0){counter.textContent=(currentTestIndex+1)+' of '+tests.length;}");
  webpage += F("else{counter.textContent='-- of --';}");
  webpage += F("}");
  
  // NEW: Previous test
  webpage += F("function prevTest(){if(currentTestIndex>0){currentTestIndex--;document.getElementById('testSelect').value=currentTestIndex;showTest(currentTestIndex);}}");
  
  // NEW: Next test
  webpage += F("function nextTest(){if(currentTestIndex<tests.length-1){currentTestIndex++;document.getElementById('testSelect').value=currentTestIndex;showTest(currentTestIndex);}}");
  
  // ORIGINAL showTest
  webpage += F("function showTest(i){i=parseInt(i);currentTestIndex=i;var L=tests[i];if(!L)return;document.getElementById('infoDate').textContent='Date: '+fmtDate(currentFile);document.getElementById('infoTime').textContent='Time: '+getVal(L,'Time');var rfid=getVal(L,'RFID No');document.getElementById('infoRfid').textContent='RFID: '+(rfid||'--');setQ('qFL','green');setQ('qFR','green');setQ('qBL','green');setQ('qBR','green');var clin=getVal(L,'Clinical').toUpperCase();var sub=getVal(L,'Sub Clinical').toUpperCase();if(sub.indexOf('FL')>-1)setQ('qFL','yellow');if(sub.indexOf('FR')>-1)setQ('qFR','yellow');if(sub.indexOf('BL')>-1)setQ('qBL','yellow');if(sub.indexOf('BR')>-1)setQ('qBR','yellow');if(clin.indexOf('FL')>-1)setQ('qFL','red');if(clin.indexOf('FR')>-1)setQ('qFR','red');if(clin.indexOf('BL')>-1)setQ('qBL','red');if(clin.indexOf('BR')>-1)setQ('qBR','red');updateArrowButtons();updateTestCounter();}");
  
  // NEW: Keyboard navigation
  webpage += F("document.addEventListener('keydown',function(e){if(document.getElementById('modal').style.display==='flex'){if(e.key==='ArrowLeft')prevTest();else if(e.key==='ArrowRight')nextTest();else if(e.key==='Escape')closeModal();}});");
  
  webpage += F("</script>");
  
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

// ============================================================
// FILE ACTIONS
// ============================================================

void File_Action(String action_type) {
  if (!SPIFFS_present) { ReportSPIFFSNotPresent(); return; }
  
  if (action_type == "download") {
    if (server.hasArg("name")) {
      String filename = server.arg("name");
      if (!filename.startsWith("/")) filename = "/" + filename;
      File f = SPIFFS.open(filename, "r");
      if (f) {
        server.sendHeader("Content-Disposition", "attachment; filename=" + filename.substring(1));
        server.streamFile(f, "application/octet-stream");
        f.close();
      } else {
        ReportFileNotPresent(filename);
      }
    }
  } 
  else if (action_type == "delete") {
    if (server.hasArg("name")) {
      String filename = server.arg("name");
      if (!filename.startsWith("/")) filename = "/" + filename;
      SPIFFS.remove(filename);
      server.sendHeader("Location", "/dir");
      server.send(302, "text/plain", "");
    }
  } 
    // ============================================================
  // DELETE ALL FILES - FIXED IMPLEMENTATION
  // ============================================================
  else if (action_type == "deleteall") {
    Serial.println("\n=== DELETE ALL FILES ===");
    
    // First, collect all filenames into an array
    String filesToDelete[100];  // Max 100 files
    int fileCount = 0;
    
    File root = SPIFFS.open("/");
    if (root) {
      File f = root.openNextFile();
      while (f && fileCount < 100) {
        if (!f.isDirectory()) {
          filesToDelete[fileCount] = String(f.name());
          fileCount++;
        }
        f = root.openNextFile();
      }
      root.close();
    }
    
    Serial.println("Found " + String(fileCount) + " files to delete");
    
    // Now delete all collected files
    int deletedCount = 0;
    int failedCount = 0;
    
    for (int i = 0; i < fileCount; i++) {
      String path = filesToDelete[i];
      if (!path.startsWith("/")) path = "/" + path;
      
      if (SPIFFS.remove(path)) {
        Serial.println("✅ Deleted: " + path);
        deletedCount++;
      } else {
        Serial.println("❌ Failed to delete: " + path);
        failedCount++;
      }
    }
    
    Serial.println("=== DELETE COMPLETE ===");
    Serial.println("Deleted: " + String(deletedCount) + " files");
    Serial.println("Failed: " + String(failedCount) + " files");
    
    // Redirect back to directory
    server.sendHeader("Location", "/dir");
    server.send(302, "text/plain", "");
  } 
  // ============================================================
  // DOWNLOAD ALL FILES - NEW IMPLEMENTATION
  // ============================================================
  else if (action_type == "downloadall") {
    Serial.println("\n=== DOWNLOAD ALL FILES ===");
    
    // Build combined content from all files
    String allContent = "";
    String separator = "\n============================================================\n";
    
    File root = SPIFFS.open("/");
    if (!root) {
      ReportFileNotPresent("root");
      return;
    }
        // Count files and build content
    int fileCount = 0;
    size_t totalSize = 0;
    
    File file = root.openNextFile();
    while (file) {
      if (!file.isDirectory()) {
        String fname = String(file.name());
        if (fname.startsWith("/")) fname = fname.substring(1);
        
        // Add file header
        allContent += separator;
        allContent += "FILE: " + fname + "\n";
        allContent += "SIZE: " + file_size(file.size()) + "\n";
        allContent += separator;
        
        // Read and add file content
        while (file.available()) {
          allContent += (char)file.read();
        }
        allContent += "\n";
        
        totalSize += file.size();
        fileCount++;
        
        Serial.println("Added file: " + fname);
      }
      file = root.openNextFile();
    }
    root.close();
    
    // Check if any files found
    if (fileCount == 0) {
      SendHTML_Header();
      webpage += F("<h3>No files to download</h3>");
      webpage += F("<p>The SPIFFS storage is empty.</p>");
      webpage += F("<a href='/dir'><button class='btn-main'>Back to File Manager</button></a>");
      append_page_footer();
      SendHTML_Content();
      SendHTML_Stop();
      return;
    }
    
    // Build header for the combined file
    String header = "";
    header += "================================================================\n";
    header += "            QUADMASTEST DATA EXPORT\n";
    header += "================================================================\n";
    header += "Device ID: " + String(uniqueChipID) + "\n";
    header += "Total Files: " + String(fileCount) + "\n";
    header += "Total Data Size: " + file_size(totalSize) + "\n";
    header += "Export Date: " + String(__DATE__) + " " + String(__TIME__) + "\n";
    header += "================================================================\n";
    
    // Prepend header to content
    allContent = header + allContent;
    
    // Add footer
    allContent += "\n";
    allContent += separator;
    allContent += "                    END OF EXPORT\n";
    allContent += separator;
    
    Serial.println("Total files: " + String(fileCount));
    Serial.println("Combined size: " + String(allContent.length()) + " bytes");
    
    // Generate filename with device ID
    String exportFilename = "QMT_" + String(uniqueChipID) + "_AllData.txt";
    
    // Send as downloadable file
    server.sendHeader("Content-Disposition", "attachment; filename=" + exportFilename);
    server.sendHeader("Content-Type", "text/plain");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(200, "text/plain; charset=UTF-8", allContent);
    
    Serial.println("=== DOWNLOAD SENT ===");
  }
}

// ============================================================
// IMPROVED WIFI SETUP - WITH AUTO-RECONNECT AND CAPTIVE PORTAL
// ============================================================

String Wifisetup(uint32_t chipID) {
  Serial.println("\n=== WiFi Setup ===");
  
  // DON'T reset settings - this preserves saved credentials!  
  // wm.resetSettings();  // ← REMOVED!  
  
  // Configure WiFiManager
  wifiManager.setConfigPortalTimeout(180);  // 3 minute timeout
  wifiManager.setConnectTimeout(30);        // 30 seconds to connect
  
  // Set custom AP name
  char apName[25];
  snprintf(apName, sizeof(apName), "QMT_%lu", (unsigned long)chipID);
  
  Serial.print("Attempting to connect to saved WiFi...");
  
  // autoConnect will:  
  // 1. Try to connect with saved credentials
  // 2. If that fails, start AP mode with config portal
  // 3. Return true when connected, false on timeout
  
  bool connected = wifiManager.autoConnect(apName, "12345678");
  
  if (!connected) {
    Serial.println("\nFailed to connect. Restarting...");
    delay(3000);
    ESP.restart();
    return "0.0.0.0";
  }
  
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  
  addIP = WiFi.localIP().toString();
  SPIFFS_present = SPIFFS.begin(true);
  
  // Register all routes
  server.on("/", SPIFFS_dir);
  server.on("/dir", SPIFFS_dir);
  server.on("/trend", Trend_Page);
  server.on("/list-files", List_Files_JSON);
  server.on("/filecontent", Get_File_Content);
  server.on("/filedownload", [](){ File_Action("download"); });
  server.on("/filedelete", [](){ File_Action("delete"); });
  server.on("/filedownloadall", [](){ File_Action("downloadall"); });
  server.on("/filedeleteall", [](){ File_Action("deleteall"); });
  
  // WiFi management routes
  server.on("/wifi-info", WiFi_Info_Page);
  server.on("/wifi-reset", WiFi_Reset_Page);
  server.on("/wifi-reset-confirm", HTTP_POST, WiFi_Reset_Confirm);
  
  // ============================================================
  // NEW:  CAPTIVE PORTAL DETECTION ROUTES - AUTO REDIRECT
  // These URLs are checked by devices to detect captive portals
  // When detected, browser automatically opens the page
  // ============================================================
  
  // Android captive portal detection
  server.on("/generate_204", handleCaptivePortal);
  server.on("/gen_204", handleCaptivePortal);
  
  // Apple/iOS captive portal detection
  server.on("/hotspot-detect.html", handleHotspotDetect);
  server.on("/library/test/success.html", handleHotspotDetect);
  server.on("/success.txt", handleHotspotDetect);
  
  // Windows captive portal detection
  server.on("/ncsi.txt", handleCaptivePortal);
  server.on("/connecttest.txt", handleCaptivePortal);
  server.on("/redirect", handleCaptivePortal);
  
  // Firefox captive portal detection
  server.on("/canonical.html", handleCaptivePortal);
  
  // Generic fallback
  server.on("/fwlink", handleCaptivePortal);
  
  // Handle all other unknown requests - redirect to /dir
  server.onNotFound(handleNotFound);
  
  server.begin();
  
  // ============================================================
  // NEW: Print connection info for user
  // ============================================================
  Serial.println("\n========================================");
  Serial.println("   WEB SERVER READY!");
  Serial.println("========================================");
  Serial.println("Open browser and go to:");
  Serial.println("   http://" + addIP);
  Serial.println("   http://" + addIP + "/dir");
  Serial.println("");
  Serial.println("Captive Portal routes registered.");
  Serial.println("Page should auto-popup on some devices.");
  Serial.println("========================================\n");
  
  return addIP;
}