# Gateway & Dashboard

## Gateway Functions
- Receives ESP-NOW packets  
- Publishes telemetry + alerts to MQTT  
- Runs alert-owner logic  
- Broadcasts alert states  
- Executes remote configuration commands  

---

## MQTT Setup
Broker: `broker.hivemq.com`  
Port: `2222`

Topics:
- `/b31iot/group135/telemetry`
- `/b31iot/group135/alert`
- `/b31iot/group135/config/interval`

---

## Node-RED Dashboard Features
- Real-time temperature + humidity  
- Color-coded alerts  
- History charts  
- Node online/offline status  
- Interval control sliders  
