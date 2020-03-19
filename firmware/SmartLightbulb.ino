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
 */

#include <WiFi.h>

// Default Wifi Network Settings.
const char* ssid     = "DavenportWifi";
const char* password = "peterandkelsie";

// Central server settings.
const char* host = "128.208.1.137";
const int httpPort = 1234;

// LED Pins.
#define PWM_FREQ 5000
#define PWM_RES 8
#define BLUE_LED_PIN 21
#define BLUE_LED_PWM_CHANNEL 0
#define GREEN_LED_PIN 22
#define GREEN_LED_PWM_CHANNEL 1
#define RED_LED_PIN 23
#define RED_LED_PWM_CHANNEL 2

int r_percent, g_percent, b_percent;

void setup_leds();
void setRedPercentage(int percentage);
void setBluePercentage(int percentage);
void setGreenPercentage(int percentage);
void setLEDS(int red, int green, int blue);
void statusFormat(WiFiClient client, char color, int value);

void setup()
{
    setup_leds();

    Serial.begin(115200);
    delay(10);

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
          if(line.length() == 5){
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
    int value = 254 - percentage*254/100;
    ledcWrite(RED_LED_PWM_CHANNEL, value);
}

void setBluePercentage(int percentage){
    b_percent = percentage;
    int value = 254 - percentage*254/100;
    ledcWrite(BLUE_LED_PWM_CHANNEL, value);
}

void setGreenPercentage(int percentage){
    g_percent = percentage;
    int value = 254 - percentage*254/100;
    ledcWrite(GREEN_LED_PWM_CHANNEL, value);
}
