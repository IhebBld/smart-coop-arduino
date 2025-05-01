# Smart Chicken Coop - Arduino & Proteus Project

An automated chicken coop system built with Arduino (simulated in Proteus) that handles feeding, security, and environmental control.

## Features
1. **Food & Water Management**  
   - Automated food and water refilling system for chickens.

2. **Security System**  
   - Automatic door operation (opens at dawn, closes at dusk)  
   - Predator detection with alert system  

3. **Environment Control**  
   - Temperature and humidity monitoring and regulation  

## Repository Structure
- **`Project/`**  
  Final Proteus simulation with all systems integrated.
- **`Tests/`**  
  Features prototypes:
  - `Food/` → Food/water system test
  - `Security/` → Door & predator alerts test
  - `Temperature/` → Climate control test

## Proteus Libraries Used
- **DS1307 (RTC)**: [Download & Guide](https://www.theengineeringprojects.com/2016/04/ds1307-library-proteus-2.html)  
- **Water Level Sensor**: [Download & Guide](https://www.theengineeringprojects.com/2020/07/water-sensor-library-for-proteus.html)  

## Getting Started
1. Install the required Proteus libraries (links above)
2. Open the project in Proteus
3. Run the simulation

> Note: This is a Proteus simulation - for real-world implementation, additional hardware considerations may be needed.