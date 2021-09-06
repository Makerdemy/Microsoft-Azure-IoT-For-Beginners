#include <WiFi.h>
#include "DHT.h"
#include "AzureIotHub.h"
#include "Esp32MQTTClient.h"

#define INTERVAL 2000
#define DEVICE_ID "ESP32Device"
#define MESSAGE_MAX_LEN 256
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

const char* ssid     = "ENTER SSID"; // Name of your Wi-Fi Network.
const char* password = "ENTER PASSWORD"; // WiFi Password

int messageCount = 1;

static const char* connectionString = "ENTER CONNECTION STRING FROM IoT HUB HERE";

const char *messageData = "{\"deviceId\":\"%s\", \"messageId\":%d, \"Temperature\":%f, \"Humidity\":%f}";

static bool hasWifi = false;
static bool messageSending = true;

static uint64_t send_interval_ms;

static void InitWifi()
{
  Serial.println("Connecting...");

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  hasWifi = true;
  
  Serial.println("WiFi connected");
  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result)
{
  if (result == IOTHUB_CLIENT_CONFIRMATION_OK)
  {
    Serial.println("Send Confirmation Callback finished.");
  }
}

// Arduino sketch
void setup()
{
  
  Serial.begin(115200);
  Serial.println(F("DHT11 test!"));
  dht.begin();
  Serial.println("ESP32 Device");
  Serial.println("Initializing...");

  //Initialize the WiFi module
  Serial.println(" > WiFi");
  hasWifi = false;
  InitWifi();
  
  if (!hasWifi)
  {
    return;
  }
  Serial.println(" > IoT Hub");
    
  Esp32MQTTClient_Init((const uint8_t*)connectionString);
  Esp32MQTTClient_SetSendConfirmationCallback(SendConfirmationCallback);

  send_interval_ms = millis();
}

void loop()
{
  if (hasWifi)
  {
    if (messageSending && 
        (int)(millis() - send_interval_ms) >= INTERVAL)
    {
      
      char messagePayload[MESSAGE_MAX_LEN];
      
      float temperature = dht.readTemperature();
      float humidity = dht.readHumidity();
      
      Serial.println("\n ");
      snprintf(messagePayload,MESSAGE_MAX_LEN,messageData, DEVICE_ID, messageCount++, temperature,humidity);
      Serial.println(messagePayload);
      
      EVENT_INSTANCE* message = Esp32MQTTClient_Event_Generate(messagePayload, MESSAGE);
      Esp32MQTTClient_SendEventInstance(message);
      
      send_interval_ms = millis();
    }
  }
  delay(10000);
}
