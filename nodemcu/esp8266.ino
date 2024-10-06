#include <WiFiClient.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>
#include <Wire.h>

#ifndef APSSID
#define APSSID "ESPap"
#define APPSK "thereisnospoon"
#endif

const char *ssid = APSSID;
const char *password = APPSK;

#define seaLevelPressure_hPa 1013.25

#define DHTPIN 13 // Digital pin connected to the DHT sensor

// Uncomment the type of sensor in use:
#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE DHT22 // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

// current temperature & humidity, updated in loop()
double t = 0.0;
double h = 0.0;

double P, p0, a;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

Adafruit_BMP085 bmp;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0; // will store last time DHT was updated

// Updates DHT readings every 10 seconds
const long interval = 6;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Grupa 8.</title>
</head>
<body>
  <h2>ESP8266 DHT Server</h2>
  <div><p>TEMP: </p><p id="temperature">%TEMPERATURE%</p></div>
  <div><p>HUM: </p><p id="humidity">%HUMIDITY%</p></div>
  <div><p>ALT: </p><p id="altitude">%ALTITUDE%</p></div>
  <div><p>PRES: </p><p id="pressure">%PRESSURE%</p></div>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 200 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 200 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("altitude").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/altitude", true);
  xhttp.send();
}, 200 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pressure").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pressure", true);
  xhttp.send();
}, 200 ) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String &var)
{
  if (var == "TEMPERATURE")
  {
    return String(t);
  }
  else if (var == "HUMIDITY")
  {
    return String(h);
  }
  else if (var == "ALTITUDE")
  {
    return String(a);
  }
  else if (var == "PRESSURE")
  {
    return String(P);
  }
  return String();
}

void setup()
{
  Serial.begin(300);

  Serial.printf("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  dht.begin();

  if (bmp.begin())
    Serial.printf("BMP180 init success");
  else
  {
    // Oops, something went wrong, this is usually a connection problem,
    // see the comments at the top of this sketch for the proper connections.

    Serial.printf("BMP180 init fail\n\n");
    while (1)
      ; // Pause forever.
  }

  // ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html, processor); });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(t).c_str()); });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(h).c_str()); });
  server.on("/altitude", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(a).c_str()); });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", String(P).c_str()); });

  // Start server
  server.begin();
}

void loop()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)

    t = bmp.readTemperature();
    Serial.printf("Temperature = %.2lf *C", t);

    P = bmp.readSealevelPressure();

    Serial.printf("Pressure (at sealevel, calculated) = %.2lf Pa", P);

    a = bmp.readAltitude(seaLevelPressure_hPa * 100);

    Serial.printf("Real altitude = %.2lf m", a);

    // Read Humidity

    double newH = dht.readHumidity();
    // if humidity read failed, don't change h value
    if (isnan(newH))
    {
      Serial.printf("Failed to read from DHT sensor!");
    }
    else
    {
      h = newH;
      Serial.printf("Humidity: %.2lf %", h);
    };

    Serial.println();
  }
}
