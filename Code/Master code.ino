#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <PubSubClient.h>

const char* WIFI_SSID       = "VM8449490 (2.4GHz)";
const char* WIFI_PASSWORD   = "Rb4dkdrhvDkv";

const char* MQTT_SERVER     = "broker.hivemq.com";
const uint16_t MQTT_PORT    = 1883;
const char* MQTT_CLIENT_ID  = "b31iot_gateway_135";

const char* MQTT_TOPIC_TELEMETRY = "b31iot/group135/telemetry";
const char* MQTT_TOPIC_ALERT     = "b31iot/group135/alert";
const char* MQTT_TOPIC_CONFIG    = "b31iot/group135/config/interval";

#define GROUP_ID    135
#define MAX_NODES   32

#define HOT_THRESHOLD   30.0 
#define COLD_THRESHOLD  10.0

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

enum MessageType : uint8_t {
  MSG_HELLO       = 0,
  MSG_ACK         = 1,
  MSG_TELEMETRY   = 2,
  MSG_ALERT       = 3,
  MSG_ALERT_CLEAR = 4,
  MSG_ALERT_BCAST = 5,
  MSG_CONFIG_SET  = 6
};

struct __attribute__((packed)) EspNowMessage {
  uint8_t   type;
  uint8_t   nodeId;
  uint8_t   groupId;
  uint8_t   alertCode;
  float     temperature;
  float     humidity;
  uint32_t  intervalMs;
};

uint8_t PMK_KEY[16] = {
  0x13, 0x37, 0xBA, 0xBE,
  0xFE, 0xED, 0x22, 0x11,
  0x77, 0x66, 0x44, 0x55,
  0xAA, 0xBB, 0xCC, 0xDD
};

uint8_t knownNodes[MAX_NODES][6];
int     nodeCount = 0;

bool    globalAlertActive = false;
uint8_t globalAlertCode   = 0;
int     alertOwnerNodeId  = -1;

uint32_t currentIntervalMs = 10000; // default 10s

void printMac(const uint8_t *mac) {
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

bool isKnownNode(const uint8_t *mac) {
  for (int i = 0; i < nodeCount; i++) {
    if (memcmp(mac, knownNodes[i], 6) == 0) return true;
  }
  return false;
}

void addNode(const uint8_t *mac) {
  if (isKnownNode(mac)) return;
  if (nodeCount >= MAX_NODES) return;

  memcpy(knownNodes[nodeCount], mac, 6);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, mac, 6);
  peer.channel = 0;
  peer.encrypt = true;

  if (esp_now_add_peer(&peer) == ESP_OK) {
    Serial.print("[GW] Added ENCRYPTED node: ");
    printMac(mac);
    Serial.println();
    nodeCount++;
  }
}

// MQTT
void ensureMqtt() {
  while (!mqttClient.connected()) {
    Serial.print("[MQTT] Connecting...");
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println("connected");
      mqttClient.subscribe(MQTT_TOPIC_CONFIG);
    } else {
      Serial.print("failed (");
      Serial.print(mqttClient.state());
      Serial.println("), retrying...");
      delay(1500);
    }
  }
}

void publishTelemetry(const EspNowMessage &msg) {
  char buff[256];
  snprintf(buff, sizeof(buff),
           "{\"nodeId\":%u,\"temperature\":%.2f,\"humidity\":%.2f,\"alertCode\":%u}",
           msg.nodeId, msg.temperature, msg.humidity, msg.alertCode);
  mqttClient.publish(MQTT_TOPIC_TELEMETRY, buff);
}

void publishAlertJson(uint8_t nodeId, float temp, float hum,
                      const char* state) {
  char buff[256];
  snprintf(buff, sizeof(buff),
           "{\"nodeId\":%u,\"temperature\":%.2f,\"humidity\":%.2f,"
           "\"alert\":\"%s\",\"timeMs\":%lu}",
           nodeId, temp, hum, state, (unsigned long)millis());
  mqttClient.publish(MQTT_TOPIC_ALERT, buff);
}

void publishAlert(const EspNowMessage &msg, const char* state) {
  publishAlertJson(msg.nodeId, msg.temperature, msg.humidity, state);
}

// Broadcast current global alert state
void broadcastAlertState(bool verbose) {
  EspNowMessage b = {};
  b.type      = MSG_ALERT_BCAST;
  b.groupId   = GROUP_ID;
  b.alertCode = globalAlertActive ? globalAlertCode : 0;

  if (verbose){
    Serial.print("[GW] BROADCAST alertCode=");
    Serial.println(b.alertCode);
  }
  for (int i = 0; i < nodeCount; i++) {
    esp_now_send(knownNodes[i], (uint8_t*)&b, sizeof(b));
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  String msg;
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];

  if (String(topic) == MQTT_TOPIC_CONFIG) {
    int seconds = msg.toInt();
    if (seconds >= 1 && seconds <= 600) {
      currentIntervalMs = seconds * 1000UL;
      Serial.printf("[GW] Interval updated via MQTT -> %lu ms\n",
                     (unsigned long)currentIntervalMs);

      EspNowMessage cfg = {};
      cfg.type       = MSG_CONFIG_SET;
      cfg.groupId    = GROUP_ID;
      cfg.intervalMs = currentIntervalMs;

      for (int i = 0; i < nodeCount; i++) {
        esp_now_send(knownNodes[i], (uint8_t*)&cfg, sizeof(cfg));
      }
    }
  }
}

void sendUnencryptedAck(const uint8_t *mac, uint8_t nodeId) {
  EspNowMessage ack = {};
  ack.type    = MSG_ACK;
  ack.nodeId  = nodeId;
  ack.groupId = GROUP_ID;
  
  esp_now_peer_info_t temp = {};
  memcpy(temp.peer_addr, mac, 6);
  temp.channel = 0;
  temp.encrypt = false;

  esp_now_add_peer(&temp);
  esp_now_send(mac, (uint8_t*)&ack, sizeof(ack));
  esp_now_del_peer(mac);

  // From now on, use encrypted peer
  addNode(mac);
}

void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len != sizeof(EspNowMessage)) return;

  const uint8_t *mac = info->src_addr;
  EspNowMessage msg;
  memcpy(&msg, data, sizeof(msg));

  if (msg.groupId != GROUP_ID) return;

  switch (msg.type) {

    case MSG_HELLO: {
      Serial.println("[GW] HELLO");
      sendUnencryptedAck(mac, msg.nodeId);
      break;
    }

    case MSG_TELEMETRY: {
      Serial.printf("[GW] TELEMETRY from node %u: T=%.2fC H=%.2f%% alert=%u\n",
                     msg.nodeId, msg.temperature, msg.humidity, msg.alertCode);

      publishTelemetry(msg);

      // Send interval config back
      EspNowMessage cfg = {};
      cfg.type       = MSG_CONFIG_SET;
      cfg.groupId    = GROUP_ID;
      cfg.intervalMs = currentIntervalMs;

//      Serial.printf("[GW] Sending interval update to node %u: %lu ms\n",
//                     msg.nodeId, (unsigned long)currentIntervalMs);

      esp_now_send(mac, (uint8_t*)&cfg, sizeof(cfg));

      broadcastAlertState(false);
      break;
    }

    case MSG_ALERT: {
      Serial.printf("[GW] ALERT from node %u\n", msg.nodeId);

      // If no alert active yet, this node becomes the owner
      if (!globalAlertActive) {
        globalAlertActive = true;
        globalAlertCode   = msg.alertCode;
        alertOwnerNodeId  = msg.nodeId;
        Serial.printf("[GW] ALERT OWNER SET -> Node %d\n", alertOwnerNodeId);
      } else {
        Serial.printf("[GW] ALERT received but owner already %d\n",
                       alertOwnerNodeId);
      }

//      const char* alertType = "ALERT";
//      // Default fallback
//      if (msg.alertCode == 1) {
//          alertType = "HOT";
//      } else if (msg.alertCode == 2) { 
//          alertType = "COLD";
//      }
      
      publishAlert(msg, "CRITICAL");
      broadcastAlertState(true);
      break;
    }

    case MSG_ALERT_CLEAR: {
      // Only the owner can clear
      if (globalAlertActive && msg.nodeId == alertOwnerNodeId) {
        Serial.printf("[GW] ALERT_CLEAR from node %u\n", msg.nodeId);
        Serial.printf("[GW] ALERT OWNER CLEARED -> was Node %d\n", msg.nodeId);

        globalAlertActive = false;
        globalAlertCode   = 0;
        alertOwnerNodeId  = -1;

        publishAlert(msg, "CLEARED");
        broadcastAlertState(true);
      } else {
          if (alertOwnerNodeId != -1) {
             Serial.printf("[GW] ALERT_CLEAR from node %u\n", msg.nodeId);
             Serial.printf("[GW] CLEAR IGNORED - owner is Node %d\n", alertOwnerNodeId);
        }
      broadcastAlertState(false);
      }
      break;
    }

    default: {
      break;
    }
  }
}

void onDataSent(const esp_now_send_info_t *info, esp_now_send_status_t status) {
}

void connectWiFi() {
  Serial.printf("[WiFi] Connecting to SSID: %s ...\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\n[WiFi] Connected!");
  Serial.printf("[WiFi] IP Address: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("[WiFi] Channel: %d\n", WiFi.channel());
  Serial.printf("[WiFi] Gateway MAC: %s\n", WiFi.macAddress().c_str());
}

void initEspNow() {
  esp_now_set_pmk(PMK_KEY);

  if (esp_now_init() != ESP_OK) {
    Serial.println("[GW] ESP-NOW init FAILED");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);
  esp_now_register_send_cb(onDataSent);
  Serial.println("[GW] ESP-NOW Ready (encrypted).");
}

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println();
  Serial.println("===========================================");
  Serial.println("         B31IoT GATEWAY (GROUP 135)");
  Serial.println("        ESP-NOW + MQTT + Encryption");
  Serial.println("===========================================");
  Serial.println();

  connectWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  ensureMqtt();

  initEspNow();

  Serial.println("\n[GATEWAY] SYSTEM READY.\n");
}

void loop() {
  // 1. Check WiFi Connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WiFi] Connection lost! Reconnecting...");
    WiFi.disconnect();
    WiFi.reconnect();
    
    // Wait a bit for WiFi to recover
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      delay(500);
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n[WiFi] Reconnected!");
      Serial.printf("[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
      Serial.println("\n[WiFi] Reconnect failed.");
    }
  }

  // 2. Check MQTT Connection (Only if WiFi is OK)
  if (WiFi.status() == WL_CONNECTED && !mqttClient.connected()) {
    Serial.println("[MQTT] Connection lost! Reconnecting...");
    ensureMqtt();
  }

  // 3. Maintain MQTT Loop
  if (mqttClient.connected()) {
    mqttClient.loop();
  }
  
  // Small delay to prevent CPU hogging if loop is tight
  delay(10); 
}
