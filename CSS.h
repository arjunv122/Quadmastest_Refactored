void append_page_header() {
  webpage = F("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
  webpage += F("<title>QMT File Manager</title>");
  webpage += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
  webpage += F("<style>");
  webpage += F("body{font-family: Arial,sans-serif;background: linear-gradient(135deg,#e0f7e9,#a8e6cf);margin:0;padding:0;text-align:center;color:#333;}");
  webpage += F(".header{background:#fff;padding:20px;border-bottom:3px solid #4CAF50;}");
  webpage += F("h1.header-title{color:#4CAF50;margin: 0;font-size:24px;}");
  webpage += F("h2{color:#4CAF50;margin:20px;}");
  webpage += F(".btn-main{margin:10px;padding:12px 24px;background:#4CAF50;color:#fff;border:none;border-radius:8px;font-size:16px;cursor:pointer;text-decoration:none;display:inline-block;}");
  webpage += F(".btn-main:hover{background:#45a049;}");
  webpage += F(".btn-danger{background:#e53935! important;}");
  webpage += F("table{width:90%;margin:20px auto;border-collapse:collapse;background:#fff;}");
  webpage += F("th,td{border:1px solid #ccc;padding:12px;text-align:center;}");
  webpage += F("th{background:#4CAF50;color:#fff;}");
  webpage += F(".btn{padding:6px 14px;background:#4CAF50;color:#fff;border:none;border-radius: 5px;font-size:14px;cursor: pointer;margin:2px;}");
  webpage += F(".btn:hover{opacity: 0.9;}");
  webpage += F(".btn-visualize{padding:6px 14px;background:#2196F3;color:#fff;border:none;border-radius: 5px;font-size:14px;cursor: pointer;margin:2px;}");
  webpage += F(".btn-visualize:hover{opacity: 0.9;}");
  webpage += F("ul{list-style:none;margin:20px 0;padding:0;}");
  webpage += F("li{display:inline;margin:0 10px;}");
  webpage += F("li a{color:#4CAF50;text-decoration: none;}");
  webpage += F("footer{padding:10px;color:#666;font-size:12px;}");
  
  // MODAL - FIXED: display: none by default, shows as overlay
  webpage += F(".modal{display:none;position:fixed;z-index:9999;left:0;top: 0;width:100%;height: 100%;background-color: rgba(0,0,0,0.6);overflow:auto;}");
  webpage += F(".modal-content{background:#fff;margin:5% auto;padding:25px;border-radius:12px;width:90%;max-width:520px;position:relative;box-shadow: 0 5px 30px rgba(0,0,0,0.3);}");
  webpage += F(".close-btn{position:absolute;right:15px;top:10px;font-size:28px;cursor:pointer;color:#888;font-weight:bold;}");
  webpage += F(".close-btn:hover{color:#333;}");
  
  // Info row
  webpage += F(".info-row{display:flex;justify-content:space-around;margin: 15px 0;padding:12px;background:#f5f5f5;border-radius:6px;}");
  webpage += F(".info-row span{color:#c00;font-weight:600;font-size:14px;}");
  
  // Circle
  webpage += F(".circle{position:relative;width:280px;height:280px;border-radius:50%;margin:20px auto;border:3px solid #333333;overflow:hidden;}");
  webpage += F(".quad{position:absolute;width:50%;height:50%;display:flex;align-items:center;justify-content:center;font-weight:bold;color:#fff;font-size:22px;}");
  webpage += F(".fl{top: 0;left:0;}");
  webpage += F(".fr{top:0;left:50%;}");
  webpage += F(".bl{top:50%;left:0;}");
  webpage += F(".br{top:50%;left:50%;}");
  webpage += F(".greenCell{background:#4CAF50;}");
  webpage += F(".yellowCell{background:#FFC107;color:#333;}");
  webpage += F(".redCell{background:#F44336;}");
  webpage += F(".divider{position:absolute;background:#333333;z-index:5;}");
  webpage += F(".divider.vertical{width:3px;height:100%;left:50%;top:0;transform:translateX(-50%);}");
  webpage += F(".divider.horizontal{height:3px;width:100%;top: 50%;left:0;transform:translateY(-50%);}");
  
  // Legend
  webpage += F(".legend{margin-top:15px;text-align:center;}");
  webpage += F(".legend span{margin:0 12px;font-weight:bold;}");
  webpage += F(".leg-n{color:#4CAF50;}");
  webpage += F(".leg-s{color:#FFC107;}");
  webpage += F(".leg-c{color:#F44336;}");
  
  webpage += F("</style></head><body>");
  webpage += F("<div class='header'><h1 class='header-title'>QMT ");
  webpage += String(uniqueChipID);
  webpage += F("</h1></div>");
}

void append_page_footer(){ 
  webpage += F("<ul><li><a href='/'>Home</a></li><li><a href='/dir'>Directory</a></li></ul>");
  webpage += F("<footer>&copy; D.L. Bird 2018</footer>");
  webpage += F("</body></html>");
}