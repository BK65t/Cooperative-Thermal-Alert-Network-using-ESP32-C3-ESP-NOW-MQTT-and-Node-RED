# System Overview

The CTAN system incorporates:

- Local peer-to-peer communication (ESP-NOW)  
- Cloud integration (MQTT)  
- Deep sleep duty cycling  
- Distributed alert propagation  
- AES-128 encrypted messaging  

---

## Sensor Nodes  
Each includes:

- ESP32-C3  
- DHT11 sensor  
- WS2812B RGB LED  
- 10-second sampling interval  
- Encrypted binary ESP-NOW messages  
- Deep sleep (8 µA)

---

## Gateway Node  
Handles:

- ESP-NOW reception  
- MQTT publishing  
- Alert-owner coordination  
- Dashboard synchronization  

---

## Node-RED Dashboard  
Provides:

- Live telemetry  
- Alert indicators  
- History charts  
- Remote sampling interval control  
