#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>


//Define pitidos
#define A4  440
#define B4  494
#define C5  523
#define D5  587
#define E5  659
#define F5  698
#define G5S 830 // G#5
#define A5  880
#define B5  988
#define C6  1047
#define D6  1175
#define CORCHEA 150  // Duración de una corchea
#define NEGRA 300    // Duración de una negra
#define NEGRAPUNTO 450 // Duración de una negra con punto
#define BLANCA 600   // Duración de una blanca
#define REDONDA 1200 // Duración de una redonda

const char* ssid = "Bepes";
const char* password = "aleix1234567890";
const int mqttPort = 1888;
const char* mqttUser = "public";
const char* mqttPassword = "public";
const char* mqtt_server = "172.20.10.5";


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

int heatRele = 33; 
int coldMotor = 13;
int fiestaMode = 12;
int BUZZER_PIN = 14;
int coldLed = 22;
int heatLed = 23;
int sensorSoroll = 36;
int sensorLlum = 39;
bool lastLightState = false;


bool lightsState = false;

int lights[6] = {1,3,32,25,26,27};
int ledBar[10] = {21, 19, 18, 5, 17, 16, 4, 0, 2, 15}; // from led0 (red) to led9 (white)

void setup() {

  Serial.begin(115200);

  for (int i = 0; i < 6; i++) {
    pinMode(lights[i], OUTPUT);
    digitalWrite(lights[i], LOW);
  }
    for(int i = 0; i < 10; i++){
    pinMode(ledBar[i], OUTPUT);
    digitalWrite(ledBar[i], LOW);
  }

  pinMode(heatRele, OUTPUT);
  pinMode(coldMotor, OUTPUT);
  pinMode(fiestaMode, OUTPUT);
  pinMode(coldLed, OUTPUT);
  pinMode(heatLed, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(sensorLlum, INPUT);
  pinMode(sensorSoroll, INPUT);

  // Initialize WiFi connection
  setup_wifi();

  // Set up MQTT client
  client.setServer(mqtt_server, mqttPort);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
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
void callback(char* topic, byte* message, unsigned int length) {
  String messageTemp;
  for (int i = 0; i < length; i++){
    messageTemp += (char)message[i];
  }
  Serial.print(messageTemp);
  if (strcmp(topic,"actuators/fans/1") == 0){
    if (messageTemp == "1") {
      digitalWrite(coldMotor, HIGH);
      digitalWrite(coldLed, HIGH);
    }
    else{
        digitalWrite(coldLed, LOW);
        digitalWrite(coldMotor, LOW);
    }
  }
  if (strcmp(topic,"actuators/heat/1") == 0){
    if (messageTemp == "1") {
      digitalWrite(heatLed, HIGH);
      digitalWrite(heatRele, HIGH);
    }
    else{
      digitalWrite(heatLed, LOW);
      digitalWrite(heatRele, LOW);
    }
  }
  if (strcmp(topic,"actuators/lights/1") == 0){
    if (messageTemp == "1") {
      lightsState = true;
      lastLightState = true;
    }
    else if(messageTemp == "0"){
      lightsState = false;
      lastLightState = false;

    }
  }
    if (strcmp(topic,"actuators/party/1") == 0){
    if (messageTemp == "1") {
        digitalWrite(fiestaMode, HIGH);
         int melodyPreChorus[] = {C5, C5, C5, C5, C5, D5, C5, F5, E5, D5, C5, C5, B4, B4, B4, B4, B4, C5, B4, B4, C5, B4, A4};
  int noteDurationsPreChorus[] = {CORCHEA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, NEGRA, NEGRA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, NEGRA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, NEGRA, CORCHEA, CORCHEA, NEGRA, NEGRAPUNTO};
  int melodyChorus[] = {E5, E5, E5, E5, F5, E5, G5S, 0, A5, 0, B5, 0, E5, E5, E5, E5, F5, E5, 0, E5, A5, B5, 0, C6, 0, 0, E5, E5, E5, E5, F5, E5, G5S, A5, B5, D6, 0, D6, C6, B5, A5};
  int noteDurationsChorus[] = {CORCHEA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, NEGRA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, NEGRA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, NEGRA, CORCHEA, CORCHEA, NEGRA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, NEGRA, NEGRA, NEGRA, BLANCA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, CORCHEA, REDONDA};
  playMelody(melodyPreChorus, noteDurationsPreChorus, sizeof(melodyPreChorus) / sizeof(int));
  delay(300);
  playMelody(melodyChorus, noteDurationsChorus, sizeof(melodyChorus) / sizeof(int));
  delay(1000);
    }
    else{
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(fiestaMode, LOW);
    }
  }
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("actuators/fans/1");
      client.subscribe("actuators/heat/1");
      client.subscribe("actuators/lights/1");
      client.subscribe("actuators/party/1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
  int lightSensorValue = analogRead(sensorLlum);
  int lightOutputValue = 0;
  
  // Si las luces están activadas
  if (lightsState) {
    // Ajustar la intensidad de las luces según el valor del sensor de luz
    if (lightSensorValue > 2500) {
      lightOutputValue = 255;
    } else if (lightSensorValue < 250) {
      lightOutputValue = 0;
    } else {
      lightOutputValue = map(lightSensorValue, 3500, 0, 255, 0);
    }
  } else {
    // Si las luces están apagadas, asegúrate de que estén apagadas completamente
    lightOutputValue = 0;
  }
  
  // Controlar las luces
  for (int i = 0; i < 6; i++) {
    analogWrite(lights[i], lightOutputValue);
  }
  
  
  int soundSensorValue = analogRead(sensorSoroll);
  int ledsToLight = map(soundSensorValue, 0, 350, 0, 10); // sensor, minValue, maxValue, minLed, maxLed
    for (int i = 0; i < 10; i++) {
      if (i < ledsToLight) {
        digitalWrite(ledBar[i], HIGH);
      } else {
        digitalWrite(ledBar[i], LOW);
      }
    }
  readAnalogic();
  delay(2000);
}

void readAnalogic(){
  int lightSensor = analogRead(sensorLlum);
  int soundSensor = analogRead(sensorSoroll);
  char str2[16];
  sprintf(str2, "%d", lightSensor);
  client.publish("sensors/light_level/1", str2); // send data
  char str1[16];
  sprintf(str1, "%d", soundSensor);
  client.publish("sensors/noise_level/1", str1); // send data

}
void playMelody(int melody[], int durations[], int size) {
  for (int thisNote = 0; thisNote < size; thisNote++) {
    int noteDuration = durations[thisNote];
    int note = melody[thisNote];
    if (note != 0) {
      tone(BUZZER_PIN, note, noteDuration);
    }
    delay(noteDuration + 50);
    noTone(BUZZER_PIN);
  }
}
