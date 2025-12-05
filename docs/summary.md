# System Summary

| Parameter | Specification |
|----------|---------------|
| Network Topology | Star (4 sensor nodes + 1 gateway) |
| Microcontroller | ESP32-C3 (RISC-V 160MHz) |
| Sensors | DHT11 temperature & humidity |
| Visual Feedback | NeoPixel WS2812B RGB LED |
| Local Communication | ESP-NOW (AES-128 encrypted) |
| Cloud Communication | MQTT (HiveMQ) |
| Power Source | USB for development |
| Projected Battery Life | 3.5 years |
| Alert Latency | 15ms local, 200ms end-to-end |
| Duty Cycle | 0.017% |
| Max Nodes | 32 |

The network consists of four sensor nodes and one gateway, designed for distributed monitoring and synchronized alert propagation.
