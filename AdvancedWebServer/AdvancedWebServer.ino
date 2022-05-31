#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DHT.h>

#ifndef STASSID
#define STASSID "info-licence-a"
#define STAPSK  "pm01web02pm03web04"
#endif

#define DHTPIN 14
#define DHTTYPE DHT11

const char *ssid = STASSID;
const char *password = STAPSK;

IPAddress local_IP(192,168,4,22);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);

DHT dht(DHTPIN, DHTTYPE);

ESP8266WebServer server(80);

void handleRoot() {
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
  </body>\
</html>",

           hr, min % 60, sec % 60
          );
  server.send(200, "text/html", temp);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

void getTemperature() {
  float temperature = dht.readTemperature();
  char textResponse[400];

  if (isnan(temperature)) {
    Serial.println("Erreur de lecture de la temperature !");
  }

  snprintf(textResponse, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Temperature: %f3</p>\
  </body>\
</html>",

           temperature
          );

  server.send(200, "text/html", textResponse);
  
}

void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  dht.begin();

  // Wait for connection
  int iteration = 0;
  while (WiFi.status() != WL_CONNECTED && iteration <= 10) {
    delay(1000);
    Serial.print(".");
    iteration++;
  }

  if (iteration > 10) {
    //mode AP
    WiFi.softAPConfig(local_IP, gateway, subnet);
    boolean result = WiFi.softAP("AP_esp1", "12345678");
    if (result) { Serial.println("Ready"); } else { Serial.println("Error AP");}
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/temperature", getTemperature); 
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
