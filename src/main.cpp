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
const char* mqtt_topic_status = "sensor/ledStatus";
const char* mqtt_topic_control = "led/control";

WiFiClient espClient;
PubSubClient client(espClient);

const int oneWireBus = 32;     
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

bool ledState = false;
unsigned long lastPublishTime = 0;
const unsigned long publishInterval = 1000;

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

// Fonction de rappel pour gérer les messages MQTT reçus
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Message reçu sur le topic ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);

  if (String(topic) == mqtt_topic_control) {
    if (message == "on") {
      ledState = true;
      digitalWrite(LED_PIN, HIGH);
      client.publish(mqtt_topic_status, "on");
    } else if (message == "off") {
      ledState = false;
      digitalWrite(LED_PIN, LOW);
      client.publish(mqtt_topic_status, "off");
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe(mqtt_topic_control);  // Abonnement au sujet de contrôle
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
  client.setCallback(callback);  // Définir la fonction de rappel pour les messages MQTT
  sensors.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int sensorValue = hallRead();

  // Publier l'état de la LED à intervalles réguliers
  if ((sensorValue < 0 || sensorValue > 250) && (millis() - lastPublishTime > publishInterval)) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);

    if (ledState) {
      client.publish(mqtt_topic_status, "on");
    } else {
      client.publish(mqtt_topic_status, "off");
    }

    lastPublishTime = millis();
  }

  delay(100);
}