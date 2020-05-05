#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>  //make HTTP requests using a very simple interface
#include <ArduinoJson.h> //Parse JSON, preallocated memory pool to store the JsonObject tree, this is done by the StaticJsonBuffer.

#include "config.h"

//abstract
void wifiInit();
void wifiScan();
//abstract

//Init Variable
unsigned long CheckWifiTimer = 0;
unsigned long HttpTimer = 0;
//

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

    //Checking Wifi Status
    Serial.println(String("SSID :") + ssid);
    Serial.println(String("RSSI :") + WiFi.RSSI());
    Serial.println(String("Status :") + WiFi.status());
    if (WiFi.status() != 3)
    {
      wifiInit();
    }
    
  }

  if (HttpCoundown < 0 && (WiFi.status() == WL_CONNECTED))
  {
    //update LastTime
    HttpTimer = millis();

    //Check the current connection status
    HTTPClient http;

    // Your Domain name with URL path or IP address with path
    String request = endpoint + key;
    Serial.println(String("request :") + request);
    http.begin(request);       //Specify the URL
    int httpCode = http.GET(); //Make the request

    if (httpCode > 0)
    { //Check for the returning code

      String response = http.getString();
      Serial.println(httpCode);
      //Serial.println(payload);

      //      const size_t capacity = JSON_ARRAY_SIZE(3) + 2 * JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 3 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(12) + 340;
      //      DynamicJsonDocument doc(capacity);
      DynamicJsonDocument doc(200);
      deserializeJson(doc, response);
      String weather = doc["weather"][0]["main"];
      Serial.println(weather);

      //        float coord_lon = doc["coord"]["lon"]; // -0.13
      //        float coord_lat = doc["coord"]["lat"]; // 51.51
      //
      //        JsonArray weather = doc["weather"];
      //
      //        JsonObject weather_0 = weather[0];
      //        int weather_0_id = weather_0["id"]; // 301
      //        const char* weather_0_main = weather_0["main"]; // "Drizzle"
      //        const char* weather_0_description = weather_0["description"]; // "drizzle"
      //        const char* weather_0_icon = weather_0["icon"]; // "09n"
      //
      //        JsonObject weather_1 = weather[1];
      //        int weather_1_id = weather_1["id"]; // 701
      //        const char* weather_1_main = weather_1["main"]; // "Mist"
      //        const char* weather_1_description = weather_1["description"]; // "mist"
      //        const char* weather_1_icon = weather_1["icon"]; // "50n"
      //
      //        JsonObject weather_2 = weather[2];
      //        int weather_2_id = weather_2["id"]; // 741
      //        const char* weather_2_main = weather_2["main"]; // "Fog"
      //        const char* weather_2_description = weather_2["description"]; // "fog"
      //        const char* weather_2_icon = weather_2["icon"]; // "50n"
      //
      //        const char* base = doc["base"]; // "stations"
      //
      //        JsonObject main = doc["main"];
      //        float main_temp = main["temp"]; // 281.87
      //        int main_pressure = main["pressure"]; // 1032
      //        int main_humidity = main["humidity"]; // 100
      //        float main_temp_min = main["temp_min"]; // 281.15
      //        float main_temp_max = main["temp_max"]; // 283.15
      //
      //        int visibility = doc["visibility"]; // 2900
      //
      //        float wind_speed = doc["wind"]["speed"]; // 1.5
      //
      //        int clouds_all = doc["clouds"]["all"]; // 90
      //
      //        long dt = doc["dt"]; // 1483820400
      //
      //        JsonObject sys = doc["sys"];
      //        int sys_type = sys["type"]; // 1
      //        int sys_id = sys["id"]; // 5091
      //        float sys_message = sys["message"]; // 0.0226
      //        const char* sys_country = sys["country"]; // "GB"
      //        long sys_sunrise = sys["sunrise"]; // 1483776245
      //        long sys_sunset = sys["sunset"]; // 1483805443
      //
      //        long id = doc["id"]; // 2643743
      //        const char* name = doc["name"]; // "London"
      //        int cod = doc["cod"]; // 200
    }

    else
    {
      Serial.println("Error on HTTP request");
    }
    http.end(); //Free the resources
  }
}

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

  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    //wifiScan();
    Serial.println(ssid);
    Serial.println(WiFi.RSSI());
    Serial.println(WiFi.status());
    delay(200);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");
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