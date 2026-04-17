#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int ledPin = 2;
unsigned long previousMillis = 0;
const char* ssid = "Personal Server";
const char* password = "mahdi@1234";

AsyncWebServer server(80);

String getIndexHTML() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css'>";
  html += "<style>body{font-family:sans-serif;background:#f4f4f9;padding:20px; color:#333;}";
  html += ".container{max-width:600px;margin:auto;background:white;padding:25px;border-radius:15px;box-shadow:0 8px 16px rgba(0,0,0,0.1);}";
  html += ".header{display:flex;justify-content:space-between;align-items:center;border-bottom:2px solid #eee;padding-bottom:15px;margin-bottom:20px;}";
  html += ".file-item{display:flex;justify-content:space-between;align-items:center;background:#fff;padding:12px 15px;margin-bottom:10px;border-radius:8px;border:1px solid #eee;}";
  html += ".btn-plus{background:#007bff;color:white;padding:12px;border-radius:50%;width:45px;height:45px;display:flex;justify-content:center;align-items:center;cursor:pointer;}";
  html += ".download-btn{color:#28a745;text-decoration:none;}</style></head><body>";
  html += "<div class='container'><div class='header'><h2>Explorer</h2>";
  html += "<label for='f-up' class='btn-plus'><i class='fas fa-plus'></i></label></div>";
  html += "<form id='u-form' method='POST' action='/upload' enctype='multipart/form-data' style='display:none;'>";
  html += "<input id='f-up' type='file' name='upload' onchange='document.getElementById(\"u-form\").submit()'></form>";
  html += "<div class='file-list'>";

  File root = SD.open("/");
  File file = root.openNextFile();
  if(!file) html += "<p>No files found</p>";
  while(file) {
    String fileName = String(file.name());
    html += "<div class='file-item'><span>" + fileName + "</span>";
    html += "<a href='/download?file=" + fileName + "' class='download-btn'><i class='fas fa-download'></i></a></div>";
    file = root.openNextFile();
  }
  html += "</div></div></body></html>";
  return html;
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  if(!SD.begin(5)) Serial.println("SD Fail");

  WiFi.softAP(ssid, password);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", getIndexHTML());
  });

  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
    if(request->hasParam("file")){
      String f = "/" + request->getParam("file")->value();
      request->send(SD, f, "application/octet-stream");
    }
  });

  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
    request->redirect("/");
  }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index) request->_tempFile = SD.open("/" + filename, FILE_WRITE);
    if(request->_tempFile && len) request->_tempFile.write(data, len);
    if(final && request->_tempFile) request->_tempFile.close();
  });

  server.begin();
}

void loop() {
  if (millis() - previousMillis >= 1000) {
    previousMillis = millis();
    digitalWrite(ledPin, !digitalRead(ledPin));
  }
}
