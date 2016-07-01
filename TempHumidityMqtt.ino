#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// Set the sensor type to DHT11
#define DHTTYPE DHT11

// The data pin.  The 12 matches the pin D6 on the ESP8266
#define DHTPIN 12

// How often do we collect data
#define COLLECTION_PERIOD_IN_MS 30000

// Update these with values suitable for your network.
const char* ssid = "put ssid here";
const char* password = "put password here";

// Set a broker client name.  This should be unique otherwise you will 
// have mqtt connection issues
const char* mqtt_client_name = "put unique client name here";

// The broker.  For testing we use the public broker from hivemq
const char* mqtt_server = "broker.hivemq.com";

const char* mqtt_temp_topic = "put temp topic here";
const char* mqtt_humidity_topic = "put humidity topic here";


// Keep track of last message time so we dont send too frequently
long lastMsg = 0;

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

char tempC[8];
char humidity[8];

void setup_wifi() {
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

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

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_client_name)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  Serial.print("Connecting to");
  Serial.println(mqtt_server); 
  client.setServer(mqtt_server, 1883);
  dht.begin();
}

void loop() {
  if (!client.connected()) {
    Serial.println("Not connected, reconnecting");
    reconnect();
  }
  
  client.loop();

  long now = millis();
  if (now - lastMsg > COLLECTION_PERIOD_IN_MS) {
    lastMsg = now;

    float h = dht.readHumidity();
    
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
  
    Serial.print("h = "); 
    Serial.print(h);
    Serial.print("\t T = ");
    Serial.println(t);
   
    dtostrf(h, 4, 4, humidity);
    dtostrf(t, 4, 4, tempC);

    client.publish(mqtt_humidity_topic, humidity);
    client.publish(mqtt_temp_topic, tempC);
  }
}
