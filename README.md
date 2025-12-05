# Cooperative Thermal Alert Network (CTAN)
A distributed thermal-monitoring system using **ESP32-C3**, **ESP-NOW**, **HiveMQ Cloud (MQTT)**, and **Node-RED**.  
Multiple sensor nodes measure temperature, send data wirelessly to a gateway, and display real-time dashboards and alerts.

---

## Overview
The **Cooperative Thermal Alert Network (CTAN)** monitors temperature across multiple locations using ESP32-C3 sensor nodes equipped with DHT11 sensors.  
Nodes communicate with the gateway using **ESP-NOW**, a lightweight and WiFi-free protocol ideal for low-power IoT systems.

The gateway publishes collected data securely to **HiveMQ Cloud**, and **Node-RED** subscribes to these MQTT topics to visualize real-time readings and historical trends.

This system is suitable for:
- Multi-room temperature monitoring  
- IoT learning and experimentation  
- Low-power wireless sensor networks  
- Distributed alert systems  

---

##  Features
- 4 ESP32-C3 sensor nodes  
- Lightweight ESP-NOW wireless communication  
- Secure cloud MQTT communication using **HiveMQ Cloud**  
- Real-time dashboards for each node  
- Historical temperature charts  
- Fully scalable architecture  

---

## System Architecture

```
ESP32-C3 Sensor Nodes (DHT11 + NeoPixel)
            │
            │  ESP-NOW
            ▼
       ESP32-C3 Gateway
            │
            │  MQTT (TLS)
            ▼
       HiveMQ Cloud Broker
            │
            │  MQTT Subscribe
            ▼
        Node-RED Dashboard
```

---

# Getting Started

## Requirements
- ESP32-C3 boards  
- DHT11 temperature sensors  
- HiveMQ Cloud (Free Tier) MQTT account  
- Node-RED installed (PC, Laptop, or Raspberry Pi)  
- USB programming cable  

---

## 🔌 HiveMQ Cloud Setup (MQTT)

1. Visit **https://console.hivemq.cloud/**  
2. Create a **Free Cluster**  
3. Create MQTT credentials (username + password)  
4. Copy your cluster hostname, e.g.:
   ```
   a12b345cdefg.s1.eu.hivemq.cloud
   ```
5. Use port **8883** for secure TLS communication  
6. Update the Gateway ESP32-C3 code:

```cpp
const char* mqtt_server = "your-hivemq-cloud-hostname";
const int mqtt_port = 8883;
const char* mqtt_user = "your-username";
const char* mqtt_password = "your-password";
```

---

## Installing and Running the Project

### Clone the repository
```sh
git clone https://github.com/BK65t/Cooperative-Thermal-Alert-Network-using-ESP32-C3-ESP-NOW-MQTT-and-Node-RED.git
```

### Flash code to sensor nodes
- Open `/Code` folder  
- Upload the **node firmware** to each ESP32-C3  
- Configure each node with a unique ID  

### Flash code to the gateway
- Upload the **gateway firmware**  
- Make sure your HiveMQ credentials are correct  

### Set up Node-RED
- Import the flow from `/Node-RED Process flow`  
- Ensure the MQTT nodes use **HiveMQ Cloud (TLS)** with your credentials  
- Deploy the flow  

### View your dashboard
Open your browser:

```
http://localhost:1880/ui
```

You will now see live temperature monitoring.

---

# Folder Structure

```
/
├── Code/                         # Firmware for nodes and gateway
├── Node-RED Process flow/        # Node-RED flow JSON
├── Node-RED dashboard/           # Screenshots used in README
├── LICENSE                       # MIT License
└── README.md                     # Documentation
```

---

# Node-RED Dashboard Screenshots

## Node Dashboards  

### **Node 1 Dashboard**
![Node 1 Dashboard](Node-RED%20dashboard/Node%201%20dashboard.jpg)

### **Node 2 Dashboard**
![Node 2 Dashboard](Node-RED%20dashboard/Node%202%20dashboard.jpg)

### **Node 3 Dashboard**
![Node 3 Dashboard](Node-RED%20dashboard/Node%203%20dashboard.jpg)

### **Node 4 Dashboard**
![Node 4 Dashboard](Node-RED%20dashboard/Node%204%20dashboard.jpg)

---

## Historical Temperature Charts  

### **Node 1 & Node 2 Historical Chart**
![Historical Chart Node 1 and 2](Node-RED%20dashboard/Historical%20chart%20of%20Node%201%20and%20Node%202.jpg)

### **Node 3 & Node 4 Historical Chart**
![Historical Chart Node 3 and 4](Node-RED%20dashboard/Historical%20chart%20of%20Node%203%20and%20Node%204.jpg)

---

# License
This project is licensed under the **MIT License**.

