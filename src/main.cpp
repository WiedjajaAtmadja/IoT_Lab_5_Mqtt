#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define MQTT_BROKER  "broker.emqx.io"
#define MQTT_TOPIC_PUBLISH   "esp32_iot/data"
#define MQTT_TOPIC_SUBSCRIBE "esp32_iot/cmd"  

Ticker timerPublish;
char g_szDeviceId[30];
Ticker timerMqtt;
WiFiClient espClient;
PubSubClient mqtt(espClient);
boolean mqttConnect();
void WifiConnect();

void onPublishMessage()
{
  char szMsg[50];
  static int nMsgCount=0;
  sprintf(szMsg, "Hello from %s - %d", g_szDeviceId, nMsgCount++);
  mqtt.publish(MQTT_TOPIC_PUBLISH, szMsg);
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  WifiConnect();
  mqttConnect();
  timerPublish.attach_ms(3000, onPublishMessage);
}

void loop()
{
  mqtt.loop();
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.write(payload, len);
  Serial.println();
}
boolean mqttConnect() {
  sprintf(g_szDeviceId, "esp32_%08X",(uint32_t)ESP.getEfuseMac());
  mqtt.setServer(MQTT_BROKER, 1883);
  mqtt.setCallback(mqttCallback);
  Serial.printf("Connecting to %s clientId: %s\n", MQTT_BROKER, g_szDeviceId);

  boolean fMqttConnected = false;
  for (int i=0; i<3 && !fMqttConnected; i++) {
    Serial.print("Connecting to mqtt broker...");
    fMqttConnected = mqtt.connect(g_szDeviceId);
    if (fMqttConnected == false) {
      Serial.print(" fail, rc=");
      Serial.println(mqtt.state());
      delay(1000);
    }
  }

  if (fMqttConnected)
  {
    Serial.println(" success");
    mqtt.subscribe(MQTT_TOPIC_SUBSCRIBE);
    Serial.printf("Subcribe topic: %s\n", MQTT_TOPIC_SUBSCRIBE);
    onPublishMessage();
  }
  return mqtt.connected();
}

void WifiConnect()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }  
  Serial.print("System connected with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %d\n", WiFi.RSSI());
}