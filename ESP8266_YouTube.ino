/**
 * Display number of youtube subscribers on a 4 digit LED Segment Display using TM1637 on NodeMCU Dev Board (ESP8266)
 *  
 * 09/2017
 */

#include <TM1637Display.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "credentials.h"

// ---- pins
const int CLK = D6;
const int DIO = D5;


// ---- wifi
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// ---- request infos from google api v3
const char* host = "www.googleapis.com";
const int httpPort = 443; // when using https we use WiFiClientSecure instead of WiFiClient
const char* url = "/youtube/v3/channels?part=statistics&id=" YOUTUBE_CHANNEL_ID "&key=" GOOGLE_API_KEY;

TM1637Display display(CLK, DIO); //set up the 4-Digit Display.



/**
 *
 */
void setup()
{
  display.setBrightness(0x0a); //set the diplay to maximum brightness

  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}



/**
   ---------------------------------------------------------------
*/
void loop()
{

  WiFiClientSecure client; // WiFiClient client;


  Serial.print("connecting to ");
  Serial.println(host);


  // ---- make request
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
   client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();

  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // ---- Read response (skip headers)
  
  bool bHeaderReceived = false;
  String jsonResponse = "";
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r" && !bHeaderReceived) {
      Serial.println("headers received");
      bHeaderReceived = true;
    }  else if (bHeaderReceived) {
      jsonResponse += line;
    }
  }



  // ---- parse json
  
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(jsonResponse);


  if (!root.success()) {
    Serial.println(jsonResponse);
    Serial.println("Parsing JSON failed");
    display.showNumberDec(0);
    delay(30 * 1000);
    return;
  } else {
    Serial.println( "Parsed JSON successfully");
  }


  JsonObject& statistics = root["items"][0]["statistics"];
  // const char* statistics_viewCount = statistics["viewCount"];
  const char* statistics_subscriberCount = statistics["subscriberCount"]; 

  display.showNumberDec(atoi(statistics_subscriberCount));

  Serial.println();
  Serial.println("closing connection");

  delay(60 * 10 * 1000); // sleep 10 minutes

}

