#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL69dqQk6Go"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "T3MOsvb6GE38Kx4OiFt0jpeXJ2ZsGWwj"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include <MQUnifiedsensor.h>
#include <BlynkSimpleEsp32.h>
#include <Arduino.h>
#include <analogWrite.h>
#include <WiFiManager.h>

#define BOTtoken "5835205598:AAG1lK-5hHgUUjEWQ0YpkHjxA5GnLDlU50E"  // your Bot Token (Get from Botfather)
#define CHAT_ID "-1001358472912"

// Replace with your network credentials
// const char* ssid = "Pasek";
// const char* password = "11oktober";
char auth[]= "T3MOsvb6GE38Kx4OiFt0jpeXJ2ZsGWwj ";


#define mq135Pin 34
#define PIN_RED  14
#define PIN_GREEN  13 // GIOP22
#define PIN_BLUE    12

#define placa "Arduino UNO"
#define Voltage_Resolution 3.3
#define ADC_Bit_Resolution 12
#define RatioMQ135CleanAir 3.6
#define type "MQ-135" 
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, mq135Pin, type);

// Initialize Telegram BOT
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

BlynkTimer timer;

int botRequestDelay = 1000; // Checks for new messages every 1 second.
unsigned long lastTimeBotRan;
String targetChatID = "";

int state = 0;
int timerCounter = 0;
  
void setup() {
Serial.begin(115200);
Serial.println("Hi! Aku Red & Sensor ESP32 Sudah Aktif!");
pinMode(PIN_RED,   OUTPUT);
pinMode(PIN_GREEN, OUTPUT);
pinMode(PIN_BLUE,  OUTPUT);

Serial.println("Hi! HACHI Lampung Sudah Aktif Nih!");
timer.setInterval(1000L, myTimerEvent);

WiFiManager wm;
wm.resetSettings();
bool res;
res = wm.autoConnect("HACHI","password"); // password protected ap
if(!res) {
    Serial.println("Failed to connect");
  } 
else {
    //if you get here you have connected to the WiFi    
    Serial.println("connected!");
    //Blynk.config(BLYNK_AUTH_TOKEN, (char*)ip, port);
    Blynk.config((char*)auth);
}
// Connect to Wi-Fi
#ifdef ESP32
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
#endif
while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.println("Connecting to WiFi..");
}
// Print ESP32 Local IP Address
Serial.println(WiFi.localIP());
bot.sendMessage(CHAT_ID, "Hai HACHI Bali Sudah Aktif Nih! ", "");

MQ135.setRegressionMethod(1);
MQ135.init(); 
Serial.print("Calibrating please wait.");
float calcR0 = 0;
for(int i = 1; i<=10; i ++)
{
  MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
  calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
  Serial.print(".");
}
MQ135.setR0(calcR0/10);
Serial.println("  done!.");

if(isinf(calcR0)) {Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
if(calcR0 == 0){Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);}
/*****************************  MQ CAlibration ********************************************/ 
Serial.println("** Values from MQ-135 ****");
Serial.println(  " NH4  ");  
}

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
Serial.println("handleNewMessages");
Serial.println(String(numNewMessages));

for (int i=0; i<numNewMessages; i++) {
  // Chat id of the requester
  String chat_id = String(bot.messages[i].chat_id);
  Serial.println(chat_id);
  if (chat_id != CHAT_ID){
    bot.sendMessage(chat_id, "Unauthorized user", "");
    continue;
  }
  targetChatID = chat_id;
  // Print the received message
  String text = bot.messages[i].text;
  Serial.println(text);
  }
}

void myTimerEvent()
{
  if (timerCounter < 601 && state == 3){
    timerCounter++;
  }
  else{
    timerCounter = 0;
    if(state == 3) {
      state = 0;
    }
  }
  MQ135.update();
  MQ135.setA(102.2 ); MQ135.setB(-2.473); // Configure the equation to calculate NH4 concentration value
  float NH4 = MQ135.readSensor();
  Serial.println(""); 
  Serial.print(" Nilai Amonia : "); 
  Serial.print(NH4); 
  Serial.print(" ppm "); 
  Blynk.virtualWrite(V4, NH4);
     
  if (NH4 <= 15 && state != 1){
    state = 1;
    setColor(0,255,0);
    Serial.println("LED HIJAU NYALA");
    Blynk.virtualWrite(V0,0);
    Blynk.virtualWrite(V1,0);
    Blynk.virtualWrite(V2,1);
    String message = String(NH4);
    bot.sendMessage(CHAT_ID, "Kandang Bali sudah Aman. \nNilai Amonia Saat ini: " + message + "ppm", "" );
  }

  else if (NH4 > 15 && NH4 < 20 && state != 2){
    state = 2;
    setColor(255,100,0);
    Serial.println("LED KUNING NYALA");
    Blynk.virtualWrite(V0,0); //merah
    Blynk.virtualWrite(V1,1); // kuning
    Blynk.virtualWrite(V2,0); // Hijau
    String message = String(NH4);
    bot.sendMessage(CHAT_ID, "Kandang Bali WASPADA! \nNilai Amonia Saat ini: " + message + "ppm, \nSegera bersihkan kandang ayam!", "" );
  }
  else if (NH4 >= 20 && state !=3){
    state = 3;
    setColor(255,0,0); //MERAH
    Serial.println("LED RED NYALA");
    Blynk.virtualWrite(V0,1);
    Blynk.virtualWrite(V1,0);
    Blynk.virtualWrite(V2,0);
    String message = String(NH4);
    bot.sendMessage(CHAT_ID, "Kandang Bali BAHAYA!!! \nNilai Amonia Saat ini: " + message + "ppm, \nSegera bersihkan kandang ayam!!!", "" );
    
  }
}

unsigned long previousTime = 0; // Previous time
unsigned long interval = 1000;

void loop() {
  Blynk.run();
  timer.run();
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

void setColor(int merah, int ijo, int biru){
  analogWrite(PIN_RED , merah);
  analogWrite(PIN_GREEN , ijo);
  analogWrite(PIN_BLUE , biru);
}