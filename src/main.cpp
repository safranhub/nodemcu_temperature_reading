/********************************************************************/
/*PROJECT: NodeMCU Temperature Reading -----------------------------*/
/********************************************************************/
/*About project:  Show temperature sensor reading every second      */
/*                in HomeAssistant that is installed on a PC.       */
/*                If temperature is higher than 30 set Alarm on     */
/*                HomeAssistant to :( and turn on RED LED.          */
/********************************************************************/

#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
/********************************************************************/
// Data wire for red LED (5 = D1 pin on ESP8266)

#define LED1 5
/********************************************************************/
// Data wire for temperature reading (4 = D2 pin on ESP8266)

#define ONE_WIRE_BUS 4
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)

OneWire oneWire(ONE_WIRE_BUS);
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature.

DallasTemperature sensors(&oneWire);
/********************************************************************/
// Declare Ticker object (we use flip() function so we wont affect
// other sensors in lopp() function if we add them in the future)

Ticker flipper;
/********************************************************************/
// Set mqtt server IP and client objects (set your IP)

const char* mqtt_server = "192.168.0.113";
WiFiClient espClient;
PubSubClient client(espClient);
/********************************************************************/
// Variables for temperature (in °C), flipper seconds and serial port

float celsius; // Temperature reading
char cels[8]; // Dloat to char
float flip_seconds = 1; // Send temperature reading every second
int serial_port = 9600; // Monitoring on port 9600
/********************************************************************/
// Flip function

void flip(){
  /********************************************************************/
  // Flip function for measurement (every 'flip_seconds' seconds)

  // MQTT connection with HomeAssistant
  if (!client.connect("Temperature")) {
    Serial.println("Not connected.");
    client.connect("Temperature");
  }

  Serial.print(" Requesting temperatures...");
  // Send the command to get temperature readings
  sensors.requestTemperatures();
  Serial.println("DONE");

  Serial.print("Temperature is: ");
  Serial.print(sensors.getTempCByIndex(0));
  celsius=sensors.getTempCByIndex(0);
  dtostrf(celsius, 6, 2, cels);
  // Send temperature reading to HomeAssistant
  client.publish("outTopic", cels);
  // If temperature is higher than 30 send warning [:(] and turn LED on
  if(celsius>30){
  client.publish("outTopic3", ":(");
  digitalWrite(LED1, HIGH);
}else{
  digitalWrite(LED1, LOW);
  client.publish("outTopic3", ":)");
}

}
/********************************************************************/
// Setup function
// Turn sensor on, set and establish WiFi connection,
// set server info and connect

void setup()
{
  /********************************************************************/
  // Setup LED

  pinMode(LED1, OUTPUT);
  digitalWrite(LED1, LOW);

  // Setup WiFi and connect

  // Start serial port on 'serial_port'
  Serial.begin(serial_port);
  Serial.println("Temperature Reading - Project");

  // Start up the library
  sensors.begin();

  // Connect to WiFi AP
  WiFi.begin("SSID", "password");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  // Set server IP, connect and send sensor readings
  client.setServer(mqtt_server, 1883);

  if (client.connect("Temperature")) {
    Serial.print("Connected to MQTT server!");
  }else{
    Serial.print("Not connected!");
  }

    // Set period of flipper repeating in seconds (e.g. every 1 second)
  flipper.attach(flip_seconds, flip);

}
/********************************************************************/
// Loop function

void loop()
{
  /********************************************************************/
  // Allow the client to process incoming messages to send publish data
  // and makes a refresh of the connection

  client.loop();

}
/********************************************************************/
/*END---------------------------------------------------------------*/
/********************************************************************/
