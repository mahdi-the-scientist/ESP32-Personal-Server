#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED সেটিংস
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int ledPin = 2;
unsigned long previousMillis = 0;
const char* ssid = "Personal Server";
const char* password = "mahdi@1234";

AsyncWebServer server(80);

// সুন্দর ওয়েব ইন্টারফেসের HTML (CSS সহ)
String getIndexHTML() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css'>";
  html += "<style>body{font-family:sans-serif;background:#f4f4f9;padding:20px;}";
  html += ".container{max-width:600px;margin:auto;background:white;padding:20px;border-radius:10px;box-shadow:0 4px 8px rgba(0,0,0,0.1);}";
  html += ".header{display:flex;justify-content:space-between;align-items:center;border-bottom:2px solid #eee;padding-bottom:10px;}";
  html += ".file-list{margin-top:20px;}.file-item{display:flex;justify-content:space-between;background:#f9f9f9;padding:10px;margin-bottom:5px;border-radius:5px;}";
  html += ".btn-upload{background:#007bff;color:white;padding:10px;border-radius:50%;width:40px;height:40px;display:flex;justify-content:center;align-items:center;cursor:pointer;text-decoration:none;}";
  html += ".download-link{color:#28a745;text-decoration:none;font-weight:bold;}</style></head><body>";
  html += "<div class='container'><div class='header'><h2><i class='fas fa-hdd'></i> My Storage</h2>";
  html += "<label for='file-input' class='btn-upload'><i class='fas fa-plus'></i></label></div>";
  html += "<form id='upload-form' method='POST' action='/upload' enctype='multipart/form-data' style='display:none;'>";
  html += "<input id='file-input' type='file' name='upload' onchange='document.getElementById(\"upload-form\").submit()'></form>";
  html += "<div class='file-list'>";

  // এসডি কার্ডের সব ফাইল লিস্ট করা
  File root = SD.open("/");
  File file = root.openNextFile();
  while(file) {
    String fileName = String(file.name());
    html += "<div class='file-item'><span><i class='far fa-file'></i> " + fileName + "</span>";
    html += "<a href='/download?file=" + fileName + "' class='download-link'><i class='fas fa-download'></i></a></div>";
    file = root.openNextFile();
  }
  
  html += "</div></div></body></html>";
  return html;
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) for(;;);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("System Booting...");
  display.display();

  if(!SD.begin(5)) {
    display.println("SD Error!");
  } else {
    display.println("SD Connected.");
  }
  display.display();

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("SERVER LIVE");
  display.print("IP: "); display.println(IP);
  display.display();

  // হোম পেজ (ফাইল লিস্ট)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", getIndexHTML());
  });

  // ফাইল ডাউনলোড সিস্টেম
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
    if(request->hasParam("file")){
      String fileName = "/" + request->getParam("file")->value();
      request->send(SD, fileName, "application/octet-stream");
    } else {
      request->send(400, "text/plain", "File not specified");
    }
  });

  // ফাইল আপলোড সিস্টেম (+) বাটন
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
    request->redirect("/");
  }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      request->_tempFile = SD.open("/" + filename, FILE_WRITE);
    }
    if(request->_tempFile && len) request->_tempFile.write(data, len);
    if(final && request->_tempFile) request->_tempFile.close();
  });

  server.begin();
}

void loop() {
  if (millis() - previousMillis >= 1000) {
    previousMillis = millis();
    digitalWrite(ledPin, !digitalRead(ledPin)); // LED Blink
  }
}