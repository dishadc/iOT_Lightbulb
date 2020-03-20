# iOT_Lightbulb
Project #3 for CSE461. Implementation of a iOT lightbulb.

## TODO:
* Peter: Finish dynamic user-definable wifi network configuration.
    * Configuration will start with the user accessing the iOT bulb as a wifi access point.
    * Once connected to the device the user will navigate to some ip addess.
    * The device will present a menu with two boxes to insert a wifi ssid and password.
    * Once these are submitted they will be saved on the device in nonvolatile memory.
    * The device will then restart, read the saved wifi settings and try to connect to that wifi network.
    * If the wifi network cannot be connected to the device will reset into "uncongifured" mode.
* Dishad: Help finish the iot_server.py
    * Read through this document and the code to get an understaning of the system so far.
    * Run iot_server.py on attu1 (from the frontend directory) and try to access the webapp at http://128.208.1.137:8000/
    * When you click the Switch button note that the iot_server should print out the color you have selected.
    * Use netcat to act like you are a iOT light bulb.
        * run 'nc 128.208.1.137 -p 1234' to connect to the iOT server as if you are the lightbulb. You will then be able to verify that you recive the color change messages such as "R047" etc.
    * Now you should be able to help us improve the server. There are the following problems with it right now:
        * There is no clean way to close the server in the current program. If you controll-C the thread that is running the webserver crashes.
        * We need to generate output messages on port 1234 when we recive PUT requests on port 8000, right now we just print out the PUT message body.
        * We need to be able to re-attach the iOT bulb incase it restarts... Currently the code only supports a single connection.
        * The server is fairly hacky - various corner cases should be addressed.
* Ryan:
    * First draft of project report.
    * Possibly investigate if we can display the current color chosen to the user of the webapp (i.e. request the status from the iOT bulb, pass it back to the webapp via the server, and display it in the webapp).
    * Possibly investigate if we can display connection status in the webapp (if the server is connected to a iOT bulb or not).

## Hardware
The iOT Lightbulb has 3 colors (Red, Green, Blue) which can be set to any brighness between 0%-100%. The bulbs are controlled by a ESP-32S "NodeMCU v1.1" develoupment kit.

## Archetecture
This project has the following components.
* iOT Lightbulb Hardware running on ESP-32S hardware.
* firmware running on the ESP-32S which:
    * Connects the ESP-32S to the local wifi network.
    * Connects to 128.208.1.137:1234.
    * Changes brightness when it recives messages such as:
        * R100  # Turn on the red light full brighness.
        * R050  # Turn on the red light half brighness.
        * R000  # Turn off the red light.
        * B037  # Turn on the blue light 37%.
        * G012  # Trun on the green light 12%.
        * Any Other Message # Respond with the status (brightnesses) of all lights in the format "R###G###B###\r\n"
* A central iot_server that:
    * Hosts the webapplication that allows the color/brighness to be picked. (HTML and Javascript)
    * Converts HTTP PUT requests (on port 8000) that contain new color selections to commands sent to the iOT light over port 1234.

## System User Process:
1. Client turns on their iOT Light.
2. TODO: Need to finish system for the client to be able to dynamically change the wifi network they attach too. Peter has a prototype of this already.
3. Client navigates to 'http://128.208.1.137:8000/'
4. Client selects and submits new color choices.

## Data Movement in the system:
1. Users click submit on the webapp, generating a HTTP PUT request on 128.208.1.137:8000.
2. The HTTP PUT Request travels over the internet to attu1 (128.208.1.137) and is recived on port 8000 at our iot_server.py app.
3. The new color in the body of the PUT request is then converted to percentage values for each color channel.
4. The iot_server.py sends out the new colors (for example "R034" "G023" "B012") to the iOT bulb on port 1234.
5. The iOT bulb recives the TCP data "R034" "G023" "B012" and then decodes these messages to set the brightness of the LED's.
