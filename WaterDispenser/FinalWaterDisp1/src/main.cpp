#include <AsyncMqttClient.h>
#include <time.h>
#include <NTPClient.h>
#include <ArduinoJson.h>
#include <HX711.h>
#include <ESP8266WiFi.h>         
#include <WiFiUdp.h>
#include <PubSubClient.h>

WiFiClient espClient;

#define DEVICE_TYPE "Food"

//-----------------------------------------------------------------------------------------
// Pin Numbers
// https://www.electronicwings.com/nodemcu/nodemcu-gpio-with-arduino-ide
//-----------------------------------------------------------------------------------------
// Define pins for sensor(s)  

#define DATA_PIN   12  
#define CLOCK_PIN  13 
#define MOTION_PIN  5
//  Solenoid pin
#define Solenoid_Pin 4    

//----------------------------------------------------------------------
int calibration_factor = 455.77; 
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//---- Enter your sensitive data 
//----------------------------------------------------------------------
char ssid[] = "AndroidAP65EB";
char pass[] = "saitamaa";
//----------------------------------------------------------------------
const char* mqtt_Server = "raspberrypi.local"; //"192.168.1.253";
const int   mqtt_Port = 1883;     //1883;

//TOPIC
const char OutFromDevice[]  = "smartPETS/devices/Food/Out";

                      

boolean WiFi_OK = false;
boolean HUB_OK = false;

float Weight, PrevWeight, WeightNew, WeightCheck;
float consumption = 0;
float consumptionTotal = 0;
float tempCons=0;


//Only track consumption when,
float MaxConsumption   = 5; //10 gms

//declare motion variables
int motion = 0;
int PetAction      = 0;
int PetActionCount = 0;

String UOM  = "gm";   

 
//declare refill variables
int fillCount = 0;
float refill =   10;      //10 ms
float stopFill = 30;     //Max Weight Limit of the Bowl (in Grams or Kg)

//declare variables to keep track of time
int counter = 0;
unsigned long dayStart = 0;


//------------------------------------------------------------------------------------
// Get time from time server
// https://werner.rothschopf.net/202011_arduino_esp8266_ntp_en.htm

/* Configuration of NTP */
#define MY_NTP_SERVER "at.pool.ntp.org"           
//#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03"   
#define MY_TZ "IST-5:30" //Put your time zone

/* Globals */
time_t now;                         // this is the epoch
tm tm;   
int DateNBR, TimeNBR, HourNBR, PrevMinuteNBR, MinuteNBR, SecondNBR;

String DateSTR, DateSTR8, TimeSTR6, TimeSTR;
String tempText;



// --- Reduced to xhrs from 24hr for Testing -----------------------------------

float day_ms = 0.10 * 60 * 60 * 1000;   //Reduced to 10mins from 24hr for Testing
//float day_ms = 1 * 60 * 60 * 1000;   //Reduced to 1hr from 24hr for Testing
//float day_ms = 24 * 60 * 60 * 1000;


//----------------------------------------------------------------------------

void MQTTcallback(char* topic, byte* payload, unsigned int length) {

   DynamicJsonDocument doc(1024);
 
   Serial.print("Message arrived in topic: ");
   Serial.println(topic);
 
   //Serial.print(" Message: ");
   String message;
   for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];  //Conver *byte to String
   }
   
   //Serial.println(message);

   String input = message;
   deserializeJson(doc, input);
   JsonObject obj = doc.as<JsonObject>();
  
   String output;
   serializeJson(doc, output);

  
} //void onMqttMessage(int messageSize) {


PubSubClient Client(mqtt_Server, mqtt_Port, MQTTcallback, espClient);

void Init_Global_Variables() {

  WiFi_OK = false;
  HUB_OK = false;
  
  PetAction = 0;
  Weight = 0;
  PrevWeight = 0;
  
  fillCount = 0;
  PetActionCount = 0;
  consumption = 0;
  consumptionTotal = 0;


} //void Init_Global_Variables() 

void GetDateTime() {
 
// ---------------------------------------------------------------
// Refer to below link for Data Time
//https://werner.rothschopf.net/202011_arduino_esp8266_ntp_en.htm

  time(&now);                       // read the current time
  localtime_r(&now, &tm);           // update the structure tm with the current time

  char dt[16];
  char tm2[16];
  
  sprintf(dt, "%02d-%02d-%02d", (tm.tm_year + 1900),(tm.tm_mon + 1),tm.tm_mday);
  sprintf(tm2, "%02d:%02d:%02d", tm.tm_hour,tm.tm_min,tm.tm_sec);

  //Convert in to Date NBR
  char dtNBR[8];
  char tm2NBR[6];
  
  sprintf(dtNBR, "%02d%02d%02d", (tm.tm_year + 1900),(tm.tm_mon + 1),tm.tm_mday);
  sprintf(tm2NBR, "%02d%02d%02d", tm.tm_hour,tm.tm_min,tm.tm_sec);

  //Arduino String Functions
  //https://pijaeducation.com/arduino-string-function/
  
  String tempDateSTR = dtNBR;
  DateSTR8 = tempDateSTR.substring(0, 8);
  TimeSTR6 = tm2NBR;
  
  //Serial.print(" *DBG* dtNBR: ");
  //Serial.print(dtNBR);
  //Serial.print(" DateSTR: ");
  //Serial.print(DateSTR);  
  //Serial.print(" tm2NBR: ");
  //Serial.println(tm2NBR);
  
  DateNBR = int(dtNBR);
  TimeNBR = int(tm2NBR);
  
  HourNBR = tm.tm_hour;
  MinuteNBR = tm.tm_min;
  SecondNBR = tm.tm_sec;

  //Serial.print(" HH:MM:SS: ");
  //Serial.print(HourNBR);
  //Serial.print(":");
  //Serial.print(MinuteNBR);
  //Serial.print(":");
  //Serial.println(SecondNBR);
    
  DateSTR = dt;
  TimeSTR = tm2;
 
} //GetDateTime()


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  int MinuteCounter = 0;
  
  while ( WiFi.status() != WL_CONNECTED && MinuteCounter < 300) {
    
    delay(1000);
    Serial.print("Connecting to WiFi...MinuteCounter: "); // Not Connected.");
    Serial.println(MinuteCounter);
    
    MinuteCounter++;
  
  } //while ( WiFi.status() != WL_CONNECTED && MinuteCounter < 300) {

  if ( WiFi.status() == WL_CONNECTED ) {
    
    GetDateTime();
    
    Serial.println("");
    Serial.println("Connected to WiFi...");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    WiFi_OK = true;
    tempText = "WiFi Ok!"; //+ WiFi.localIP().toString(); //String(broadCast);
  } //if (WiFi.status() == WL_CONNECTED ) {

  
} //End of void setup_wifi()

void reconnect() {
  
  String tempText = "";
  // Loop until we're reconnected

  int MinuteCounter = 0;
  
  while (!Client.connected() && MinuteCounter < 300) {
    yield();
    
    Serial.print("Attempting MQTT connection...MinuteCounter: ");
    Serial.println(MinuteCounter);
    
    // set the message receive callback
    //Client.setCallback(onMqttMessage);
    
    // Attempt to connect
    if (Client.connect("ESP8266Client")) {
      
      if (Client.connected()) {

          //Client.setBufferSize(MQTT_PACKET_SIZE);
          
          Serial.print("connected! MQTT Buffer Size: ");

          Client.setCallback(MQTTcallback);
  
          GetDateTime();
          
          HUB_OK = true;
          
          // >>> Performed in Setup routine
          
      } //if (Client.connected()) {
      
    } 
    else {
      tempText = " HUB failed, rc: " + String(Client.state()); 
      Serial.print("failed, rc=");
      Serial.print(Client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }

      
  } //while (!Client.connected()) {
 
} //End of void reconnect() {



HX711 scale;  // Initializes library functions.

//---------------------------------------------------------------------------------------------------------------------

void Setup_Scale() {
    
    scale.begin(DATA_PIN, CLOCK_PIN);
    scale.set_scale();
    
    //scale.tare();
    
    scale.set_scale(calibration_factor);  // Adjusts the calibration factor. Required during scale startup
    
    scale.tare();                         //Reset the scale to 0
        
} //void Setup_Scale() {

void setup() {

  Init_Global_Variables();
  
  //Initialize pins
  
    pinMode(Solenoid_Pin, OUTPUT);
    
    pinMode(MOTION_PIN, INPUT);

    
  // Start Serial Communication 
  delay(10000);
  Serial.begin(115200);
  

  Serial.println("\nNTP TZ DST - bare minimum");
  configTime(MY_TZ, MY_NTP_SERVER); 
  
  Setup_Scale();
  
  setup_wifi();
  
  
  Client.setServer(mqtt_Server, mqtt_Port);
  Client.setCallback(MQTTcallback);  

  // Serial.print('FreeHeap: ');
  // Serial.println(ESP.getFreeHeap());
  
} //End of void setup() {

//--------------------------------------------------------------------------------------------------------------
float GetWeight() {   

  Weight = scale.get_units(12);   //Takes the average of 12 readings from the ADC
  if(Weight<=0)
  {
    
    Weight=0;
  }

  Serial.print(String(Weight,3));
  Serial.print(" grams ");
       
  return Weight;
  
} // End of GetWeight


void TurnON_WaterPump() {
  
    digitalWrite(Solenoid_Pin, LOW);
    delay(1000);
    
} //End of void TurnON_WaterPump()

void TurnOFF_WaterPump() {

  digitalWrite(Solenoid_Pin,HIGH);
  delay(1000);

} //End of void TurnON_WaterPump()


void Refill() {

  //Check if Food level is below the refill threshold and fill up
  if (Weight < refill ) {

    Serial.print("Filling...");

    TurnON_WaterPump();

    WeightCheck = Weight;
    int iterations = 0;
    
    while ( WeightCheck <= stopFill && iterations < 10 ) { //20 ) {  //15) {
      
      yield();
      //GetDateTime();

      Serial.print("FillCount:  ");
      Serial.println(fillCount);
      
      iterations++;
      WeightCheck = GetWeight();  

      delay(5000);  //delay(3000); //delay(1000); //delay(100);
 
    } 

    TurnOFF_WaterPump();

    //CHECK Weight after filling
    Weight = GetWeight(); 
    
    GetDateTime();
  }
    //-----------------------------------------
    //Send Message if it was really filled.
    //-----------------------------------------
    if ( Weight > refill ) { //refill =   0.01;      
      
      ++fillCount;       
      GetDateTime();

    //   SendMessage_to_HUB("03", "Refilled"); 
    } //if ( Weight > 0 ) {
    //----------------------------------------- 
    
 
  
} //void Refill() {
int MonitorMotion(int MaxMotionCheckMinute, int MaxDrinkCheckSec) {
  
  
  counter = 0;

  
  //--------------------------------------------------------------
  //Update Previous Weight due to Possible Refill!
  //if ( Weight > 0 && Weight > PrevWeight ) PrevWeight = Weight;
  //--------------------------------------------------------------
  
  //monitor motion for x minutes or until detected
  //-----------------------------------------------------------------
  while (counter < MaxMotionCheckMinute ) {  //20) {  //300 = 5 min
    
    yield();
    
    GetDateTime();
    // Moved to end of while loop delay(1000);
  
    
    Serial.print("Checking motion, Second(s) passed: ");   
    Serial.print(counter);
    Serial.print(" of ") ;   
    Serial.print(MaxMotionCheckMinute);
    Serial.print(" PrevWeight: ");  
    Serial.print(PrevWeight);
    Serial.print(" grams Weight: ");  
    Serial.print(Weight);  
    Serial.println(" gm ");  
    //Serial.println(" Seconds");

    
    motion = digitalRead(MOTION_PIN);
    
    if (motion == HIGH) {
    
      PetAction = 1;                  //detect if dog is taking food
      counter = MaxMotionCheckMinute; //300;     //end loop
      
    }
    
    Weight = GetWeight();
    
    delay(1000);

  } //while (counter < MaxMotionCheckMinute ) {

  //--------------------------------------------------------------------------------  
  // Wait for motion to be stopped for 1 minute and then calculate consumption
  //--------------------------------------------------------------------------------
  counter = 0;
  
    
  while (PetAction == 1 && counter < MaxDrinkCheckSec ) {     //60) {

    yield();
    
    GetDateTime();
    
    delay(1000);
  
    Serial.print("Motion detected! Motion PIN: "); 
    Serial.print(motion);  
    Serial.print(" Allow to Eat; Second(s) passed: ");   
    Serial.print(counter);
    Serial.print(" of ");
    Serial.print(MaxDrinkCheckSec);
    
    Serial.print(" PrevWeight: ");  
    Serial.print(PrevWeight);   
    Serial.print(" grams Weight: ");  
    Serial.print(Weight);        
    Serial.print(" gm PetAction: ");
    Serial.println(PetAction);
    
    
    motion = digitalRead(MOTION_PIN);
    if (motion == HIGH) {
      

      tempText = "Still Eating...";
      
      Serial.print(tempText);
      Serial.print(counter);
      Serial.println(" of 10");     
      
    }    
    //Weight = GetWeight();
      counter++;
  } //while (PetAction == 1 && counter < MaxDrinkCheckSec )
  
  
  return PetAction;

} //void Monitor_Motion(MaxMinute)


void SendMessage_to_HUB(String MessageType, String CommentSTR) {

     char DoubleQuote = '"';
     
     
    //Set payload
    String msgPayload = String("{\"Date\":\""          + DateSTR      +     DoubleQuote +
                             ",\"Time\":"             +   DoubleQuote + TimeSTR      +     DoubleQuote +
                             ",\"MsgType\":"          +   DoubleQuote + MessageType   +     DoubleQuote +                              
                             ",\"DeviceType\":"       +   DoubleQuote + DEVICE_TYPE   +     DoubleQuote + 
                             ",\"Weight\":"           +   DoubleQuote + String(Weight,3)        + DoubleQuote +
                             ",\"PrevWeight\":"       +   DoubleQuote + String(PrevWeight,3)    + DoubleQuote +
                            //  ",\"UOM\":"              +   DoubleQuote + UOM                     + DoubleQuote +
                             ",\"Consumption\":"      +   DoubleQuote + String(consumption,3)   + DoubleQuote +
                             ",\"TempConsumption\":"      +   DoubleQuote + String(tempCons,3)   + DoubleQuote +
                             ",\"Total\":"            +   DoubleQuote + String(consumptionTotal,3)  + DoubleQuote +
                             ",\"PetActionCount\":"   +   DoubleQuote + String(PetActionCount)      + DoubleQuote +
                            //  ",\"fillCount\":"        +   DoubleQuote + String(fillCount)           + DoubleQuote + 
                             ",\"Comment\":"          +   DoubleQuote + CommentSTR                  +  DoubleQuote +                     
                             "}");                                 

    // send message //datestr8 timestr6 
    if (!Client.connected()) reconnect();
    
    if (HUB_OK) { 
      
        Client.publish(OutFromDevice,msgPayload.c_str());

        Serial.println(" ****** published msgPayload ****** ");
        Serial.println(msgPayload);
        
       
        
    
    } //if (HUB_OK) {
    
} //void SendMessage_to_HUB() {

void loop(){


  if (!Client.connected()) 
  {
    yield();
    reconnect();
  }
  Client.loop();  

  yield();
  delay(1000);
    
  GetDateTime();
  
  Serial.print("Date: ");
  Serial.print(DateSTR); 
  
  Serial.print(" Time: ");
  Serial.println(TimeSTR); 

  //Serial.print(" NBR HH:MM:SS: ");
  //Serial.print(HourNBR);
  //Serial.print(":");
  //Serial.print(MinuteNBR);
  //Serial.print(":");
  //Serial.println(SecondNBR);  
  
  //Serial.print(" PREV Minute ");
  //Serial.println(PrevMinuteNBR);


  
  //Check if it is the same day and reset variables if a new day and correct for when millis() resets at 49.7 days
  //---------------------------------------------------------------------------------------------------------------
  if (((millis() - dayStart) >= day_ms) && (millis() >= (0.3 * day_ms))) {
          
    //Send daily totals 

    SendMessage_to_HUB("05", "Summary");

    //Reset daily tracking values

    consumption = 0;    
    consumptionTotal = 0;
    PetActionCount = 0;
    fillCount = 0;
    tempCons=0;
    
    dayStart = millis();
   
    
  } // if (((millis() - dayStart) >= day_ms) && (millis() >= (0.3 * day_ms))) {

  
    

  //Get Weight - Repeat every 1 minute with no motion//
  int iterations = 45;
  WeightNew = GetWeight(); 

  consumption = 0;
  if ( PrevWeight > 0 ) consumption = PrevWeight - WeightNew;  
  
  
  //--------------------------------------
  //Send Device Heartbeat every minute
  //--------------------------------------
  if (    MinuteNBR != PrevMinuteNBR ) { // New MinuteNBR - Send Heartbeat Message
       
      
      SendMessage_to_HUB("01", "Normal"); 
      tempCons=0;
  
  } ////Send Device Heartbeat
  
  
  yield(); 
  
  //If Bowl is empty refill during feeding time
  //---------------------------------------------
  if (   Weight < refill  ) {  //refill =   0.01;      //10 Grams = 0.01 Kg
     
      Refill();
      

  } // if ( Weight < refill ) {

  PetAction = 0;
  counter = 0;

  if ( Weight > 0 ) PrevWeight = Weight;  //Save Current Weight for consumption 
    
  motion = digitalRead(MOTION_PIN);
  if (motion == HIGH) PetAction = MonitorMotion(20, 10); //Monitor motion for 5 minutes or until detected (300,30)
  
      
  if (PetAction == 1) {
    
    //Weight CALCULATION - after taking content (food or water).
    int iterations = 45;
    WeightNew = GetWeight();  

    if ( PrevWeight > 0 ) consumption = PrevWeight - WeightNew; 

           
    if (consumption >= MaxConsumption) {  

      
      ++PetActionCount;
      
      consumptionTotal = consumptionTotal + consumption;
       //Pending Conversion in to Current UOM; i.e. OZ
       
      Weight = WeightNew;
      
      // Displays the consumption on the Serial Monitor
      Serial.print("Consumption: ");
      Serial.print(consumption);
      Serial.println(UOM);

      Serial.print("Total consumption: ");
      Serial.print(consumptionTotal);
      Serial.println(UOM); 

      Serial.print("Pet Action Count: ");
      Serial.println(PetActionCount);

    //   SendMessage_to_HUB("02", "Consummed"); 
      tempCons=consumption;

    }
    else {
      consumption = 0;
      tempCons=0;//
    }
  
  }
  PrevMinuteNBR = MinuteNBR;

  yield();
  delay(3000);
  
} //End of void loop() {

