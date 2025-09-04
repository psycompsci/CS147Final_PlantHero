#include <Arduino.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>
#include <string>
#include <HTTPClient.h> 
#include <WiFi.h> 
#include <WiFiClient.h> 
#include "Wire.h" 
#include "DHT20.h"
#include "pitches.h"


// WiFi credentials
#define WIFI_SSID "SETUP-1520"
#define WIFI_PASSWORD "crumb6382depend"

#define SAS_TOKEN "SharedAccessSignature sr=cs147final34.azure-devices.net%2Fdevices%2Fcs147esp32&sig=CLgVwcJEgaEOLA%2B4ce55U7xCGdclSOnQZUhFEV67RY8%3D&se=1756960121"

const char* root_ca = \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIEtjCCA56gAwIBAgIQCv1eRG9c89YADp5Gwibf9jANBgkqhkiG9w0BAQsFADBh\n" \
  "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
  "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n" \
  "MjAeFw0yMjA0MjgwMDAwMDBaFw0zMjA0MjcyMzU5NTlaMEcxCzAJBgNVBAYTAlVT\n" \
  "MR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xGDAWBgNVBAMTD01TRlQg\n" \
  "UlMyNTYgQ0EtMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMiJV34o\n" \
  "eVNHI0mZGh1Rj9mdde3zSY7IhQNqAmRaTzOeRye8QsfhYFXSiMW25JddlcqaqGJ9\n" \
  "GEMcJPWBIBIEdNVYl1bB5KQOl+3m68p59Pu7npC74lJRY8F+p8PLKZAJjSkDD9Ex\n" \
  "mjHBlPcRrasgflPom3D0XB++nB1y+WLn+cB7DWLoj6qZSUDyWwnEDkkjfKee6ybx\n" \
  "SAXq7oORPe9o2BKfgi7dTKlOd7eKhotw96yIgMx7yigE3Q3ARS8m+BOFZ/mx150g\n" \
  "dKFfMcDNvSkCpxjVWnk//icrrmmEsn2xJbEuDCvtoSNvGIuCXxqhTM352HGfO2JK\n" \
  "AF/Kjf5OrPn2QpECAwEAAaOCAYIwggF+MBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYD\n" \
  "VR0OBBYEFAyBfpQ5X8d3on8XFnk46DWWjn+UMB8GA1UdIwQYMBaAFE4iVCAYlebj\n" \
  "buYP+vq5Eu0GF485MA4GA1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcD\n" \
  "AQYIKwYBBQUHAwIwdgYIKwYBBQUHAQEEajBoMCQGCCsGAQUFBzABhhhodHRwOi8v\n" \
  "b2NzcC5kaWdpY2VydC5jb20wQAYIKwYBBQUHMAKGNGh0dHA6Ly9jYWNlcnRzLmRp\n" \
  "Z2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RHMi5jcnQwQgYDVR0fBDswOTA3\n" \
  "oDWgM4YxaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0R2xvYmFsUm9v\n" \
  "dEcyLmNybDA9BgNVHSAENjA0MAsGCWCGSAGG/WwCATAHBgVngQwBATAIBgZngQwB\n" \
  "AgEwCAYGZ4EMAQICMAgGBmeBDAECAzANBgkqhkiG9w0BAQsFAAOCAQEAdYWmf+AB\n" \
  "klEQShTbhGPQmH1c9BfnEgUFMJsNpzo9dvRj1Uek+L9WfI3kBQn97oUtf25BQsfc\n" \
  "kIIvTlE3WhA2Cg2yWLTVjH0Ny03dGsqoFYIypnuAwhOWUPHAu++vaUMcPUTUpQCb\n" \
  "eC1h4YW4CCSTYN37D2Q555wxnni0elPj9O0pymWS8gZnsfoKjvoYi/qDPZw1/TSR\n" \
  "penOgI6XjmlmPLBrk4LIw7P7PPg4uXUpCzzeybvARG/NIIkFv1eRYIbDF+bIkZbJ\n" \
  "QFdB9BjjlA4ukAg2YkOyCiB8eXTBi2APaceh3+uBLIgLk8ysy52g2U3gP7Q26Jlg\n" \
  "q/xKzj3O9hFh/g==\n" \
  "-----END CERTIFICATE-----\n";

/*
const char* root_ca = \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIEtjCCA56gAwIBAgIQCv1eRG9c89YADp5Gwibf9jANBgkqhkiG9w0BAQsFADBh\n" \
  "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
  "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n" \
  "MjAeFw0yMjA0MjgwMDAwMDBaFw0zMjA0MjcyMzU5NTlaMEcxCzAJBgNVBAYTAlVT\n" \
  "MR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xGDAWBgNVBAMTD01TRlQg\n" \
  "UlMyNTYgQ0EtMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMiJV34o\n" \
  "eVNHI0mZGh1Rj9mdde3zSY7IhQNqAmRaTzOeRye8QsfhYFXSiMW25JddlcqaqGJ9\n" \
  "GEMcJPWBIBIEdNVYl1bB5KQOl+3m68p59Pu7npC74lJRY8F+p8PLKZAJjSkDD9Ex\n" \
  "mjHBlPcRrasgflPom3D0XB++nB1y+WLn+cB7DWLoj6qZSUDyWwnEDkkjfKee6ybx\n" \
  "SAXq7oORPe9o2BKfgi7dTKlOd7eKhotw96yIgMx7yigE3Q3ARS8m+BOFZ/mx150g\n" \
  "dKFfMcDNvSkCpxjVWnk//icrrmmEsn2xJbEuDCvtoSNvGIuCXxqhTM352HGfO2JK\n" \
  "AF/Kjf5OrPn2QpECAwEAAaOCAYIwggF+MBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYD\n" \
  "VR0OBBYEFAyBfpQ5X8d3on8XFnk46DWWjn+UMB8GA1UdIwQYMBaAFE4iVCAYlebj\n" \
  "buYP+vq5Eu0GF485MA4GA1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcD\n" \
  "AQYIKwYBBQUHAwIwdgYIKwYBBQUHAQEEajBoMCQGCCsGAQUFBzABhhhodHRwOi8v\n" \
  "b2NzcC5kaWdpY2VydC5jb20wQAYIKwYBBQUHMAKGNGh0dHA6Ly9jYWNlcnRzLmRp\n" \
  "Z2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RHMi5jcnQwQgYDVR0fBDswOTA3\n" \
  "oDWgM4YxaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0R2xvYmFsUm9v\n" \
  "dEcyLmNybDA9BgNVHSAENjA0MAsGCWCGSAGG/WwCATAHBgVngQwBATAIBgZngQwB\n" \
  "AgEwCAYGZ4EMAQICMAgGBmeBDAECAzANBgkqhkiG9w0BAQsFAAOCAQEAdYWmf+AB\n" \
  "klEQShTbhGPQmH1c9BfnEgUFMJsNpzo9dvRj1Uek+L9WfI3kBQn97oUtf25BQsfc\n" \
  "kIIvTlE3WhA2Cg2yWLTVjH0Ny03dGsqoFYIypnuAwhOWUPHAu++vaUMcPUTUpQCb\n" \
  "eC1h4YW4CCSTYN37D2Q555wxnni0elPj9O0pymWS8gZnsfoKjvoYi/qDPZw1/TSR\n" \
  "penOgI6XjmlmPLBrk4LIw7P7PPg4uXUpCzzeybvARG/NIIkFv1eRYIbDF+bIkZbJ\n" \
  "QFdB9BjjlA4ukAg2YkOyCiB8eXTBi2APaceh3+uBLIgLk8ysy52g2U3gP7Q26Jlg\n" \
  "q/xKzj3O9hFh/g==\n" \
  "-----END CERTIFICATE-----\n";
*/
String iothubName = "cs147final34";
String deviceName = "cs147esp32";
String url = "https://" + iothubName + ".azure-devices.net/devices/" + deviceName + "/messages/events?api-version=2021-04-12";

#define TELEMETRY_INTERVAL 3000 // Send data every 3 seconds
#define LED 13
#define BUZZER_PIN 25
#define BUZZER_CHANNEL 0

DHT20 dht;
uint8_t count = 0;
uint32_t lastTelemetryTime = 0;
TFT_eSPI tft = TFT_eSPI();

const int sensorPin = 32;
int moisture = 0;
const int uvPin = 33;
float uvVoltage = 0.0;
float uvIndex = 0.0;
int uvRawValue = 0;

void setup() {
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(sensorPin, INPUT);
  pinMode(uvPin, INPUT);
  pinMode(LED, OUTPUT);
  delay(1000);

  Wire.begin();
  dht.begin(); // Initialize DHT20 sensor

  WiFi.mode(WIFI_STA);
  delay(1000);
  Serial.println();
  Serial.println("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.print(WiFi.status());
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());

  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_DARKGREEN);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
  tft.setTextSize(2);
  tft.setTextWrap(true);
  tft.drawString("Welcome", 67, 20, 2);
  tft.drawString("to", 107, 50, 2);
  tft.drawString("PLANT HERO!", 43, 80, 2);
  delay(10000);
  tft.fillScreen(TFT_DARKGREEN);
}

void loop() {
  // put your main code here, to run repeatedly:
  // Read sensors
  tft.drawString("...", 105, 60);
  dht.read();
  float temperature = dht.getTemperature();
  float humidity = dht.getHumidity();
  moisture = analogRead(sensorPin);
  uvRawValue = analogRead(uvPin);
  uvVoltage = uvRawValue * (3.3 / 4095.0);
  uvIndex = uvVoltage * 10.0;

  // Send telemetry at interval
  if (millis() - lastTelemetryTime >= TELEMETRY_INTERVAL) {
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT20 sensor!");
    } else {
      ArduinoJson::JsonDocument doc;
      doc["temperature"] = temperature;
      doc["humidity"] = humidity;
      doc["moisture"] = moisture;
      doc["uvIndex"] = uvIndex;
      char buffer[256];
      serializeJson(doc, buffer, sizeof(buffer));

      WiFiClientSecure client;
      client.setCACert(root_ca);
      HTTPClient http;
      http.begin(client, url);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Authorization", SAS_TOKEN);
      int httpCode = http.POST(buffer);

      if (httpCode == 204) {
        Serial.println("Telemetry sent: " + String(buffer));
      } else {
        Serial.println("Failed to send telemetry, HTTP code: " + String(httpCode));
      }
      http.end();
      lastTelemetryTime = millis();
    }
  }

  delay(2000);
  tft.fillScreen(TFT_DARKGREEN);

  
  // Display sensor values
  tft.drawString("Temperature: " + String(temperature), 10, 10);
  tft.drawString("Humidity: " + String(humidity), 10, 35);
  tft.drawString("Moisture: " + String(moisture), 10, 60);
  tft.drawString("UV: " + String(uvIndex, 2), 10, 85);

  delay(4000);
  tft.fillScreen(TFT_DARKGREEN);

  // Display plant condition
  tft.drawString("CONDITION: ", 10, 10, 2);

  //Temperature
  if(temperature < 15) {
    tft.drawString("Too Cold!", 10, 50);
    digitalWrite(LED, HIGH); // Turn on LED
    // tone(BUZZER_PIN, 500); 
    // delay(300); 
    // tone(BUZZER_PIN, 0);
    // delay(1000); 
  } else if(temperature >= 15 && temperature < 25) {
    tft.drawString("Optimal", 10, 50);
    tft.drawString("Temperature :)", 12, 70);
    digitalWrite(LED, LOW); // Turn off LED
  } else if(temperature >= 25 && temperature < 30) {
    tft.drawString("Warm Temperature", 10, 50);
    digitalWrite(LED, LOW); // Turn off LED
  } else if(temperature >= 30) {
    tft.drawString("Too Hot!", 10, 50);
    digitalWrite(LED, HIGH); // Turn on LED
    // tone(BUZZER_PIN, 500); 
    // delay(300); 
    // tone(BUZZER_PIN, 0);
    // delay(1000);
  }

  delay(2000);
  tft.fillScreen(TFT_DARKGREEN);
  tft.drawString("CONDITION: ", 10, 10, 2);

  //Humidity
  if(humidity < 30) {
    tft.drawString("Air is dry", 10, 50);
    digitalWrite(LED, HIGH); // Turn on LED
    // tone(BUZZER_PIN, 500); 
    // delay(300); 
    // tone(BUZZER_PIN, 0);
    // delay(1000); 
  } else if(humidity >= 40 && humidity <= 60) {
    tft.drawString("Optimal Humidity :)", 10, 50);
    digitalWrite(LED, LOW); // Turn off LED
  } else if(humidity >= 80) {
    tft.drawString("Air is too humid", 10, 50);
    digitalWrite(LED, HIGH); // Turn on LED
  } else {
    tft.drawString("Humidity is OK", 10, 50);
    digitalWrite(LED, LOW); // Turn off LED
  }

  delay(2000);
  tft.fillScreen(TFT_DARKGREEN);
  tft.drawString("CONDITION: ", 10, 10, 2);

  //Moisture
  if (moisture < 300) {
    tft.drawString("Soil is dry", 10, 50);
    digitalWrite(LED, HIGH); // Turn on LED
    // tone(BUZZER_PIN, 500); 
    // delay(300); 
    // tone(BUZZER_PIN, 0);
    // delay(1000); 
  } else if (moisture < 2000) {
    tft.drawString("Soil is moist :)", 10, 50);
    digitalWrite(LED, LOW); // Turn off LED
  } else {
    tft.drawString("Soil is too wet", 10, 50);
    digitalWrite(LED, HIGH); // Turn on LED
  }

  delay(2000);
  tft.fillScreen(TFT_DARKGREEN);
  tft.drawString("CONDITION: ", 10, 10, 2);

  //UV
  if(uvIndex < 2) {
    tft.drawString("Low UV Index", 10, 50);
    digitalWrite(LED, HIGH); // Turn on LED
    // tone(BUZZER_PIN, 500); 
    // delay(300); 
    // tone(BUZZER_PIN, 0);
    // delay(1000); 
  } else if(uvIndex >= 2 && uvIndex < 5) {
    tft.drawString("Moderate UV", 10, 50);
    tft.drawString("Index :)", 12, 70);
    digitalWrite(LED, LOW); // Turn off LED
  } else if(uvIndex >= 5 && uvIndex < 7) {
    tft.drawString("High UV Index", 10, 50);
    digitalWrite(LED, LOW); // Turn off LED
  } else if(uvIndex >= 7 && uvIndex < 10) {
    tft.drawString("Very High UV Index", 10, 50);
    digitalWrite(LED, HIGH); // Turn off LED
  } else {
    tft.drawString("Extreme UV Index", 10, 50);
    digitalWrite(LED, HIGH); // Turn on LED
  }

  delay(2000);


  tft.fillScreen(TFT_DARKGREEN);
  //if any of those values are below a certain threshold, turn on the LED
  // digitalWrite(LED, HIGH);
  // delay(500);
  // digitalWrite(LED, LOW);
  // delay(500);
}
