# Alert Coordination

The alert system uses a hybrid approach:

---

##  Local Detection
Node detects HOT or COLD threshold crossing → LED turns RED/TURQUOISE.

---

##  ESP-NOW Alert
Node sends encrypted `MSG_ALERT` to gateway.

---

##  Gateway Broadcast
Gateway sends `MSG_ALERT_BCAST` to all nodes → all LEDs synchronize.

---

##  Alert-Owner Model
Prevents redundant broadcasts.

- First node detecting alert = **owner**  
- Other nodes’ alerts ignored  
- Avoids broadcast storms  
- Saves power  

---

## Alert Timeline (Measured)

| Event | Time |
|-------|--------|
| Node detects alert | 0.000s |
| LED turns red | 0.002s |
| ESP-NOW alert sent | 0.005s |
| Gateway receives | 0.007s |
| Broadcast to nodes | 0.012s |
| All nodes synchronized | **0.015s** |
| Dashboard updated | **0.200s** |
