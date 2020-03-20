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
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <EEPROM.h>

// Default Wifi Network Settings.
const char *setup_ssid = "iOT Bulb";
bool isConfigured = false;
char ssid[64];
char password[64];


// Central iot_server settings.
const char* host = "128.208.1.137";
const int httpPort = 1234;

// Hardware defines.
#define PWM_FREQ 5000
#define PWM_RES 8
#define BLUE_LED_PIN 21
#define BLUE_LED_PWM_CHANNEL 0
#define GREEN_LED_PIN 22
#define GREEN_LED_PWM_CHANNEL 1
#define RED_LED_PIN 23
#define RED_LED_PWM_CHANNEL 2
#define EEPROM_SIZE 128

// Last percentage set.
int r_percent, g_percent, b_percent;

// Server for network setup.
WiFiServer server(80);

// Function Primitives.
void setup_leds();
void setRedPercentage(int percentage);
void setBluePercentage(int percentage);
void setGreenPercentage(int percentage);
void setLEDS(int red, int green, int blue);
void statusFormat(WiFiClient client, char color, int value);
void ConfigurationMode();
void setup_app();
void save_network_settings();
void clear_network_settings();
void load_network_settings();
void(* resetFunc) (void) = 0;

void setup() {
  // Setup main hardware.
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  setup_leds();
  Serial.println("Hardware Started.");

  // Check for prior network settings.
  load_network_settings();

  if(isConfigured){
    Serial.println("Loaded configuration from memory.");
    Serial.print("ssid: ");
    Serial.println(ssid);  
    Serial.print("pass: ");
    Serial.println(password);
  } else {
    Serial.println("Need to configure.");
  }

  // You can remove the password parameter if you want the AP to be open.
  if(!isConfigured){
    // Set up access point for user to connect to.
    WiFi.softAP(setup_ssid);
    IPAddress myIP = WiFi.softAPIP();
    server.begin();

    // Host a server that prompts the user for their network info.
    while(isConfigured != true){
      setup_app();
    }
  }

    // Connect to a wifi network.
    WiFi.begin(ssid, password);
    int time_to_find = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        time_to_find++;
        if(time_to_find > 30){
          isConfigured = false;
          clear_network_settings();
          resetFunc(); // Reset, unable to connect to this network.
        }
        Serial.print(".");
    }

    // Save these settings for next time.
    save_network_settings();
    Serial.println("\r\nWiFi connected");
}

void loop() {
    delay(5000);

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

void setup_app(){
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            
            // HTTP Contents - HTML page with login form.
            client.print("<h3>Input your network name (ssid) and passoword.</h3><form action=\"\" method=\"get\">");
            client.print("<ul><li><label for=\"ssid\">ssid:</label><input type=\"text\" id=\"ssid\" name=\"ssid\"></li>");
            client.print("<li><label for=\"pass\">pass:</label><input type=\"text\" id=\"pass\" name=\"pass\"></li>");
            client.print("<li class=\"button\"><button type=\"submit\">Configure.</button></li></ul>");
            client.print("</form>");
            client.println(); // end http request.
            break;
          } else {
              // Newline, so check what the contents of this line were.
              // If they were the GET line containing the configuration data set that data.
              if(currentLine.startsWith("GET /?ssid=")){
              String ssid_in;
              String pass_in;
              ssid_in = currentLine.substring(11);
              int pass_start = ssid_in.indexOf('=');
              int pass_end = ssid_in.indexOf(' ');
              pass_in = ssid_in.substring(pass_start+1, pass_end);
              ssid_in = ssid_in.substring(0, pass_start-5);
              Serial.print("ssid = ");
              Serial.println(ssid_in);
              Serial.print("pass = ");
              Serial.println(pass_in);
              ssid_in.toCharArray(ssid, 64);
              pass_in.toCharArray(password, 64);
              isConfigured = true;
            }
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    client.stop();
  }
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
}

void setBluePercentage(int percentage){
    b_percent = percentage;
    int value = 256 - percentage*254/100;
    ledcWrite(BLUE_LED_PWM_CHANNEL, value);
}

void setGreenPercentage(int percentage){
    g_percent = percentage;
    int value = 256 - percentage*254/100;
    ledcWrite(GREEN_LED_PWM_CHANNEL, value);
}

void save_network_settings(){
  for(int i = 0; i < EEPROM_SIZE/2; i++) {
    EEPROM.write(i, ssid[i]);
  }
  for(int i = EEPROM_SIZE/2; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, password[i-EEPROM_SIZE/2]);
  }
  EEPROM.commit();
}

void clear_network_settings(){
  for(int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
   EEPROM.commit();
}

void load_network_settings(){
  if(EEPROM.read(0) != 0){
    // Copy the ssid from memory.
    for(int i = 0; i < EEPROM_SIZE/2; i++) {
      ssid[i] = EEPROM.read(i);
    }
    // Copy the password from the EEPROM memory.
    for(int i = EEPROM_SIZE/2; i < EEPROM_SIZE; i++) {
      password[i-EEPROM_SIZE/2] = EEPROM.read(i);
    }
    isConfigured = true;
  }
}
