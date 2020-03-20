/*
 *  This program will connect to a central iOT control server and wait on
 *  commands from the server to change the LED state. It will expect commands
 *  of the type:
 *  R000\r\n - Set the red led off.
 *  R073\r\n - Set the red led to 73%.
 *  R100\r\n - Set the red led to 100%
 *  G045\r\n - Set the green led to 45%.
 *  B001\r\n - Set the blue led to 1%.
 *  
 *  Each command will be seperated by a newline.
 *  Any other line will return a status with the format:
 *  "R###G###B###\r\n" which gives the current brightness of each led.
 *  
 *  The EEPROM nonvolatile memory contains the saved network settings.
 *  B0  -  B63  : All Zeroes if unconfigured. Else ssid.
 *  B64 - B127  : All Zeroes if unconfigured. Else password.
 */

#include <WiFi.h>
#include <EEPROM.h>

// Default Wifi Network Settings.
char ssid[64]     = "DavenportWifi";
char password[64] = "peterandkelsie";

// Central iot_server settings.
const char* host = "128.208.1.137";
const int httpPort = 1234;

#define PWM_FREQ 5000
#define PWM_RES 8
#define BLUE_LED_PIN 21
#define BLUE_LED_PWM_CHANNEL 0
#define GREEN_LED_PIN 22
#define GREEN_LED_PWM_CHANNEL 1
#define RED_LED_PIN 23
#define RED_LED_PWM_CHANNEL 2
#define EEPROM_SIZE 128

int r_percent, g_percent, b_percent;

void setup_leds();
void setRedPercentage(int percentage);
void setBluePercentage(int percentage);
void setGreenPercentage(int percentage);
void setLEDS(int red, int green, int blue);
void statusFormat(WiFiClient client, char color, int value);
void ConfigurationMode();

void setup()
{
    // Debug console output.
    Serial.begin(115200);
    delay(10);

    // LED Hardware config.
    setup_leds();

    // Testing configuration mode:
    //ConfigurationMode();

    // Check for a saved congiguration - Load in ssid and password from config.
    /*
    EEPROM.begin(EEPROM_SIZE);
    if(EEPROM.read(0) != 0){
      Serial.println("Eeprom contains config.");
      // Copy ssid from the EEPROM memory.
      for(int i = 0; i < EEPROM_SIZE/2; i++) {
        ssid[i] = EEPROM.read(i);
      }
      // Copy the passowrd from the EEPROM memory.
      for(int i = EEPROM_SIZE/2; i < EEPROM_SIZE; i++) {
        password[i-EEPROM_SIZE/2] = EEPROM.read(i);
      }
    } else {
      // No saved configuration, we need to prompt the user for a config.
      Serial.println("Eeprom does not contain config.");
      ConfigurationMode();
    }*/

    // We start by connecting to a WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

int value = 0;

void loop()
{
    delay(5000);
    ++value;

    Serial.print("connecting to ");
    Serial.println(host);

    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }

    // Send test string.
    client.print("Hi, I am the smart bulb. Send me a message:\r\n");

    while (true) {
  
      // Wait for any delay.
      while (client.available() == 0) {} // Wait for a reponse.
  
      // Read reply lines.
      while(client.available()) {
          int value = 0;
          String line = client.readStringUntil('\r');
          if(line.charAt(0) < ' ') line = line.substring(1);
          if(line.length() == 4){
            switch(line.charAt(0)){
              case 'R':
                setRedPercentage(line.substring(1, 4).toInt());
                break;
              case 'B':
                setBluePercentage(line.substring(1, 4).toInt());
                break;
              case 'G':
                setGreenPercentage(line.substring(1, 4).toInt());
                break;
            }
          } else {
            statusFormat(client, 'R', r_percent);
            statusFormat(client, 'G', g_percent);
            statusFormat(client, 'B', b_percent);
            client.print("\r\n");
          }
      }
    }
    Serial.println();
    Serial.println("closing connection");
}

void statusFormat(WiFiClient client, char color, int value){
  client.print(color);
  if (value<100) client.print('0');
  if (value<10) client.print('0');
  client.print(value);
}

void setup_leds(){
    // Sertup red
    ledcSetup(RED_LED_PWM_CHANNEL, PWM_FREQ, PWM_RES);
    ledcAttachPin(RED_LED_PIN, RED_LED_PWM_CHANNEL);
    // Serup Blue
    ledcSetup(BLUE_LED_PWM_CHANNEL, PWM_FREQ, PWM_RES);
    ledcAttachPin(BLUE_LED_PIN, BLUE_LED_PWM_CHANNEL);
    // Setup Green
    ledcSetup(GREEN_LED_PWM_CHANNEL, PWM_FREQ, PWM_RES);
    ledcAttachPin(GREEN_LED_PIN, GREEN_LED_PWM_CHANNEL);
    // Turn all LEDs off.
    setLEDS(0,0,0);
}

void setLEDS(int red, int green, int blue){
    setRedPercentage(red);
    setGreenPercentage(green);
    setBluePercentage(blue);
}

void setRedPercentage(int percentage){
    r_percent = percentage;
    int value = 256 - percentage*254/100;
    ledcWrite(RED_LED_PWM_CHANNEL, value);
    Serial.print("R");
    Serial.println(value);
}

void setBluePercentage(int percentage){
    b_percent = percentage;
    int value = 256 - percentage*254/100;
    ledcWrite(BLUE_LED_PWM_CHANNEL, value);
    Serial.print("B");
    Serial.println(value);
}

void setGreenPercentage(int percentage){
    g_percent = percentage;
    int value = 256 - percentage*254/100;
    ledcWrite(GREEN_LED_PWM_CHANNEL, value);
    Serial.print("G");
    Serial.println(value);
}

void ConfigurationMode(){
  // Create a wifi access point caled 'iOT Bulb Setup'.
  const char *setup_ssid = "iOT Bulb Setup";
  WiFiServer server(80);
  WiFi.softAP(ssid);
  IPAddress myIP = WiFi.softAPIP();
  server.begin();

  bool unconfigured = true;
  while(unconfigured){
    WiFiClient client = server.available();
    if (client) {
        String currentLine = "";
        while (client.connected()) {
          if (client.available()) {
            char c = client.read();
            Serial.write(c);
            if (c == '\n') {
              // if the current line is blank, you got two newline characters in a row.
              // that's the end of the client HTTP request, so send a response:
              if (currentLine.length() == 0) {
                // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                // and a content-type so the client knows what's coming, then a blank line:
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                client.println();
    
                // the content of the HTTP response follows the header:
                client.print("<form action=\"\" method=\"get\">");
                client.print("<ul><li><label for=\"ssid\">ssid:</label><input type=\"text\" id=\"ssid\" name=\"ssid\"></li>");
                client.print("<li><label for=\"pass\">pass:</label><input type=\"text\" id=\"pass\" name=\"pass\"></li>");
                client.print("<li class=\"button\"><button type=\"submit\">Configure.</button></li></ul>");
                client.print("</form>");
    
                // The HTTP response ends with another blank line:
                client.println();
                // break out of the while loop:
                break;
              } else {    // if you got a newline, then clear currentLine:
                currentLine = "";
              }
            } else if (c != '\r') {  // if you got anything else but a carriage return character,
              currentLine += c;      // add it to the end of the currentLine
            }
          }
        }
        // close the connection:
        client.stop();
      }
  }
}
