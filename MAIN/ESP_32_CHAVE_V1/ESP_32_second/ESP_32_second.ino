#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

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

int heatRele = 33; // Rele simulant electrovalvula
int coldMotor = 13;
int fiestaMode = 12;
int pito = 14;
int coldLed = 22;
int heatLed = 23;
int sensorSoroll = 36;
int sensorLlum = 39;


bool lightsState = false;

int lights[6] = {1,3,32,25,26,27};

bool isConnected = false; // Variable para indicar si el ESP está conectado a Internet


void setup() {

  Serial.begin(115200);

  // Initialize pins for lights
  for (int i = 0; i < 6; i++) {
    pinMode(lights[i], OUTPUT);
    digitalWrite(lights[i], LOW);
  }

  // Initialize pins for heatRele and coldMotor
  pinMode(heatRele, OUTPUT);
  pinMode(coldMotor, OUTPUT);
  pinMode(fiestaMode, OUTPUT);
  pinMode(coldLed, OUTPUT);
  pinMode(heatLed, OUTPUT);
  pinMode(pito, OUTPUT);
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
    isConnected = true; // El ESP está conectado a Internet

}
bool fan = false;

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
    }
    else{
      lightsState = false;
    }
  }
    if (strcmp(topic,"actuators/party/1") == 0){
    if (messageTemp == "1") {
        digitalWrite(fiestaMode, HIGH);
        digitalWrite(pito, HIGH);
    }
    else{
        digitalWrite(pito, LOW);
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

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(heatRele, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(heatRele, LOW);
    }
  }
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
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
  if(lightsState){
    for(int i = 0; i < 6; i++){
    digitalWrite(lights[i], HIGH);
  }

  }else{
    for(int i = 0; i < 6; i++){
    digitalWrite(lights[i], LOW);
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
  client.publish("sensors/light_level/1", str2);
  char str1[16];
  sprintf(str1, "%d", soundSensor);
  client.publish("sensors/noise_level/1", str1);

}
