#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>  //make HTTP requests using a very simple interface
#include <ArduinoJson.h> //Parse JSON, preallocated memory pool to store the JsonObject tree, this is done by the StaticJsonBuffer.

#include "config.h"

//Init Variable
unsigned long CheckWifiTimer = 0;
unsigned long HttpTimer = 0;
//

void wifiInit()
{
  //DO NOT TOUCH
  //  This is here to force the ESP32 to reset the WiFi and initialise correctly.
  Serial.print("WIFI status = ");
  Serial.println(WiFi.getMode());
  WiFi.disconnect(true);
  delay(200);
  WiFi.mode(WIFI_STA);
  delay(200);
  Serial.print("WIFI status = ");
  Serial.println(WiFi.getMode());
  // End silly stuff !!!

  wifiScan();
  delay(200);
  WiFi.begin(ssid, password);
  int retryWifiConnect = 0;
  int maxConnectionAttempt = 60 * 15;
  while (retryWifiConnect < maxConnectionAttempt && WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    //wifiScan();
    Serial.println(ssid);
    Serial.println(WiFi.RSSI());
    Serial.println(WiFi.status());
    delay(1000);
    Serial.println("Connecting to WiFi..");
    retryWifiConnect++;
  }

  if (retryWifiConnect >= maxConnectionAttempt)
  {
    restartDevice();
  }
  if (WiFi.waitForConnectResult() == WL_CONNECTED)
  {
    Serial.println("Connected to the WiFi network");
  }
}

void wifiScan()
{
  Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
  {
    Serial.println("no networks found");
  }
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print("'" + WiFi.SSID(i) + "'");
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
}

void checkWifiStatus()
{
  //Checking Wifi Status
  Serial.println(String("SSID :") + ssid);
  Serial.println(String("RSSI :") + WiFi.RSSI());
  Serial.println(String("Status :") + WiFi.status());
  if (WiFi.status() != 3)
  {
    wifiInit();
  }
}

void doHttpRequest()
{
  //Check the current connection status
  HTTPClient http;

  // Your Domain name with URL path or IP address with path
  String request = endpoint + key;
  Serial.println(String("request :") + request);
  http.begin(request);       //Specify the URL
  int httpCode = http.GET(); //Make the request
  int retry = 0;
  int maxAttempt = 60 * 5;
  while (retry < maxAttempt && httpCode <= 0)
  {
    httpCode = http.GET();
    delay(1000);
    Serial.println("Make the request");
    retry++;
  }

  if (retry >= maxAttempt)
  {
    restartDevice();
  }

  if (httpCode > 0)
  { //Check for the returning code

    String response = http.getString();
    Serial.println(httpCode);

    DynamicJsonDocument doc(200);
    deserializeJson(doc, response);
    String weather = doc["weather"][0]["main"];
    Serial.println(weather);
  }

  else
  {
    Serial.println("Error on HTTP request");
  }
  http.end(); //Free the resources
}

void restartDevice(bool skipCoolingDown = false)
{
  if (skipCoolingDown == false)
  {
    Serial.println("Please Wait.. Cooling down Device");
    delay(60 * 5 * 1000);
  }
  Serial.println("Restart Device");
  ESP.restart();
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);

  wifiInit();
}

void loop()
{
  // put your main code here, to run repeatedly:
  //Check Coundown
  long CheckWifiCoundown = CheckWifiDelay - (millis() - CheckWifiTimer);
  long HttpCoundown = HttpDelay - (millis() - HttpTimer);
  // Serial.println(String("Millis :") + millis());
  // Serial.println(String("timerDelay :") + timerDelay);
  // Serial.println(String("Cowndown :") + (timerDelay - (millis() - lastTime)));

  if (CheckWifiCoundown < 0)
  {
    //update Timer
    CheckWifiTimer = millis();
    checkWifiStatus();
  }

  if (HttpCoundown < 0 && (WiFi.status() == WL_CONNECTED))
  {
    //update LastTime
    HttpTimer = millis();
    doHttpRequest();
  }
}