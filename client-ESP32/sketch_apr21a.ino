#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <WiFi.h>
#include <PubSubClient.h>

#define LED_PIN LED_BUILTIN
#define BTN_PIN 23
#define PHOTO_SENSOR_PIN 36

// SSID/Password
char* ssid = (char*)"View2";
const char* password = "";

// MQTT Broker IP address:
const char* mqtt_server = "192.168.43.31";

// OLED display width & height, in pixels
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define RESET_OLED 16
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, RESET_OLED);

WiFiClient espClient;
PubSubClient client(espClient);

//stats on/off
int btn_stats_on = 0;
int light_sensor_stats_on = 0;

struct button_s
{
  int state;
  int timer;
  unsigned long period;
};

struct lightsensor_s
{
  int value;
  int timer;
  unsigned long period;
};

struct button_s btn_s;
struct lightsensor_s ls_s;

void setup() {
  Serial.begin(115200);
  
  setup_wifi();
  setup_periphericals();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_periphericals()
{
  //LEDS
  pinMode(LED_PIN, OUTPUT);
  Serial.println("Led ok");

  //OLED
  Wire.begin(4,15);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.display();
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  
  display.println("Init...");
  display.display();
  display.println("Ok");
  display.display();
  Serial.println("OLED ok");

  //BUTTON
  pinMode(BTN_PIN, INPUT_PULLUP);
  btn_stats_on = 0;
  btn_s.state = 0;
  btn_s.timer = 0;
  btn_s.period = 3000000;
  
  //LIGHT SENSOR
  light_sensor_stats_on = 0;
  ls_s.value = 0;
  ls_s.timer = 1;
  ls_s.period = 3000000;
}
void setup_wifi() {
  delay(10);
  
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
  Serial.println("SSID: ");
  Serial.println(WiFi.SSID());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: .");
  Serial.print(topic);
  Serial.print(". Message: ");
  char* messageTemp = (char*)malloc(sizeof(char)*length);

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp[i] = (char)message[i];
  }
  Serial.print("   size: ");
  Serial.print(length);
  Serial.println();

  if(strcmp(topic, "ESP/led") == 0)
  {
    if(strncmp(messageTemp, "ON", 2) == 0)
    {
      Serial.println("LED ON");
      digitalWrite(LED_PIN, HIGH);
    }
    else if(strncmp(messageTemp, "OFF", 3) == 0)
    {
      Serial.println("LED OFF");
      digitalWrite(LED_PIN, LOW);
    }
    else
    {
      Serial.println("command not found");
    }
  }
  else if(strcmp(topic, "ESP/button") == 0)
  {
    if(strncmp(messageTemp, "ON", 2) == 0)
    {
      Serial.println("Button stats ON");
      btn_stats_on = 1;
    }
    else if(strncmp(messageTemp, "OFF", 3) == 0)
    {
      Serial.println("Button stats OFF");
      btn_stats_on = 0;
    }
    else
    {
      Serial.println("command not found");
    }
  }
  else if(strcmp(topic, "ESP/light") == 0)
  {
    if(strncmp(messageTemp, "ON", 2) == 0)
    {
      Serial.println("Photo sensor stats ON");
      light_sensor_stats_on = 1;
    }
    else if(strncmp(messageTemp, "OFF", 3) == 0)
    {
      Serial.println("Photo sensor OFF");
      light_sensor_stats_on = 0;
    }
    else
    {
      Serial.println("command not found");
    }
  }
  else if(strcmp(topic, "ESP/lcd") == 0)
  {
    display.clearDisplay();
    display.display();
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    
    display.println(messageTemp);
    display.display();
  }
  else
  {
    Serial.println("No treatement for this topic");
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
      client.subscribe("ESP/led");
      client.subscribe("ESP/button");
      client.subscribe("ESP/light");
      client.subscribe("ESP/lcd");
      
      Serial.println("subscribed to:");
      Serial.println("ESP/led");
      Serial.println("ESP/button");
      Serial.println("ESP/light");
      Serial.println("ESP/lcd");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

#define MAX_WAIT_FOR_TIMER 2
unsigned int waitFor(int timer, unsigned long period){
  static unsigned long waitForTimer[MAX_WAIT_FOR_TIMER];  // il y a autant de timers que de tâches périodiques
  unsigned long newTime = micros() / period;              // numéro de la période modulo 2^32 
  int delta = newTime - waitForTimer[timer];              // delta entre la période courante et celle enregistrée
  if ( delta < 0 ) delta = 1 + newTime;                   // en cas de dépassement du nombre de périodes possibles sur 2^32 
  if ( delta ) waitForTimer[timer] = newTime;             // enregistrement du nouveau numéro de période
  return delta;
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }  

  if(light_sensor_stats_on && waitFor(btn_s.timer, btn_s.period))
  {
    ls_s.value = map(analogRead(PHOTO_SENSOR_PIN), 0, 3000, 100, 0);
    Serial.print("ls: ");
    Serial.println(ls_s.value);
    char str[16];
    itoa(ls_s.value, str, 10);
    client.publish("SERVER/light", str);
  }
  if(btn_stats_on && waitFor(ls_s.timer, ls_s.period))
  {
    btn_s.state = 1-digitalRead(BTN_PIN);
    Serial.print("btn: ");
    Serial.println(btn_s.state);
    char str[16];
    itoa(btn_s.state, str, 10);
    client.publish("SERVER/button", str);
  }
  client.loop();
}
