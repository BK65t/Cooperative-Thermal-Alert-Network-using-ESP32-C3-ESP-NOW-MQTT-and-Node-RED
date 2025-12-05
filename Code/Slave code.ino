#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <DHT.h>
#include <Adafruit_NeoPixel.h>

#define NODE_ID        4 
#define GROUP_ID       135
#define MAX_CHANNEL    13

#define DHT_PIN        4
#define DHTTYPE        DHT11
#define RGB_PIN        5

#define HOT_THRESHOLD   30.0
#define WARN_HOT_THRESHOLD 25.0 
#define WARN_COLD_THRESHOLD 15.0 
#define COLD_THRESHOLD  10.0

uint8_t broadcastAddress[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

float lastTemp = NAN;
float lastHum  = NAN;

unsigned long txStartTime_us = 0;

bool    localAlertNow     = false;
uint8_t localAlertCode    = 0;  
bool    remoteAlertActive = false;

uint32_t sendIntervalMs = 10000;

uint8_t gatewayMac[6] = {0};

RTC_DATA_ATTR bool     rtcPaired      = false;
RTC_DATA_ATTR int      rtcChannel     = 1;
RTC_DATA_ATTR uint32_t rtcIntervalMs  = 10000;
RTC_DATA_ATTR uint8_t  rtcGatewayMac[6] = {0};

RTC_DATA_ATTR uint8_t  rtcLastAlertCode = 0;  
RTC_DATA_ATTR uint32_t rtcLastColor     = 0;  

uint8_t PMK_KEY[16] = {
  0x13, 0x37, 0xBA, 0xBE,
  0xFE, 0xED, 0x22, 0x11,
  0x77, 0x66, 0x44, 0x55,
  0xAA, 0xBB, 0xCC, 0xDD
};

enum MessageType : uint8_t {
  MSG_HELLO        = 0,
  MSG_ACK          = 1,
  MSG_TELEMETRY    = 2,
  MSG_ALERT        = 3,
  MSG_ALERT_CLEAR  = 4,
  MSG_ALERT_BCAST  = 5,
  MSG_CONFIG_SET   = 6
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

DHT dht(DHT_PIN, DHTTYPE);
Adafruit_NeoPixel pixel(1, RGB_PIN, NEO_GRB + NEO_KHZ800);

void setChannel(int ch);
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len);
void onDataSent(const esp_now_send_info_t *info, esp_now_send_status_t status);


// Save LED state
void saveLEDToRTC(uint8_t r, uint8_t g, uint8_t b, uint8_t alertCode)
{
  rtcLastAlertCode = alertCode;
  rtcLastColor = ( (uint32_t)r << 16 ) | ( (uint32_t)g << 8 ) | b;

  Serial.printf("[NODE] Saved LED RTC: alert=%u RGB=(%u,%u,%u)\n",
                alertCode, r, g, b);
}

// Restore LED state at boot
void restoreLEDFromRTC()
{
  if (rtcLastColor == 0) {
    Serial.println("[NODE] No previous LED (first boot)");
    return;
  }

  uint8_t r = (rtcLastColor >> 16) & 0xFF;
  uint8_t g = (rtcLastColor >> 8) & 0xFF;
  uint8_t b = (rtcLastColor) & 0xFF;

  pixel.setPixelColor(0, pixel.Color(r, g, b));
  pixel.show();

  Serial.printf("[NODE] Restored LED: alert=%u RGB=(%u,%u,%u)\n",
                rtcLastAlertCode, r, g, b);
}


// LED CONTROL

void setLED(uint8_t r, uint8_t g, uint8_t b) {
  pixel.setPixelColor(0, pixel.Color(r, g, b));
  pixel.show();
  saveLEDToRTC(r, g, b, localAlertCode);
}

void updateLED() {
   
    if (localAlertNow || remoteAlertActive) {
        setLED(255, 0, 0); // RED (Critical Alert)
    }
    // PRIORITY 2: Temperature Zones
    // HOT WARNING: T >= 25.0
    else if (lastTemp >= WARN_HOT_THRESHOLD) {
        setLED(255, 255, 0); // YELLOW
    }
    // NORMAL: 15.0 < T < 25.0
    else if (lastTemp > WARN_COLD_THRESHOLD) {
        setLED(0, 255, 0);   // GREEN
    }
    // COLD WARNING: T <= 15.0
    else if (lastTemp <= WARN_COLD_THRESHOLD) {
        setLED(0, 255, 255); // TURQUOISE
    }
    // Fallback (Should mathematically never happen if sensor works)
    else { 
        setLED(128, 0, 128); // PURPLE (Safety fallback)
    }
}

// ========================================================================
// ESP-NOW CALLBACKS
// ========================================================================
void addGatewayPeerIfNeeded() {
  if (!rtcPaired) return;

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, gatewayMac, 6);
  peer.encrypt = true;

  esp_now_add_peer(&peer);
}

void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len != sizeof(EspNowMessage)) return;

  EspNowMessage msg;
  memcpy(&msg, data, sizeof(msg));

  if (msg.groupId != GROUP_ID) return;

  switch (msg.type) {
    case MSG_ACK:
      if (!rtcPaired && msg.nodeId == NODE_ID) {
        rtcPaired = true;
        memcpy(gatewayMac, info->src_addr, 6);
        memcpy(rtcGatewayMac, info->src_addr, 6);
        Serial.println("[NODE] Paired with gateway");
        addGatewayPeerIfNeeded();
      }
      break;

    case MSG_ALERT_BCAST:
      remoteAlertActive = (msg.alertCode != 0);
      if(remoteAlertActive){
        localAlertCode = msg.alertCode;
      }
      break;

    case MSG_CONFIG_SET:
      if (msg.intervalMs >= 1000 && msg.intervalMs <= 600000) {
        rtcIntervalMs  = msg.intervalMs;
        sendIntervalMs = msg.intervalMs;
      }
      break;
  }
}

void onDataSent(const esp_now_send_info_t *info, esp_now_send_status_t status) {
  unsigned long endTime = micros();
  unsigned long activeTime = 0;

  
  if (txStartTime_us > 0) {
    activeTime = endTime - txStartTime_us;
    txStartTime_us = 0; // Reset for the next cycle
  }

  Serial.printf("[TX_DONE] Status: %s. Active Time: %lu us\n",
                (status == ESP_NOW_SEND_SUCCESS) ? "SUCCESS" : "FAIL",
                activeTime);
}


// ESP-NOW + WIFI INIT

void setChannel(int ch) {
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
}

void initEspNow() {
  esp_now_set_pmk(PMK_KEY);

  if (esp_now_init() != ESP_OK) {
    Serial.println("[NODE] ESP-NOW init FAILED!");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);
  esp_now_register_send_cb(onDataSent);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, broadcastAddress, 6);
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  if (rtcPaired) {
    memcpy(gatewayMac, rtcGatewayMac, 6);
    addGatewayPeerIfNeeded();
  }

  Serial.println("[NODE] ESP-NOW ready");
}


void sendHello() {
  EspNowMessage msg = {};
  msg.type = MSG_HELLO;
  msg.nodeId = NODE_ID;
  msg.groupId = GROUP_ID;
  esp_now_send(broadcastAddress, (uint8_t*)&msg, sizeof(msg));
}

void sendTelemetry() {
  if (!rtcPaired) return;
  EspNowMessage msg = {};
  msg.type        = MSG_TELEMETRY;
  msg.nodeId      = NODE_ID;
  msg.groupId     = GROUP_ID;
  msg.alertCode   = localAlertCode;
  msg.temperature = lastTemp;
  msg.humidity    = lastHum;


  txStartTime_us = micros();
  Serial.printf("[TX] Sending Telemetry at: %lu us\n", txStartTime_us);
  
  esp_now_send(gatewayMac, (uint8_t*)&msg, sizeof(msg));
}

void sendAlert(uint8_t code, bool clear) {
  if (!rtcPaired) return;
  EspNowMessage msg = {};
  msg.type        = clear ? MSG_ALERT_CLEAR : MSG_ALERT;
  msg.nodeId      = NODE_ID;
  msg.groupId     = GROUP_ID;
  msg.alertCode   = code;
  msg.temperature = lastTemp;
  msg.humidity    = lastHum;
  esp_now_send(gatewayMac, (uint8_t*)&msg, sizeof(msg));
}


bool scanForGateway() {
  Serial.println("[NODE] Scanning for gateway...");

  for (int ch = 1; ch <= MAX_CHANNEL; ch++) {
    setChannel(ch);
    rtcChannel = ch;
    sendHello();

    unsigned long t = millis();
    while (millis() - t < 300) {
      if (rtcPaired) return true;
      delay(5);
    }
  }

  Serial.println("[NODE] No gateway found.");
  return false;
}

// DEEP SLEEP

void goToSleep() {
  uint32_t s = rtcIntervalMs;
  if (s < 1000) s = 1000;

  Serial.printf("[NODE] Sleep %u ms\n", s);

  esp_sleep_enable_timer_wakeup((uint64_t)s * 1000ULL);
  esp_deep_sleep_start();
}


void setup() {
  Serial.begin(115200);
  delay(2000);

  pixel.begin();
  pixel.show();

  restoreLEDFromRTC();

  dht.begin();
  WiFi.mode(WIFI_STA);
  initEspNow();

  sendIntervalMs = rtcIntervalMs;

  if (!rtcPaired) {
    if (!scanForGateway()) goToSleep();
  } else {
    memcpy(gatewayMac, rtcGatewayMac, 6);
  }

  setChannel(rtcChannel);

 
  lastTemp = dht.readTemperature();
  lastHum  = dht.readHumidity();

  
  localAlertNow = false;
  localAlertCode = 0;

  if (!isnan(lastTemp)) {
  
    if (lastTemp >= HOT_THRESHOLD) { 
        localAlertNow = true; 
        localAlertCode = 1; // 1 = HOT
    }

    else if (lastTemp <= COLD_THRESHOLD) { 
        localAlertNow = true; 
        localAlertCode = 2; // 2 = COLD
    }

  if (localAlertNow) sendAlert(localAlertCode, false);
  else               sendAlert(0, true);
  }
  sendTelemetry();

  
  unsigned long t = millis();
  while (millis() - t < 500) delay(10);

 
  updateLED();

  
  goToSleep();
}


void loop() {
  }

