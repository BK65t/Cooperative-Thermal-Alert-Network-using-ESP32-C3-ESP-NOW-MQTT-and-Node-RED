# Hardware Specifications

## Node Components

| Component | Model | Details |
|----------|--------|---------|
| Microcontroller | ESP32-C3 | 32-bit RISC-V, 160MHz, 4MB Flash |
| Temperature Sensor | DHT11 | ±2°C accuracy |
| Humidity Sensor | DHT11 | ±5% RH accuracy |
| LED | WS2812B | RGB, programmable |
| Sleep Current | ESP32-C3 | 8 µA |
| TX Current | ESP32-C3 | 240 mA |

---

## LED Colour Meanings  
- **Red:** Critical hot alert (≥30°C)  
- **Yellow:** Hot warning (≥25°C)  
- **Green:** Safe / normal  
- **Turquoise:** Cold warning (≤15°C)  
- **Purple:** Sensor error  

Designed for long-term battery-powered deployment.
