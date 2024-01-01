#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Galaxy M12 FD04";
const char* password = "Thanu0408";
const char* serverName = "https://maker.ifttt.com/trigger/hello/json/with/key/ckbRYiLhB46Zdqg3tPPg3";

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.println("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;

      http.begin(client, serverName);

      // http.setAuthorization("REPLACE WITH SERVER USERNAME", "REPLACE WITH SERVER PASSWORD");
      // http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      String httpRequestData = "value1=24.25&value2=49.548&value3=1005.14";
      // int httpResponseCode = http.POST(httpRequestData);

      // If you need an HTTP request with a content type: application/json, use the following:
      http.addHeader("Content-Type", "application/json");
      int httpResponseCode = http.POST("{\"value\": \"new_filelink.url\"}");

      // int httpResponseCode = http.POST("{\"api_key\": \"tPmAT5Ab3/7F98\", \"sensor\": \"BME280\", \"value1\": 24.25, \"value2\": 49.54, \"value3\": 1005.141}");

      // If you need an HTTP request with a content type: text/plain
      // http.addHeader("Content-Type", "text/plain");
      // int httpResponseCode = http.POST("Hello, World!");

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      // Free resources
      http.end();
    } else {
      Serial.println("WIFI Disconnected");
    }

    lastTime = millis();
  }
}
