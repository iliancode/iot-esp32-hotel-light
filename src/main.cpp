#include <WiFi.h>
#include <PubSubClient.h>
#include <esp32-hal-gpio.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define HALL_SENSOR_PIN 13
#define LED_PIN 4

const char* ssid = "Campus-Eductive";
const char* password = "R3s3@u-G3S";

const char* mqtt_server = "10.213.131.116";
const char* mqtt_topic = "sensor/ledStatus";

WiFiClient espClient;
PubSubClient client(espClient);

const int oneWireBus = 32;     
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

bool ledState = false;

void setup_wifi() {
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(HALL_SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(32, INPUT_PULLUP);

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  sensors.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int sensorValue = hallRead();
  
  if (sensorValue < 0 || sensorValue > 250) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);

    if (ledState) {
      client.publish(mqtt_topic, "LED On");
    } else {
      client.publish(mqtt_topic, "LED Off");
    }
  }

  // Lire la température (facultatif, peut être ignoré pour l'instant)
  // sensors.requestTemperatures(); 
  // float temperatureC = sensors.getTempCByIndex(0);
  // float temperatureF = sensors.getTempFByIndex(0);
  // Serial.print(temperatureC);
  // Serial.println("ºC");
  // Serial.print(temperatureF);
  // Serial.println("ºF");

  // delay(500);
}