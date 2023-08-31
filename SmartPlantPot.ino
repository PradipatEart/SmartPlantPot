//Pin Define for connection
//D2 = Button WiFi Config
//D5 = Water Pump Relay Pin
//D6 = Trig Pin Uts
//D7 = Echo Pin Uts
//A0 = Soil Moisture Pin

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <FirebaseESP8266.h>

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
double waterDistance;
double waterDivide;
double waterPercent;

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
  if(fbReadData("SmartPlantPot/wateringStatus").equals("1")){
    watering();
  }
  String minMoisture_fir = fbReadData("SmartPlantPot/minMoisture");
  String str_minMoisture_fir = getStringPartByNr(minMoisture_fir, '"', 1);
  int minMoisture = str_minMoisture_fir.toInt();
  if(fbReadData("SmartPlantPot/moisture").toInt() <= minMoisture && (fbReadData("SmartPlantPot/moisture").toInt()) > 20) {
    fbSendStringData("SmartPlantPot/wateringStatus", "1");
    delay(0);
  }
  measureMoist();
  delay(500);
  measureWater();
  delay(500);
}

  String getStringPartByNr(String data, char separator, int index){
    // spliting a string and return the part nr index
    // split by separator

    int stringData = 0;        //variable to count data part nr 
    String dataPart = "";      //variable to hole the return text

    for(int i = 0; i<data.length()-1; i++) {    //Walk through the text one letter at a time

      if(data[i]==separator) {
        //Count the number of times separator character appears in the text
        stringData++;

      }else if(stringData==index) {
        //get the text when separator is the rignt one
        dataPart.concat(data[i]);

      }else if(stringData>index) {
        //return text and stop if the next separator appears - to save CPU-time
        return dataPart;
        break;

      }

    }
    //return text if this is the last part
    return dataPart;
  }

void watering(){
  digitalWrite(pumpPin, LOW);
  fbSendStringData("SmartPlantPot/wateringStatus", "0");
  String waterAmount_fir = fbReadData("SmartPlantPot/waterAmount");
  String str_waterAmount_fir = getStringPartByNr(waterAmount_fir, '"', 1);
  double waterAmount = str_waterAmount_fir.toInt();

  double x = 23;
  delay((waterAmount / x) * 1000.00);
  digitalWrite(pumpPin, HIGH);
  delay(1000);
}

void measureMoist(){
  moist = analogRead(soilPin);
  if(moist <= 15){
    fbSendIntData("SmartPlantPot/moisture", 0);
  }else {
    moist = map(moist, 0, 1023 , 10, 550);
    moist = map(moist, 550, 10, 0, 100);
    fbSendIntData("SmartPlantPot/moisture", moist);
  }
}

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

  waterDistance = 16 - distance;
  waterDivide = waterDistance / 16;
  waterPercent = waterDivide * 100;
  if(waterPercent < 10.00){
fbSendIntData("SmartPlantPot/waterLevel", 0);
  }else {
    fbSendIntData("SmartPlantPot/waterLevel", waterPercent);
  }
}

bool fbSendStringData(String data, String send) {
  bool fb = Firebase.setString(firebaseData, data, send);
  return fb;
}
bool fbSendIntData(String data, int send) {
  bool fb = Firebase.setString(firebaseData, data, send);
  return fb;
}
String fbReadData(String data){
  if (Firebase.getString(firebaseData, data)) {   
        String read_data = (firebaseData.stringData());
      return read_data;
  } else {
    Serial.println(firebaseData.errorReason());
    return "0";
  }
}