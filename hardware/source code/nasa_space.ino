

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
int pin = 2;
const char* ssid = "j";
const char* password = "123456789";
const char* mqtt_server = "server.archeindustries.com";
String data = "";
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int threshold = 500;
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {
  data = "";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    data = data + payload[i];
   
  }
  String topic_c = "cuton";
  String off_topic = "cutoff";
  String recTopic(topic);
  Serial.println("data : "+ data);
//  Serial.println("topic_C value: "+topic_c);
  if(recTopic==topic_c){
    Serial.println("in cutoff");
    digitalWrite(pin,HIGH);
    delay(100);
    }
    else if(recTopic == off_topic){
      Serial.println("not in cutoff");
      digitalWrite(pin,LOW);
      delay(100);
      }
//    if(topic=="cutoff"){
//      Serial.println("in the cutoff topic");
//      if(data == "on"){
//         digitalWrite(pin, HIGH);
//         delay(100);
//         Serial.println("Connection established");
//        }}
//        else if(topic== ""){
//           digitalWrite(pin,LOW);
//           delay(100);
//           Serial.println("Disconnected the circuit");
//          }
//      }
 
  Serial.println();



}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("cutoff");
      client.subscribe("cuton");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(pin, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("ok", msg);
  }
}
