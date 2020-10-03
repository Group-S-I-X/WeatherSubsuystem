//group6

//DHT11
#include "DHT.h"
#define DHTPIN 2     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

// anemometer
const byte interruptPin = 3; //anemomter input to digital pin
volatile unsigned long sTime = 0; //stores start time for wind speed calculation
unsigned long dataTimer = 0; //used to track how often to communicate data
volatile float pulseTime = 0; //stores time between one anemomter relay closing and the next
volatile float culPulseTime = 0; //stores cumulative pulsetimes for averaging
volatile bool start = true; //tracks when a new anemometer measurement starts
volatile unsigned int avgWindCount = 0; //stores anemometer relay counts for doing average wind speed
float aSetting = 60.0; //wind speed setting to signal alarm
float wSpeedMPH1 =0;



void setup() {
  //DHT11
  Serial.begin(9600);
  //Serial.println(F("DHTxx test!"));
  dht.begin();
  //wind sensor
  pinMode(13, OUTPUT); //setup LED pin to signal high wind alarm condition
  pinMode(interruptPin, INPUT_PULLUP); //set interrupt pin to input pullup
  attachInterrupt(interruptPin, anemometerISR, RISING); //setup interrupt on anemometer input pin, interrupt will occur whenever falling edge is detected
  dataTimer = millis(); //reset loop timer
}

void loop() {
  
  
  //wind sensor
  unsigned long rTime = millis();
  if((rTime - sTime) > 2500) pulseTime = 0; //if the wind speed has dropped below 1MPH than set it to zero
     
  if((rTime - dataTimer) > 1800){ //See if it is time to transmit
   
    detachInterrupt(interruptPin); //shut off wind speed measurement interrupt until done communication
    float aWSpeed = getAvgWindSpeed(culPulseTime,avgWindCount); //calculate average wind speed
    if(aWSpeed >= aSetting) digitalWrite(13, HIGH);   // high speed wind detected so turn the LED on
    else digitalWrite(13, LOW);   //no alarm so ensure LED is off
    culPulseTime = 0; //reset cumulative pulse counter
    avgWindCount = 0; //reset average wind count

    float aFreq = 0; //set to zero initially
    if(pulseTime > 0.0) aFreq = getAnemometerFreq(pulseTime); //calculate frequency in Hz of anemometer, only if pulsetime is non-zero
    float wSpeedMPH = getWindMPH(aFreq); //calculate wind speed in MPH, note that the 2.5 comes from anemometer data sheet
   
//    Serial.begin(57600); //start serial monitor to communicate wind data
//    Serial.println();
//    Serial.println("...................................");
//    Serial.print("Anemometer speed in Hz ");
//    Serial.println(aFreq);
//    Serial.print("Current wind speed is ");
      wSpeedMPH = 40;
      wSpeedMPH1 = wSpeedMPH;
//    Serial.println(wSpeedMPH);
//    Serial.print("Current average wind speed is ");
//    Serial.println(aWSpeed);
//    Serial.end(); //serial uses interrupts so we want to turn it off before we turn the wind measurement interrupts back on
   
    start = true; //reset start variable in case we missed wind data while communicating current data out
    attachInterrupt(digitalPinToInterrupt(interruptPin), anemometerISR, RISING); //turn interrupt back on
    dataTimer = millis(); //reset loop timer
  }
//DHT11
  // Wait a few seconds between measurements.
  delay(2000);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);
  
//  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(" ");
//  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(" ");
//  Serial.print(F("C degree "));
  Serial.print(f);
  Serial.print(" ");
//  Serial.print(F("F degree  Heat index: "));
  Serial.print(hic);
  Serial.print(" ");
//  Serial.print(F("C degree "));
  Serial.print(hif);
  Serial.print(" ");
//  Serial.print(F("F degree "));
//  Serial.print(F("Windspeed : "));
  Serial.println(wSpeedMPH1);
//  Serial.println(F(" mph"));
  int condition =0;
  if(wSpeedMPH1<35 && hic<28){
    condition =1;
  }
  if(wSpeedMPH1<45 && wSpeedMPH1>35 && hic<35){
    condition =2;
  }
  if(wSpeedMPH1>50 && hic>35){
    condition =3;
  }
//  Serial.print(F("Alarm : "));
//  Serial.println(condition);
//  
}

//using time between anemometer pulses calculate frequency of anemometer
float getAnemometerFreq(float pTime) { return (1/pTime); }
//Use anemometer frequency to calculate wind speed in MPH, note 2.5 comes from anemometer data sheet
float getWindMPH(float freq) { return (freq*2.5); }
//uses wind MPH value to calculate KPH
float getWindKPH(float wMPH) { return (wMPH*1.61); }
//Calculates average wind speed over given time period
float getAvgWindSpeed(float cPulse,int per) {
  if(per) return getWindMPH(getAnemometerFreq((float)(cPulse/per)));
  else return 0; //average wind speed is zero and we can't divide by zero
  }

//This is the interrupt service routine (ISR) for the anemometer input pin
//it is called whenever a falling edge is detected
void anemometerISR() {
  unsigned long cTime = millis(); //get current time
  if(!start) { //This is not the first pulse and we are not at 0 MPH so calculate time between pulses
   // test = cTime - sTime;
    pulseTime = (float)(cTime - sTime)/1000;
    culPulseTime += pulseTime; //add up pulse time measurements for averaging
    avgWindCount++; //anemomter went around so record for calculating average wind speed
  }
  sTime = cTime; //store current time for next pulse time calculation
  start = false; //we have our starting point for a wind speed measurement
}
