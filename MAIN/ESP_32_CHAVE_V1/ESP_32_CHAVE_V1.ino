#include <WiFi.h>
#include <PubSubClient.h>




unsigned long previousMillisLightSound = 0;
unsigned long previousMillisClimate = 0;
const long intervalLightSound = 1000;
const long intervalClimate = 50;
// MQTT Configuration
const char* ssid = "Bepes";
const char* password = "aleix1234567890";
const char* mqttServer = "172.20.10.5";
const int mqttPort = 1888;
const char* mqttUser = "public";
const char* mqttPassword = "public";
//int lights[3] = {25, 26, 27};
int lights[3] = {1, 3, 32};
int lightSensor = 12; // min 3500, max 0
int ledBar[10] = {21, 19, 18, 5, 17, 16, 4, 0, 2, 15}; // from led0 (red) to led9 (white)
int soundSensor = 14; // NEEDS TO BE RECALIBRATED
int heatLed = 23;
int heatRele = 33;
int heatButton = 36;
int coldLed = 22;
int coldButton = 34;
int coldMotor = 13;
int neutralButton = 39;
int lastHeatButtonState = LOW;
int lastColdButtonState = LOW;
int lastNeutralButtonState = LOW;
int minSound = 1024;
int maxSound = 0;




WiFiClient espClient;
PubSubClient client(espClient);


void setup() {
  Serial.begin(115200);
  pinMode(lightSensor, INPUT);
  for(int i = 0; i < 3; i++){
    pinMode(lights[i], OUTPUT);
    digitalWrite(lights[i], LOW);
  }
  pinMode(soundSensor, INPUT);
  for(int i = 0; i < 10; i++){
    pinMode(ledBar[i], OUTPUT);
    digitalWrite(ledBar[i], LOW);
  }
  pinMode(heatLed, OUTPUT);
  digitalWrite(heatLed, LOW);
  pinMode(heatRele, OUTPUT);
  digitalWrite(heatRele, LOW);
  pinMode(heatButton, INPUT);
  pinMode(coldLed, OUTPUT);
  digitalWrite(coldLed, LOW);
  pinMode(coldMotor, OUTPUT);
  pinMode(coldButton, INPUT);
  pinMode(neutralButton, INPUT);
  
  WiFi.begin(ssid, password);
  Serial.println("...................................");
  Serial.print("Connecting to WiFi.");
  pinMode(lightSensor, INPUT);
  while (WiFi.status() != WL_CONNECTED){
          delay(500);
          Serial.print(".") ;
  }

  Serial.println("Connected to the WiFi network");

  client.setServer(mqttServer, mqttPort);
  while (!client.connected()){ 
         Serial.println("Connecting to MQTT...");
       if (client.connect("ESP32Client", mqttUser, mqttPassword ))
           Serial.println("connected");
       else
       {   Serial.print("failed with state ");
           Serial.print(client.state());
           delay(2000);
       }
  }
}
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisLightSound >= intervalLightSound) {
    previousMillisLightSound = currentMillis;
    int soundSensorValue = analogRead(soundSensor);
    Serial.print("Sound ");
    Serial.println(soundSensorValue, DEC);
    if(soundSensorValue < minSound){
      minSound = soundSensorValue;
    }
    else if(soundSensorValue > maxSound){
      maxSound = soundSensorValue;
    }
    Serial.print("SOUND VALUES - Min: ");
    Serial.print(minSound, DEC);
    Serial.print(" Max: ");
    Serial.println(maxSound, DEC);
    int lightSensorValue = analogRead(lightSensor);
    client.loop();
    Serial.println(lightSensorValue, DEC);
  if (!isnan(lightSensorValue)) {
    char str[16];
    sprintf(str, "%i", lightSensorValue);
    client.publish("sensors/light/1", str);
    Serial.println(str);
  }
     delay(500);
    Serial.print("Light ");
    // > 2500 turns on the lights | < 250 turns off the lights | in between attenuate the lights depending on the light
    int lightOutputValue = 0;
    if(lightSensorValue > 2500){
      lightOutputValue = 255;
    }
    else if(lightSensorValue < 250){
      lightOutputValue = 0;
    }
    else {
      lightOutputValue = map(lightSensorValue, 3500, 0, 255, 0);
    }
    Serial.print("Light Output ");
    Serial.println(lightOutputValue, DEC);
    int ledsToLight = map(soundSensorValue, 0, 350, 0, 10); // sensor, minValue, maxValue, minLed, maxLed
    for (int i = 0; i < 3; i++) {
      analogWrite(lights[i], lightOutputValue);
    }
    for (int i = 0; i < 10; i++) {
      if (i < ledsToLight) {
        digitalWrite(ledBar[i], HIGH);
      } else {
        digitalWrite(ledBar[i], LOW);
      }
    }
  }
  if (currentMillis - previousMillisClimate >= intervalClimate) {
    previousMillisClimate = currentMillis;
    int heatButtonState = digitalRead(heatButton);
    int coldButtonState = digitalRead(coldButton);
    int neutralButtonState = digitalRead(neutralButton);
    if (heatButtonState == HIGH && lastHeatButtonState == LOW) {
      digitalWrite(heatLed, HIGH);
      digitalWrite(heatRele, HIGH);
      digitalWrite(coldLed, LOW);
      digitalWrite(coldMotor, LOW);
    } else if (coldButtonState == HIGH && lastColdButtonState == LOW) {
      digitalWrite(coldLed, HIGH);
      digitalWrite(coldMotor, HIGH);
      digitalWrite(heatLed, LOW);
      digitalWrite(heatRele, LOW);
    } else if (neutralButtonState == HIGH && lastNeutralButtonState == LOW) {
      digitalWrite(heatLed, LOW);
      digitalWrite(heatRele, LOW);
      digitalWrite(coldLed, LOW);
      digitalWrite(coldMotor, LOW);
    }
    lastHeatButtonState = heatButtonState;
    lastColdButtonState = coldButtonState;
    lastNeutralButtonState = neutralButtonState;
  }
}