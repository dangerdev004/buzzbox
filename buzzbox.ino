/*
 * Twilio SMS and MMS on ESP8266 Example.
 */


#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>

#include "twilio.hpp"

// defines pins numbers
int trigPin = D1;
int echoPin = D2;
int buzzer = D3;
int ledPin = D4;
int irPin = D5;
int motionPin = D6;

// defines variables
long ultrasonicduration;
int ultrasonicdistance;
int safetyDistance;
int ir_in;

#define USE_SOFTWARE_SERIAL 0

// Print debug messages over serial?
#define USE_SERIAL 1

// Your network SSID and password
const char* ssid = "";
const char* password = "";

// Find the api.twilio.com SHA1 fingerprint, this one was valid as of July 2020. 
const char fingerprint[] = "94 F4 74 8E C7 25 00 D6 C3 2A F9 E9 D8 9B 2A 2F 71 25 85 C9";

// Twilio account specific details, from https://twilio.com/console
const char* account_sid = "";
const char* auth_token = "";

// Details for the SMS we'll send with Twilio.  Should be a number you own 
// (check the console, link above).
String to_number    = "";
String from_number = "";
String message_body    = "DANGER!!! Elephants Detected";

// The 'authorized number' to text the ESP8266 for our example
String master_number    = "";

// Optional - a url to an image.  See 'MediaUrl' here: 
// https://www.twilio.com/docs/api/rest/sending-messages
String media_url = "";

// Global twilio objects
Twilio *twilio;
ESP8266WebServer twilio_server(8000);

//  Optional software serial debugging
#if USE_SOFTWARE_SERIAL == 1
#include <SoftwareSerial.h>
// You'll need to set pin numbers to match your setup if you
// do use Software Serial
extern SoftwareSerial swSer(14, 4, false, 256);
#else
#define swSer Serial//     twilio_server.begin();
#endif

/*
 * Callback function when we hit the /message route with a webhook.
 * Use the global 'twilio_server' object to respond.
 */
void handle_message() {
        #if USE_SERIAL == 1
        swSer.println("Incoming connection!  Printing body:");
        #endif
        bool authorized = false;
        char command = '\0';

        // Parse Twilio's request to the ESP
        for (int i = 0; i < twilio_server.args(); ++i) {
                #if USE_SERIAL == 1
                swSer.print(twilio_server.argName(i));
                swSer.print(": ");
                swSer.println(twilio_server.arg(i));
                #endif

                if (twilio_server.argName(i) == "From" and 
                    twilio_server.arg(i) == master_number) {
                    authorized = true;
                } else if (twilio_server.argName(i) == "Body") {
                        if (twilio_server.arg(i) == "?" or
                            twilio_server.arg(i) == "0" or
                            twilio_server.arg(i) == "1") {
                                command = twilio_server.arg(i)[0];
                        }
                }
        } // end for loop parsing Twilio's request


}

void setup() {
        pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
        pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
        pinMode(buzzer, OUTPUT);
        pinMode(irPin, INPUT);
        pinMode(ledPin, OUTPUT);
        pinMode(motionPin, INPUT);
        // Clears the trigPin
        digitalWrite(trigPin, LOW);
        digitalWrite(buzzer, LOW);
        digitalWrite(ledPin,LOW);
        Serial.begin(115200); // Starts the serial communication
        WiFi.begin(ssid, password);
        twilio = new Twilio(account_sid, auth_token, fingerprint);

        #if USE_SERIAL == 1
        swSer.begin(115200);
        while (WiFi.status() != WL_CONNECTED) {
                delay(1000);
                swSer.print(".");
        }
        swSer.println("");
        swSer.println("Connected to WiFi, IP address: ");
        swSer.println(WiFi.localIP());
        #else
        while (WiFi.status() != WL_CONNECTED) delay(1000);
        #endif

}


/* 
 *  In our main loop, we listen for connections from Twilio in handleClient().
 */
void loop() {
twilio_server.handleClient();
  
  
  ultrasonicduration = pulseIn(echoPin, HIGH, 30000);  // Increase the timeout to 30 milliseconds

  // Calculating the distance
  ultrasonicdistance = ultrasonicduration * 170;

  safetyDistance = ultrasonicdistance;
      
        
if (safetyDistance <= 2 && digitalRead(irPin) == LOW && digitalRead(motionPin) == HIGH) 
{
     
     // Response will be filled with connection info and Twilio API responses
     // from this initial SMS send.
     String response;
     bool success = twilio->send_message(
             to_number,
             from_number,
             message_body,
             response,
             media_url
     );
      
     #if USE_SERIAL == 1
     swSer.println(response);
     #endif
     twilio_server.on("/hello-messaging", handle_message);
     twilio_server.begin();
     digitalWrite(buzzer, HIGH);
     digitalWrite(ledPin,HIGH);
     delay(250);
     digitalWrite(ledPin,LOW);
     delay(250);
} else {
  
    digitalWrite(buzzer, LOW);
    digitalWrite(ledPin, LOW);
}
  // Prints the distance on the Serial Monitor
  ir_in=digitalRead(irPin);
  Serial.begin(9600);
  Serial.print("IR: ");
  Serial.println(irPin);
  Serial.print("Motion: ");
  Serial.println(motionPin);
  Serial.print("Distance: ");
  Serial.println(safetyDistance);
  delay(1000);
}
