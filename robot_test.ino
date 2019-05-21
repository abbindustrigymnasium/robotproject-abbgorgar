// includerar alla bibliotek 
#include <Wire.h>
#include <VL53L0X.h>
#include <Servo.h>
#include "EspMQTTClient.h"
// pins för motorn
byte Pwm_a = 5;
byte Dir_a1 =0;
// pin för analog in
int analogPin = A0;
int avr = 0;  // def analog in
// definerar två sensorer
VL53L0X sensor1;
VL53L0X sensor2;
Servo My_servo; // definerar servo  
// client funktion
void onConnectionEstablished();
// client info
EspMQTTClient client(
 "ABBIndgymIoT_2.4GHz",           // Wifi ssid
  "ValkommenHit!",           // Wifi password
  "192.168.0.102",  // MQTT broker ip
  1883,             // MQTT broker port
  "jocke",            // MQTT username
  "apa",       // MQTT password
  "microdator",          // Client name
  onConnectionEstablished, // Connection established callbak
  true,             // Enable web updater
  true              // Enable debug messages
);
// globala variabler
String riktning = "";
int DistanceSensorValue1; // vänster sensor
int DistanceSensorValue2; // fram sensor
bool off=false;  // variabeln off sätts som falsk
String riktning1 = riktning;  // variabel för riktningen

void setup(){
// pin för servo
  My_servo.attach (15); // pin för servot
  My_servo.write(90); // vinkeln från början
  pinMode(Pwm_a, OUTPUT); // döpar till output
  pinMode(Dir_a1, OUTPUT);  
  Serial.begin(9600); 
// XSHUT pins för båda sensorer    
  pinMode(10, OUTPUT);  
  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);  // sätter de till LOW
  digitalWrite(10, LOW);
  delay(500);
//pins för data och clock  
  Wire.begin(12,13);
// första sensorn som tittar vänster
  digitalWrite(14, HIGH); // sätter på den
  delay(150);  
  sensor1.init(true);
  sensor1.setAddress((uint8_t)22);  // addresserar sensorn
// andra sensorn som tittar fram
  digitalWrite(10, HIGH); // samma sak här
  delay(150);  
  sensor2.init(true);
  sensor2.setAddress((uint8_t)25);
}
// case för hur bilen ska köra 
void acc(int hastighet, int state, String riktning1){
  DistanceSensorValue1 = sensor1.readRangeSingleMillimeters();  // läser in värdet från sensor 1 (vänster)
  DistanceSensorValue2 = sensor2.readRangeSingleMillimeters();  // läser in värdet från sensor 2 (fram) 
  if(state == 1){       // bak och sedan höger
    digitalWrite(Dir_a1, HIGH); // motorn säts till HIGH det vill säga kör tillbaka
    analogWrite(Pwm_a,hastighet); // beskriver hastigheten
    My_servo.write(80); // servot säts till rakt fram
    client.publish("mess","bak"); // det som ska skrivas ut
    delay(1400);  // hur lång tid ska det ske
    digitalWrite(Dir_a1, LOW);  // motorn kör fram
    analogWrite(Pwm_a,hastighet);
    My_servo.write(170);  // servot säts till höger
    delay(1000);  
    Serial.println(riktning1);  // skriver ut riktningen
    client.publish("mess",riktning);    // skickar till brokern
  }
   else if(state == 2){ // vänster
    digitalWrite(Dir_a1, LOW);
    analogWrite(Pwm_a,hastighet);
    My_servo.write(10); // servot sätts till vänster
    delay(1200);
    Serial.println(riktning1);  
    client.publish("mess",riktning);
  } 
  else if(state == 3){  // fram
    My_servo.write(80);
    digitalWrite(Dir_a1, LOW);
    analogWrite(Pwm_a,hastighet);
    delay(500);
    Serial.println(riktning1);
    client.publish("mess",riktning);
  } 
   else if(state == 6){       // bak
    digitalWrite(Dir_a1, HIGH);
    analogWrite(Pwm_a,hastighet);
    My_servo.write(80);
    delay(1000);
    client.publish("mess","bak"); 
   }
   else if(state == 7){       // lite höger och bak
    digitalWrite(Dir_a1, HIGH);
    analogWrite(Pwm_a,hastighet);
    My_servo.write(130);
    delay(1000); 
    acc(400,3,"kör fram");
    client.publish("mess"," lite föger ach bak"); 
   }
   else if (state == 8){  // höger
    digitalWrite(Dir_a1, LOW);
    analogWrite(Pwm_a,hastighet);
    My_servo.write(170);
    delay(1000); 
    Serial.println(riktning1);
    client.publish("mess",riktning); 
   }
   else if (state == 9){  // lite vänster
    digitalWrite(Dir_a1, LOW);
    analogWrite(Pwm_a,hastighet);
    My_servo.write(50);
    Serial.println(riktning1);
    client.publish("mess",riktning); 
   }
   else if (state == 10){ //lite höger
    digitalWrite(Dir_a1, LOW);
    analogWrite(Pwm_a,hastighet);
    My_servo.write(130);
    Serial.println(riktning1);
    client.publish("mess",riktning); 
   }
}
void onConnectionEstablished(){}
void loop(){
  avr = 0;// mäter värdet från analog in (strömen från motorn)
  for (int i = 0; i < 1000; i++){ // tar alla tills det är 1000 st 
    avr = avr + analogRead(A0); // adderar tillsammans
  }
  avr = avr/1000; // delar med tusen 
  Serial.println(avr);  // skriver ut värdena
  // vilka värde ska läsa variabeln för sensors distans
  DistanceSensorValue1 = sensor1.readRangeSingleMillimeters();  
  Serial.println(DistanceSensorValue1);
  if (sensor1.timeoutOccurred()) { Serial.print(" TIMEOUT 1"); }
  DistanceSensorValue2 = sensor2.readRangeSingleMillimeters();
  Serial.println(DistanceSensorValue2);
  if (sensor2.timeoutOccurred()) { Serial.print(" TIMEOUT 2"); }
    // funktion för att köra fram
  if (DistanceSensorValue1 < 150 && DistanceSensorValue2 > 100){ // kör fram om det inte finns något framför och den är nära väggen
    acc(400,3,"kör fram");
    riktning="kör fram";
    if (avr > 400){ // om stömen ökar 
      acc(400,7,"bak"); // bakar med lite sväng till höger
    }
      if (DistanceSensorValue1 > 60){ // närmar sig med vägen
        acc(400,9,"kör lite vänster");
        if (avr > 400){
          acc(400,7,"bak");
        }
      }
      // lite höger
      else if (DistanceSensorValue1 < 60){  // åker ifrån vägen 
        acc(400,10,"kör lite höger"); // 
        if (avr > 400){
          acc(400,7,"bak");
        }
      } 
  }
    // funktion för att köra vänster
  if (DistanceSensorValue1 > 150 && DistanceSensorValue2 > 140){  // om det finns inget frame och på vänster sidan som är nära  
    acc(400,2, "kör vänster");  // kör vänster då
    riktning="kör vänster";
    DistanceSensorValue2 = sensor2.readRangeSingleMillimeters();
    if (avr > 400){
      acc(400,7,"bak");
    }
    if (DistanceSensorValue2 > 250){  // om det finns inget frame då kör fram
      acc(400,3,"kör fram");
      riktning="kör fram"; 
      delay(500);
      if (avr > 400){
      acc(400,7,"bak");
      }
    }
    else{
    acc(400,2, "kör vänster");  // annars fortsätt köra vänster
      if (avr > 400 ){
      acc(400,7,"bak");
      }
    }
  }
  // funktion för att baka sedan köra höger
  if (DistanceSensorValue2 < 100){ // om det är något framme och väldigt nära
    acc(400,1,"kör höger");
    riktning="kör höger";
    acc(400,3,"kör fram");
    riktning="kör fram"; 
    if (avr > 400 ){
      acc(400,7,"bak");
    if (DistanceSensorValue2 > 200){ // om det finns inget framme kör fram
      acc(400,3,"kör fram");
      riktning="kör fram"; 
      delay(500);
      if (avr > 400){
      acc(400,7,"bak");
      }
    }
    else{
    acc(400,8, "kör höger");  // annars fortsätt köra höger
      if (avr > 360 ){
      acc(400,7,"bak");
      }
    }
    }  
  }
  // börjar client funktionen
  client.loop(); 
}
