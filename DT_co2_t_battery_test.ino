
/* modalità deep sleep 
alimento l'scd30 con il pin d5(pin 14) al posto del 3v (non usare il D4 perchè accende il built in led della scheda)
alimento il dht22 con il pin d8 (pin 15) al posto del 3v
così li sveglio solo per la lettura
*/


#define scd_debug 0
#define SCD30WIRE Wire
#define DHTPIN 13
#define DHTTYPE DHT22 
#define dht22wakeup 15
#define read_interval 60



#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "DHT.h"
#include "paulvha_SCD30.h"
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

SCD30 airSensor;

DHT dht(DHTPIN, DHTTYPE);


float it;
int ih;
float CO2;

const char* ssid = "";     
const char* password = "";    


IPAddress ip(192,168,1,15);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
IPAddress dns(192,168,1,1);

WiFiClientSecure client;
// SHA1 fingerprint of the certificate, don't care with your GAS service
// const char* fingerprint = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6";
       

// Libreries to get time for MQTT message
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Libreries for MQTT comunication
WiFiClient mqttWifiClient;
PubSubClient mqttClient(mqttWifiClient);

// Values for MQTT broker
const char *MQTT_HOST = "mqtt.datacake.co";
const int MQTT_PORT_INSECURE = 1883;
const int MQTT_PORT = 8883;
const char *MQTT_USER = "1044b4b86f3483781bcc22968e0856eb1de20090";
const char *MQTT_PASS = "1044b4b86f3483781bcc22968e0856eb1de20090";

#define mqtt_topic "dtck/tco2battery/ec4f801a-8e2d-4460-8743-dcf48931e2e5/+"
#define mqtt_temp_pub "dtck-pub/tco2battery/ec4f801a-8e2d-4460-8743-dcf48931e2e5/TEMPERATURE"
#define mqtt_co_pub "dtck-pub/tco2battery/ec4f801a-8e2d-4460-8743-dcf48931e2e5/CO2"
#define mqtt_hm_pub  "dtck-pub/tco2battery/ec4f801a-8e2d-4460-8743-dcf48931e2e5/HUMIDITY"
#define mqtt_bt_pub  "dtck-pub/tco2battery/ec4f801a-8e2d-4460-8743-dcf48931e2e5/BATTERY"




void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(2000);


pinMode(dht22wakeup, OUTPUT);


//lascio dormire il wifi ancora

  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  delay( 1 );

// --- sensore di Co2 -------


SCD30WIRE.begin();
airSensor.setDebug(scd_debug);



//This will cause readings to occur every two seconds
  if (! airSensor.begin(SCD30WIRE))
  {
    Serial.println(F("The SCD30 did not respond. Please check wiring."));
    while(1);
  }



  //This will cause readings to occur every two seconds
airSensor.begin();

airSensor.setMeasurementInterval(read_interval);

// --- sensore di temperatura ------
digitalWrite(dht22wakeup, HIGH);
delay(500);
dht.begin();
//delay(100);


 
// void loop

float h = dht.readHumidity();
float t = dht.readTemperature();
float c = airSensor.getCO2();
    
it = (float) t-0.8;
ih = (int) h;
CO2 = (float)c;

Serial.println(it);
Serial.println(ih);
Serial.println(CO2);

// test di invio batteria
/*
int b = analogRead(A0);
Serial.print("A0 pin value:");
Serial.print(b);

*/

digitalWrite(dht22wakeup, LOW); 
 /////    -------------   connecting to internet //////////

  // sveglio il wifi

 WiFi.forceSleepWake();
delay( 1 );

// Disable the WiFi persistence.  The ESP8266 will not load and save WiFi settings in the flash memory.
WiFi.persistent( false );

// mi collego
  
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, dns, gateway, subnet);
  //WiFi.config(staticIP ,gateway, subnet);
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  
//void mqtt
sendMqttData(it, ih, CO2);
delay(500);

WiFi.disconnect(true);
delay(1);


ESP.deepSleep(600e6, WAKE_RF_DISABLED);

}

void loop()
{
  

 }

 void sendMqttData(float it, int ih, float CO2)
{
 
  mqttClient.setServer(MQTT_HOST, MQTT_PORT_INSECURE);
   while (!mqttClient.connected()) {
        Serial.print("MQTT connecting...");
        // MQTT Hostname should be empty for Datacake
        if (mqttClient.connect("", MQTT_USER, MQTT_PASS)) {
            Serial.println("connected");
            mqttClient.subscribe(mqtt_topic);
        } else {
            Serial.print("failed, status code =");
            Serial.print(mqttClient.state());
            Serial.println("try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);

        }
   }

//publish temperature
mqttClient.publish(mqtt_temp_pub, String(it).c_str(), true);

//publish co2
mqttClient.publish(mqtt_co_pub, String(CO2).c_str(), true);

//publish humidity
mqttClient.publish(mqtt_hm_pub, String(ih).c_str(), true);

//publish battery
//mqttClient.publish(mqtt_bt_pub, String(b).c_str(), true);

 }

 
