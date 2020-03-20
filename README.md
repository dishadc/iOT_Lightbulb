# iOT_Lightbulb

# Archetecture
iOT Lightbulbs have three colors, each can be set to any brightness between 0% and 100%.
We have a central server, which we run on *attu1*, which has ip address *128.208.1.137*. The server can be run by running 'python iot_server.py' from inside the 'frontend' directory.
The central server both maintains a web-application (access from a browser at http://128.208.1.137:8000/ while the server is running) and proxies color change PUT requests to the iOT light by sending commands over TCP to the light. The light is a client that attached to the server on port 1234.