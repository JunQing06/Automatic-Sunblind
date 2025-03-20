#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <DHT.h>
#include <HTTPClient.h>

Servo myServo;
DHT dht(23, DHT11);
int servoPin = 19;
int servo_status = 0;
unsigned long long lastTimeBotRan = 0;
unsigned long long lastTimeTRead = 0;
unsigned long long lastTimeHRead = 0;
float temp = 0.0;
float humidity = 0.0;
const char *ssid = "Ok";
const char *password = "Ooi";
#define CHAT_ID "1513532791"
#define BOTtoken "7898979256:AAHPmY23eai--9UmGMVuK3ISuophuKLjQ_M"
String apiKey = "GKHZZ1ARSR7N2TDE";
String url = "https://api.thingspeak.com/update";
String message;
uint8_t httpCode;
HTTPClient http;
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

void handleNewMessages(int numNewMessages){
  for(int i = 0; i < numNewMessages; i++){
    String chat_id = String(bot.messages[i].chat_id);
    if(chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorised user","");
      continue;
    }
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    if(text == "/start"){
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following command: \n";
      welcome += "/open_sunblind\n/close_sunblind\n/read_temperature\n/read_humidity\n";
      bot.sendMessage(chat_id, welcome, "");
    }
    if(text == "/open_sunblind"){
      myServo.write(180);
      servo_status = 1;
      bot.sendMessage(chat_id, "Sunblind is opened","");
    }
    if(text == "/close_sunblind"){
      myServo.write(0);
      servo_status = 0;
      bot.sendMessage(chat_id, "Sunblind is closed","");
      }
    if(text == "/read_temperature"){
      if (isnan(temp)) {
        bot.sendMessage(chat_id, "Failed to read temperature");
        } else {
          bot.sendMessage(chat_id, "Temperature: " + String(temp) + " °C");
          }
      }
    if(text == "/read_humidity"){
      if (isnan(humidity)) {
        bot.sendMessage(chat_id, "Failed to read humidity");
        } else {
          bot.sendMessage(chat_id, "Humidity: " + String(humidity) + " %");
        }
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  myServo.attach(servoPin);
  myServo.write(0);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if(millis() - lastTimeBotRan > 250){
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    if(numNewMessages > 0){
      handleNewMessages(numNewMessages);
    }
    lastTimeBotRan = millis();
  }
    if(millis() - lastTimeTRead > 2500){
     temp = dht.readTemperature();
     humidity = dht.readHumidity();
      Serial.println("Temperature: " + String(temp) + " °C");
      Serial.println("Humidity: " + String(humidity) + " %");
      lastTimeTRead = millis();
    }
  message = url + "?api_key=" + apiKey + "&field1=" + String(temp) + "&field2=" + String(humidity) + "&field3=" + String(servo_status);
  http.begin(message);
  Serial.println(message);
  httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    message = http.getString();
    Serial.println(message);
  }
}