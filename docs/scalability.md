# Node Discovery & Scalability

## Auto-Discovery Process
1. Node boots  
2. Scans channels 1–13  
3. Sends `MSG_HELLO`  
4. Gateway replies `MSG_ACK`  
5. Node stores gateway MAC in RTC  
6. Encrypted communication begins  

---

## Scalability

| Nodes | Load | Alert Time | Congestion |
|-------|--------|--------------|--------------|
| 4 | Very Low | 15ms | None |
| 8 | Low | 30ms | None |
| 16 | Moderate | 60ms | Minimal |
| 32 | High | 120ms | Moderate |

Max recommended: **32 nodes**
