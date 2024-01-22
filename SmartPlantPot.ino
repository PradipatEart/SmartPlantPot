//Pin Define for connection
//D2 = Button WiFi Config
//D5 = Water Pump Relay Pin
//A0 = Soil Moisture Pin

//Set Watering Duration
int waterDuration = 5 * 1000;

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <FirebaseESP8266.h>
#include <NTPClient.h>

//Define for FirebaseDatabase
#define FIREBASE_HOST "smartplantpot-831ff-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "AIzaSyDn9qEPDlW5JBkBKboX-83m1-aYIq3uDwM"
FirebaseData firebaseData;

//Define Ultrasonic
#define trigPin D6
#define echoPin D7
#define D6 12
#define D7 13
int duration;
long distance;
long waterlevel;
double waterpercent;

//Define for WiFiManager
#define ledPin D4
#define D4 2
#define ConfigWiFi_Pin D2
#define D2 4
#define ESP_AP_NAME "SmartPlantPot_AP"

//Define for Watering
#define pumpPin D5
#define D5 14

//Define for Soil Moisture
#define soilPin A0
int moist;

//NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffsetInSeconds = 25200;
const int daylightOffsetInSeconds = 3600;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting...");

  //Wifi Section
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  WiFiManager wm;
  wm.autoConnect(ESP_AP_NAME);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.println(".");
  }
  if (digitalRead(ConfigWiFi_Pin) == LOW) {
    wm.resetSettings();
  }
  Serial.println("WiFi connected");
  Serial.print("IP Adress : ");
  Serial.println(WiFi.localIP());
  digitalWrite(ledPin, LOW);

  //Watering Section
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, HIGH);

  //Firebase Section
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Moisture
  pinMode(soilPin, INPUT);

  //Ultrasonic
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

}

void loop() {
  //Check for application watering status
  if(fbReadData("SmartPlantPot/wateringStatus").equals("1")){
    watering();
  }
  //Check for soil moisture read
  if(fbReadData("SmartPlantPot/moisture").toInt() <= 40{
    watering();
    delay(60000); //Delay 1 min
  }
  measureMoist();
  measureWater();
  delay(500);
}

//For doing watering method
void watering(){
  digitalWrite(pumpPin, LOW);
  fbSendStringData("SmartPlantPot/wateringStatus", "0");
  delay(waterDuration);
  digitalWrite(pumpPin, HIGH);
}

//Measuring Moist Method
void measureMoist(){
  moist = analogRead(soilPin);
  if(moist <= 15){
    fbSendIntData("SmartPlantPot/moisture", 0);
  }else {
    moist = map(moist, 0, 1023 , 10, 550);
    moist = map(moist, 550, 10, 0, 100);
    fbSendIntData("SmartPlantPot/moisture", moist);
  }
  delay(1000);
}

//Measuring WaterLevel Method
void measureWater(){
  digitalWrite(trigPin, LOW);
  
  delayMicroseconds(5);
  
 //Trigger sensor
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  //Read Sensor
  duration = pulseIn(echoPin, HIGH);
  
  //Calculate distance
  distance = duration*0.034/2;

  waterpercent = ((16 - waterlevel) / 16) * 100;

  fbSendIntData("SmartPlantPot/waterLevel", waterpercent);
  delay(500);
}

//Sending data(String) to Firebase
bool fbSendStringData(String data, String send) {
  bool fb = Firebase.setString(firebaseData, data, send);
  return fb;
}
//Sending data(Integer) to Firebase
bool fbSendIntData(String data, int send) {
  bool fb = Firebase.setString(firebaseData, data, send);
  return fb;
}
//Read data form Firebase
String fbReadData(String data){
  if (Firebase.getString(firebaseData, data)) {   
        String read_data = (firebaseData.stringData());
      return read_data;
  } else {
    Serial.println(firebaseData.errorReason());
    return "0";
  }
}
