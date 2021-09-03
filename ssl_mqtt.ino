#include <ssl_client.h>
#include "DHT.h"
#include "MQ135.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//Topics

#define mqttTemperature "mesure/temperature"
#define mqttHumidite "mesure/humidite"
#define mqttco2 "mesure/co2"
/*#define mqttnh4 "mesure/nh4"
#define mqttacetone "mesure/acetone"
#define mqtttoluene "mesure/toluene"
#define mqttethanol "mesure/ethanol"*/

//Card Id
String CardId = "Hach" ;


//MQTT 
#define mqtt_server "192.168.1.19"
//#define mqtt_host  "DESKTOP-3TRO9B7"
  

//Wifi
 
const char* ssid     =     "TT_6028";                  // ESP32 and ESP8266 uses 2.4GHZ wifi only  "ooredoo_70E8A8" ; 
const char* password =     "i9ikc3d8zj";                       //"3MJJJJMF3XCCM" 
WiFiClientSecure espClient;
PubSubClient client(espClient);


//Sensors
const int mq135Pin = 35; 
MQ135 gasSensor = MQ135(mq135Pin);  // Initialise l'objet MQ135 sur le Pin spécifié    
#define DHTPIN 32     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);


//Time 

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;



//certificate


const char* ca_cert =\
"-----BEGIN CERTIFICATE-----\n"\
"MIIEDzCCAvegAwIBAgIUbgM1P+Ex9dJj9t9osNXMZVK7YDcwDQYJKoZIhvcNAQEL\n" \
"BQAwgZYxCzAJBgNVBAYTAlROMRAwDgYDVQQIDAdUdW5pc2lhMQ4wDAYDVQQHDAVU\n" \
"dW5pczEOMAwGA1UECgwFVGFsYW4xEjAQBgNVBAsMCWlvdF9UYWxhbjEXMBUGA1UE\n" \
"AwwOMTkyLjE2OC40My4xNTExKDAmBgkqhkiG9w0BCQEWGWdoYXJiaS5oYWNoZW05\n" \
"OEBnbWFpbC5jb20wHhcNMjEwNzI5MTcyNDIxWhcNMjYwNzI5MTcyNDIxWjCBljEL\n" \
"MAkGA1UEBhMCVE4xEDAOBgNVBAgMB1R1bmlzaWExDjAMBgNVBAcMBVR1bmlzMQ4w\n" \
"DAYDVQQKDAVUYWxhbjESMBAGA1UECwwJaW90X1RhbGFuMRcwFQYDVQQDDA4xOTIu\n" \
"MTY4LjQzLjE1MTEoMCYGCSqGSIb3DQEJARYZZ2hhcmJpLmhhY2hlbTk4QGdtYWls\n" \
"LmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALO9HCbbDBLOyxRa\n" \
"XPrztPiBJOi+BVz3wdwY+OsCGAbGI9eJGqIuXK1ipWpwsyp/NHlp+ho4GALKOqT1\n" \
"Rj2qN8oQup3oUP0Crzm7AhDjn0Uzx1De8hCCHdKC1aB9JMM2uyi0uZlnegZRo43V\n" \
"TB1Vl3AJkHUa2HQH1m9XGna8Tq3/uObf9RlX9M1s+C7A0wI4LGCBufsKxpg0G0K2\n" \
"PC4MZP8yho59DaV584VpKWTlGYYDRrOPcfCO6Ht3KWYDui8qucKGC0BSzY35c8y5\n" \
"0kLh8546vf5WadWXrBaQHJFSKNxwgjqREJXH+p/jmmGTWZLfhj31xtv8XfHZcwSC\n" \
"qwMM218CAwEAAaNTMFEwHQYDVR0OBBYEFKEdRN6fvPPijQLIZIdB3SG368eMMB8G\n" \
"A1UdIwQYMBaAFKEdRN6fvPPijQLIZIdB3SG368eMMA8GA1UdEwEB/wQFMAMBAf8w\n" \
"DQYJKoZIhvcNAQELBQADggEBADoVXNkDPFYO1+3yZi4zoUxI1DfRvRuKCb+bwWYC\n" \
"vmGXQXsuVETaH5CC7U0lwiegc8C6t/zzVn97PhVkilmetDtbdX7rGBi82PZAuhpK\n" \
"L5qOO5Iaur4efiNVSrHXuWNrGiEozbMbPz5X/7GVKRou6wfcX2AYnA3N5Zlh/NFH\n" \
"dGhvszylSoDoBZH7Yu8i6sfQmV5IVU0xDeM9zqsCIjXbW2K25nP9f+3N6hRuTNXC\n" \
"iAvDTi0xw/04UT7Pj+mNXWofGopMSewM355YayJ4ptQ/PbIsBF0sCgtyQ+VyBF4K\n" \
"JLDGH+l379dIu7XFH1IoOsZIqXcdNvHfVcHWQMRICZC3YTk=\n" \
"-----END CERTIFICATE-----" ;



//MQTT connect method

  void reconnect() {
  // Loop until we're reconnected
  int counter = 0;
  while (!client.connected()) {
    if (counter==5){
      ESP.restart();
    }
    counter+=1;
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
   
    if (client.connect("esp")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}





void setup() {
  Serial.begin(9600);
    
//   begin Wifi connect

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(2000);
  WiFi.begin(ssid, password);
  
 while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //end Wifi connect

  //Mqtt port
 espClient.setCACert(ca_cert);
 client.setServer(mqtt_server, 1884);

//Start temperature sensor    
  dht.begin();

//Time

// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(3600);
  

  //Security connect ssl

   /*setup MDNS for ESP32 */
/*  if (!MDNS.begin("esp32")) {
      Serial.println("Error setting up MDNS responder!");
      while(1) {
          delay(1000);
      }
  }
  /* get the IP address of server by MDNS name */
 /* Serial.println("mDNS responder started");
  IPAddress serverIp = MDNS.queryHost(mqtt_host);
  Serial.print("IP address of server: ");
  Serial.println(serverIp.toString());
  /* set SSL/TLS certificate */
 /* espClient.setCACert(ca_cert);
  /* configure the MQTT server with IPaddress and port */
 /* client.setServer(serverIp, 8883);
  */
  
  
    
  
}



void loop() {

  //Connect to MQTT

if (!client.connected()){
   reconnect();
  }

 //Gaz Values
    float ppm = gasSensor.getPPM();
    float Ethanol = gasSensor.getEthanol();
    float NH4 = gasSensor.getNH4();
    float Toluene = gasSensor.getToluene();
    float Acetone = gasSensor.getAcetone();
    
    Serial.print(" gazes:co2 ethanol nh4 toluene acetone ");
    Serial.println(ppm);
    Serial.println(Ethanol);
    Serial.println(NH4);
    Serial.println(Toluene);
    Serial.println(Acetone);
       

  //client.loop();

 //Temperature and Humidity
 
  float h = dht.readHumidity();
  
  float t = dht.readTemperature();
  

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
   Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
 


//Time

  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z

  formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);
  
  
 
 
//Define data to send json

  //Temperature
  
 StaticJsonDocument<500> doc;
  JsonObject root = doc.to<JsonObject>();

  root["value"] = t;
  root["type"]="temperature";
  root["dates"]=formattedDate;  
  root["card"]=CardId ;
  
char Message[200];
  
    serializeJsonPretty(doc, Message);
    Serial.print(Message);
    Serial.println();

  
  //Humidity
     StaticJsonDocument<500> doc1;
  JsonObject root1 = doc1.to<JsonObject>();

  
  root1["value"] = h;
  root1["type"]="humidite";
  root1["dates"]=formattedDate;
  root1["card"]=CardId ;
    

  char Message1[200];
  
    serializeJsonPretty(doc1, Message1);
    Serial.print(Message1);
    Serial.println();

  //Co2  

     StaticJsonDocument<500> doc2;
  JsonObject root2 = doc2.to<JsonObject>();
  
  root2["value"] = ppm;
  root2["type"]="co2";
  root2["dates"]=formattedDate; 
  root2["card"]=CardId ;
   

  char Message2[200];
  
    serializeJsonPretty(doc2, Message2);
    Serial.print(Message2);
    Serial.println();

    //Ethanol 

     StaticJsonDocument<500> doc3;
  JsonObject root3 = doc3.to<JsonObject>();
  
  root3["value"] = Ethanol;
  root3["type"]="ethanol";
  root3["dates"]=formattedDate; 
  root3["card"]=CardId ;
   

  char Message3[200];
  
    serializeJsonPretty(doc3, Message3);
    Serial.print(Message3);
    Serial.println();

//NH4

     StaticJsonDocument<500> doc4;
  JsonObject root4 = doc4.to<JsonObject>();
  
  root4["value"] = NH4;
  root4["type"]="nh4";
  root4["dates"]=formattedDate; 
  root4["card"]=CardId ;
   

  char Message4[200];
  
    serializeJsonPretty(doc4, Message4);
    Serial.print(Message4);
    Serial.println();


 //Toluene 

     StaticJsonDocument<500> doc5;
  JsonObject root5 = doc5.to<JsonObject>();
  
  root5["value"] = Toluene;
  root5["type"]="toluene";
  root5["dates"]=formattedDate; 
  root5["card"]=CardId ;
   

  char Message5[200];
  
    serializeJsonPretty(doc5, Message5);
    Serial.print(Message5);
    Serial.println();

 //Acetone 

     StaticJsonDocument<500> doc6;
  JsonObject root6 = doc6.to<JsonObject>();
  
  root6["value"] = Acetone;
  root6["type"]="acetone";
  root6["dates"]=formattedDate; 
  root6["card"]=CardId ;
   

  char Message6[200];
  
    serializeJsonPretty(doc6, Message6);
    Serial.print(Message6);
    Serial.println();
    
//Publish data 5s in between  each minute

  client.publish(mqttTemperature, Message);
  delay(5000) ;
  
  client.publish(mqttHumidite, Message1);
  delay(5000) ;
  
  client.publish(mqttco2, Message2);
  delay(5000) ;
  
  client.publish(mqttethanol, Message3);
  delay(5000) ;
  
  client.publish(mqttnh4, Message4);
  delay(5000) ;
  
  client.publish(mqtttoluene, Message5);
  delay(5000) ;
  
  client.publish(mqttacetone, Message6);
  delay(30000) ;
    
    

}
