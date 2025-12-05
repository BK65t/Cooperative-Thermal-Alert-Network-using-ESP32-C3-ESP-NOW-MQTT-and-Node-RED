# Communication Architecture

CTAN uses a dual-layer communication model:

---

# 1. ESP-NOW (Local Wireless)
- AES-128 encrypted  
- Connectionless  
- Ultra-low power  
- Used for telemetry + alerts  

### Message Types
- MSG_HELLO  
- MSG_ACK  
- MSG_TELEMETRY  
- MSG_ALERT  
- MSG_ALERT_CLEAR  
- MSG_ALERT_BCAST  
- MSG_CONFIG_SET  

---

## Encoding Comparison

| Format | Size | TX Time | Energy |
|--------|-------|-----------|--------|
| **Binary (C-struct)** | 24 bytes | 1.7ms | 0.408 mAs |
| JSON | 80–120 bytes | 15–20ms | 3.6–4.8 mAs |
| XML | 150–200 bytes | 30–40ms | 7.2–9.6 mAs |

Binary encoding saves **88% energy**.

---

# 2. MQTT (Cloud Layer)
Broker: `broker.hivemq.com:2222`

### Topics
- `/b31iot/group135/telemetry`
- `/b31iot/group135/alert`
- `/b31iot/group135/config/interval`
