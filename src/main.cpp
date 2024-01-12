#include <Arduino.h>
#include <Wifi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

#include "wificonfig.h"

// #region 通用配置区
const char *ssid = "embedLab";
const char *mqtt_server = "192.168.1.200";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
// #endregion

// TODO #region 个性化配置

const char *cmdTopic = "mogu/servo";
const char *reportTopic = "report/all";
String clientId(cmdTopic);
// #endregion

// TODO #region 本地离线逻辑

const int servoPin=32;
Servo servo;

// 连接wifi过程
void setup_wifi()
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// 收到mqtt订阅回调
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println();
  // payload转成char数组
  char cmd[length + 1];
  cmd[length] = '\0';
  for (int i = 0; i < length; i++)
  {
    cmd[i] = (char)payload[i];
    Serial.print(cmd[i]);
  }
  // char数组再转成String
  String cmdstr(cmd);
  String topicstr(topic);

  // TODO =================主逻辑=================
  if (topicstr == clientId)
  {
    Serial.println("ReceiveMyCmd!");
    Serial.println(cmdstr);
    if (cmdstr == "ISONLINE")
    {
      client.publish(reportTopic, "mogu/servo: live");

    }
    if (cmdstr == "drop")
    {
      client.publish(reportTopic, "mogu/servo: drop");
      servo.write(30);
      delay(1000);
      servo.write(180);
    }
  }
  // ======================================
}

// 和MQTT服务器断连后自动重连
void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe(cmdTopic);
      // 上报上线消息
      client.publish(reportTopic, "mogu/servo: online"); // TODO 改名字

    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  // 初始化串口
  Serial.begin(115200);

  setup_wifi();

  // espClient.setFingerprint(fingerprint);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // TODO #region 本地离线逻辑
  pinMode(servoPin, OUTPUT);
  delay(100);
  servo.attach(servoPin);
  servo.write(180);
}


void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();


}
