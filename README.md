## Members
- Dishad Chopra
- Peter Davenport
- Ryan Bartruff

## Overview
For our project, we have made a smart controllable lightbulb. We use a ESP32-DEVKITC-32D board to control an RGB led strip. The client is able to remote control the color of the led strip by selecting a color on our application which then sends a message to the server which then communicates with the board to change the color of the led.
After selecting a color, when the client clicks the On button an HTTP PUT request is sent to the server running on Attu, which is received at our iot_server application which then generates a command that is sent out over tcp to the iOT bulb device. Firmware on the bulb device interprets these commands and changes the brightness of the led strip based on the command. After turning the light on the button changes to an off button which the client can press to turn off the light.
We chose to connect to a central server application for two reasons. First this creates a single place where the user can navigate to using their web browser on any device, no matter where they are in the world. Second, having the iOT bulb connect to a specific static ip address allows us to use the iOT bulb in locations that do not have static IP addresses such as the typical home.

## Software Stack
- HTML/CSS/Javascript single page web-application.
- Python central server that both serves the webapp to the browser, and connects to the bulb hardware.
- C firmware on the iOT Lightbulb.

## Instructions for use:
- Install the firmware on the esp32 using the arduino ide.
- Configure the network settings by:
    - Connecting to the "iOT Bulb" access point.
    - Navigating to "http://192.168.4.1"
    - Putting in your local network properties.
- Run the iot_server.py with python2 (not 3).
    - Only run it on attu1.
    - If another instance is running on the ports 8000 or 1234 you need to close that.
    - Run it using the command "python iot_server.py" from inside the front_end dir.
- Navigate to http://128.208.1.137:8000/ to use the webapplication.
