#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define WIFI_SSID "ENTER_WIFI_SSID"
#define WIFI_PASSWORD "ENTER_WIFI_PASS"

#define API_KEY "ENTER_FIREBASE_RTDB_API_KEY"
#define DATABASE_URL "ENTER_RTDB_URL" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

//USonic params
const int trigPin = 12; //D6
const int echoPin = 14; //D5
#define soundVelocity 0.034
#define cmToInch 0.393701
long duration;
float distanceCm, distanceInch;

void setup(){
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;
  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

}

void loop(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * soundVelocity/2;
  distanceInch = distanceCm * cmToInch;

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 3000/*DELAY*/ || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    if (Firebase.RTDB.setFloat(&fbdo, "test/distance in CM", distanceCm)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());

      Serial.print("Distance (cm): ");
      Serial.println(distanceCm);
      delay(3000);
    }

    if (Firebase.RTDB.setFloat(&fbdo, "test/distance in INCH", distanceInch)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      
      Serial.print("Distance (inch): ");
      Serial.println(distanceInch);
      delay(3000);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}

