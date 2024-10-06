#include <LiquidCrystal.h>
#include <SPI.h>
#include <Ethernet.h>

enum request
{
  TEMPERATURE,
  HUMIDITY,
  ALTITUDE,
  PRESSURE
};

byte buffer[150];
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

int lcd4RS = A0,
    lcd6E = A1,
    lcd11D4 = A2,
    lcd12D5 = A3,
    lcd13D6 = A4,
    lcd14D7 = A5;

LiquidCrystal lcd(lcd4RS, lcd6E, lcd11D4, lcd12D5, lcd13D6, lcd14D7);

int potOut;

IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);

EthernetClient clnt;

enum request allnames[] = {TEMPERATURE, HUMIDITY, ALTITUDE, PRESSURE};
const char *urls[] = {"temperature", "humidity", "altitude", "pressure"};

unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;

bool printWebData = true;

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
      Serial.println("Ethernet shield was not found.Sorry, can't run without hardware. :(");
      while (true)
      {
        delay(1);
      }
    }
    if (Ethernet.linkStatus() == LinkOFF)
    {
      Serial.println("Ethernet cable is not connected.");
    }
    Ethernet.begin(mac, ip, myDns);
  }
  else
  {
    Serial.print("DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }

  delay(1000);

  beginMicros = micros();

  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  
  lcd.begin(16, 2);
}

void makeRequest(enum request name)
{
  int i;
  for (i = 0; i < sizeof(allnames) / sizeof(enum request); i++)
  {
    if (allnames[i] == name)
    {
      if (clnt.connect("192.168.4.1", 80))
      {
        Serial.println();
        Serial.println();
        Serial.print("connected to ");
        Serial.println(clnt.remoteIP());
        char fullurl[30];
        sprintf(fullurl, "GET /%s HTTP/1.1", urls[i]);
        clnt.println(fullurl);
        clnt.println();
      }
      else
      {
        Serial.println("connection failed");
      }
      delay(500);
      int len = clnt.available();
      if (len > 0)
      {
        if (len > 150)
          len = 150;
        clnt.read(buffer, len);
        if (printWebData)
        {
          Serial.write(buffer, len);
        }
        byteCount = byteCount + len;
      }
      if (!clnt.connected())
      {
        endMicros = micros();
        Serial.println();
        Serial.println("disconnecting.");
        clnt.stop();
        Serial.print("Received ");
        Serial.print(byteCount);
        Serial.print(" bytes in ");
        float seconds = (float)(endMicros - beginMicros) / 1000000.0;
        Serial.print(seconds, 4);
        float rate = (float)byteCount / seconds / 1000.0;
        Serial.print(", rate = ");
        Serial.print(rate);
        Serial.print(" kbytes/second");
        Serial.println();
        while (true)
        {
          delay(1);
        }
      }
    }
  }
}

void loop()
{
  for (;;)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TEMP (C):");
    lcd.setCursor(10, 0);
    makeRequest(TEMPERATURE);
    float num;
    char *buf;
    int q;
    buf = (char *)buffer;
    for (q = 0; buf[q]; q++)
      ;
    for (; buf[q] != '\n'; q--)
      ;
    ;
    q++;
    lcd.print(buf + q);
    lcd.setCursor(0, 1);
    lcd.print("HUM (%):");
    lcd.setCursor(9, 1);
    makeRequest(HUMIDITY);
    buf = (char *)buffer;
    for (q = 0; buf[q]; q++)
      ;
    for (; buf[q] != '\n'; q--)
      ;
    ;
    q++;
    lcd.print(buf + q);
    delay(5000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PR (Pa):");
    lcd.setCursor(9, 0);
    makeRequest(PRESSURE);
    buf = (char *)buffer;
    for (q = 0; buf[q]; q++)
      ;
    for (; buf[q] != '\n'; q--)
      ;
    ;
    q++;
    lcd.print(buf + q);
    lcd.setCursor(0, 1);
    lcd.print("ALT (m):");
    lcd.setCursor(9, 1);
    makeRequest(ALTITUDE);
    buf = (char *)buffer;
    for (q = 0; buf[q]; q++)
      ;
    for (; buf[q] != '\n'; q--)
      ;
    ;
    q++;
    lcd.print(buf + q);
    delay(5000);
  }
}