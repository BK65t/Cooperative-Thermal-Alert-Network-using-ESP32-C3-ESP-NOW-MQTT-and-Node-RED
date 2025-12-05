# Cooperative Thermal Alert Network (CTAN) 

## Overview  
The Cooperative Thermal Alert Network (CTAN) uses ESP32-C3 devices, DHT11 temperature sensors, and NeoPixel LEDs to monitor temperature across multiple sensor nodes. Nodes communicate locally using ESP-NOW. A gateway aggregates data from all nodes, forwards it via MQTT, and visualizes it on Node-RED. With a duty cycle of ~0.017%, the network is designed for low-power and efficient temperature monitoring and alerting.  

## Features  
- Distributed temperature sensing using multiple ESP32-C3 nodes  
- Local communication between nodes over ESP-NOW (no WiFi infrastructure required for nodes)  
- Central gateway that publishes data to MQTT broker  
- Real-time visualization and alerting via Node-RED dashboard  
- Low-power node operation thanks to duty-cycle scheduling  

## System Architecture  
*(Optional: Insert a small diagram or ASCII art of how sensor nodes → gateway → MQTT → Node-RED relate.)*  

```
Sensor Node (ESP32-C3 + DHT11 + NeoPixel)  ── ESP-NOW ➜  Gateway (ESP32-C3)  ── MQTT ➜  Node-RED Dashboard / Alerts
```

## Getting Started  

### Prerequisites  
- ESP32-C3 microcontroller boards  
- DHT11 sensors for each node  
- NeoPixel LED (or similar LED) for alert indication (optional)  
- MQTT broker (local or cloud)  
- Computer running Node-RED for dashboard  

### Installation & Setup  
1. Clone this repository:  
   ```sh
   git clone https://github.com/BK65t/Cooperative-Thermal-Alert-Network-using-ESP32-C3-ESP-NOW-MQTT-and-Node-RED.git
   ```  
2. Flash sensor-node firmware onto each ESP32-C3 board (see `/Code` folder).  
3. Flash gateway firmware onto one ESP32-C3 board (also in `/Code`).  
4. Configure MQTT broker credentials and broker URL in the gateway code.  
5. Start your MQTT broker.  
6. Import and run the Node-RED flow in `/Node-RED Process flow`.  
7. Open the Node-RED dashboard (`/Node-RED dashboard`) to see real-time readings and alerts.  

## Usage Example  
Once set up and running:  
- Sensor nodes periodically measure temperature and send data to the gateway via ESP-NOW.  
- The gateway publishes data to MQTT.  
- Node-RED subscribes to MQTT topics, displays temperature data on the dashboard, and triggers alerts (e.g., LED on sensor node) when thresholds are exceeded.  

*(Add screenshots of the Node-RED dashboard, sensor data, or alert LEDs below.)*  

## Demo / Screenshots  
![Node-RED Dashboard](images/node-red-dashboard.png)  
![Sensor Node Setup](images/sensor-node-esp32-dht11.png)  
_(Add more relevant screenshots)_  

## Folder Structure  

```
/
├── Code/                      # ESP32-C3 firmware for nodes & gateway  
├── Node-RED Process flow/     # .json or flow definition for Node-RED  
├── Node-RED dashboard/        # Dashboard UI definition / config   
├── LICENSE                    # Project license (MIT)  
└── README.md                  # This file  
```

## Contributing  
Feel free to fork the project and submit pull-requests. If you find issues or have suggestions — please open an issue.  

## License  
This project is licensed under the terms of the MIT License.  
