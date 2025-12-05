# Energy Efficiency

## Duty Cycling
Nodes operate:

- **Active:** 1.698 ms  
- **Sleep:** 9,998.3 ms  
- **Duty Cycle:** 0.017%  
- **Average Current:** 48.75 µA  

---

## Power Breakdown

| Phase | Duration | Current | Energy |
|-------|------------|----------|-----------|
| Deep Sleep | 9998.3ms | 8 µA | 0.0800 mAs |
| Wake | 0.5ms | 80 mA | 0.0400 mAs |
| DHT11 Read | 1ms | 120 mA | 0.1200 mAs |
| ESP-NOW TX | 1.7ms | 240 mA | 0.4080 mAs |

---

## Battery Life
Using 1500mAh:

**Lifetime = 3.51 years**

---

## RTC Memory Saves
- Pairing state  
- Gateway MAC  
- Interval  
- LED state  
