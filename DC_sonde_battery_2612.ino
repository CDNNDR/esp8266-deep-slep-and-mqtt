#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define wakeup 14 //la alimento sul pin D5 della scheda così si accende solo a necessità


float it;


const char* ssid = "Vodafone-A67777465";     //Vodafone-A67777465 //TP-LINK_C26C1E //My ASUS_6564 //ASUS_X00TD HUAWEI-ED66 //SantAgostino CMS_5Ghz SantAgostino-24 CMSA_Ingresso
const char* password = "e762mMLnq3r2RbCY";     //e762mMLnq3r2RbCY //oneway645 CMSA.wifi.2019 //CMSA2.wifi.2017 //CMSA.wifi.2012

//192.168.148.49 ip sensore
//gateway twt ho 192.168.1.1

IPAddress ip(192,168,1,25);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
IPAddress dns(192,168,1,1);

// mettere 13 se la sonda è collegata al pin D7 mettere 2 se la sonda è collegata l pin D4
const int oneWireBus = 13;     

OneWire oneWire(oneWireBus);


DallasTemperature sensors(&oneWire);

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

#define mqtt_topic "dtck/test_sonde_temp/1e270269-0d76-478a-b274-18deabd4bac7/+"
#define mqtt_temp_pub "dtck-pub/test_sonde_temp/1e270269-0d76-478a-b274-18deabd4bac7/TEMPERATURE"
#define mqtt_bt_pub "dtck-pub/test_sonde_temp/1e270269-0d76-478a-b274-18deabd4bac7/BATTERY"



void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(2000);

pinMode(wakeup, OUTPUT);

//lascio dormire il wifi ancora

  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  delay( 1 );


//faccio partire sensore

digitalWrite(wakeup, HIGH);
delay(1000);
sensors.begin();

sensors.requestTemperatures();  
float t = sensors.getTempCByIndex(0);

    
it = (float) t;


Serial.println(it);

// test di invio batteria

int bb = analogRead(A0);
Serial.print("A0 pin value:");
Serial.print(bb);
int tensione = bb*(3.7/1023);
int b =(tensione-3.7)*100;
Serial.println(b);


digitalWrite(wakeup, LOW); 
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
  WiFi.begin(ssid, password);
  WiFi.config(ip, dns ,gateway, subnet);
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
sendMqttData(it, b);
delay(500);

WiFi.disconnect(true);
delay(1);


ESP.deepSleep(600e6); //, WAKE_RF_DISABLED);

}

void loop()
{
  

 }

 void sendMqttData(float it, int b)
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

//publish battery
mqttClient.publish(mqtt_bt_pub, String(b).c_str(), true);



 }

  
