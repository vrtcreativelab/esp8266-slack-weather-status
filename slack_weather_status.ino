
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <DHT_U.h>


#ifndef STASSID
#define STASSID "YOUR WIFI ID"
#define STAPSK  "YOUR WIFI PASSWORD"
#endif

#define TOKEN "YOUR SLACK APP TOKEN"

#define DHTPIN            D2
#define DHTTYPE           DHT11

DHT_Unified dht(DHTPIN, DHTTYPE, 15);

const char fingerprint[] PROGMEM = "C1 0D 53 49 D2 3E E5 2B A2 61 D5 9E 6F 99 0D 3D FD 8B B2 B3";
const char* host = "slack.com";
const int httpsPort = 443;

void setup() {

  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.begin(STASSID, STAPSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  dht.begin();
}

void loop() {
  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClientSecure client;
    HTTPClient http;

    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      Serial.println("T: <error>");
      Serial.println(event.temperature);
    }
    else {
      int temperature = (int)event.temperature;

      Serial.print("connecting to ");
      Serial.println(host);

      Serial.printf("Using fingerprint '%s'\n", fingerprint);
      client.setFingerprint(fingerprint);

      if (!client.connect(host, httpsPort)) {
        Serial.println("connection failed");
        return;
      }
      String emoji;
      if (temperature >= 27) {
        emoji = ":sunny:";
      } else if (temperature >= 25) {
        emoji = ":sun_small_cloud:";
      } else if (temperature >= 22) {
        emoji = ":sun_behind_cloud:";
      } else if (temperature > 20) {
        emoji = ":cloud:";
      } else {
        emoji = ":snowman:";
      }

      String url = "/api/users.profile.set";
      String payload = "{\"profile\":{\"status_text\":\"Op mijn zolder is het " + (String)temperature  + "\\u00b0C\",\"status_expiration\":0, \"status_emoji\": \""+ emoji + "\"}}";


      String request = String("POST ") + url + " HTTP/1.1\r\n" +
                       "Host: " + host + "\r\n" +
                       "User-Agent: ESP8266\r\n" +
                       "Content-Type: application/json\r\n" +
                       "Content-Length: " + payload.length() + "\r\n" +
                       "Authorization: Bearer "+ TOKEN + "\r\n" +
                       "Connection: close\r\n\r\n" + payload + "\r\n\r\n";
      Serial.println(request);
      client.print(request);

      Serial.println("request sent");
      while (client.connected()) {
        String line = client.readStringUntil('\n\n');
        Serial.println(line);

        if (line == "\r") {
          Serial.println("headers received");
          break;
        }
      }
      String chunk = "";
      int limit = 1;
      String response = "";

      Serial.println(response);
      // String line = client.readStringUntil('z');
      do {
        if (client.connected()) {
          client.setTimeout(2000);
          chunk = client.readStringUntil('\n');
          response += chunk;
          Serial.println(chunk);
        }
      } while (chunk.length() > 0 && ++limit < 100);


    }
  }

  delay(60000);
}


