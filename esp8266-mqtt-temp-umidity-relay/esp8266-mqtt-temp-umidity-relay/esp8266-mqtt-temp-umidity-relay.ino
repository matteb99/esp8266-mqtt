#include <Arduino.h>


//               D1 mini
// + shield DHT22(igrometer thermometer) onewire (D4)
// + MQTT adafruit library
// Libraries
#include <ESP8266WiFi.h>
//
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
//
#include <DHT.h>
#define DHTTYPE DHT22 //tipo di DHT
#define DHTPIN D4     // pin dove è collegato il DHT non cambiare in caso si usa una della  shield
#include <Wire.h>
#define relaypin D1


DHT dht(DHTPIN, DHTTYPE);
// Temperature Variables
float hum_f, temp_f;  // Values read from sensor

// WiFi parameters
#define WLAN_SSID       "SSID" //nome della propria rete wi-fi
#define WLAN_PASS       "password" //nome della propria password

// Adafruit IO
#define AIO_SERVER      "IP server" //ip server
#define AIO_SERVERPORT  1883          //porta del server
#define AIO_USERNAME    "username"   //nome utente del server
#define AIO_KEY         "passwd" //passwod del server

// Functions
void connect();


// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Store the MQTT server, client ID, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;
// Set a unique MQTT client ID using the AIO key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
const char MQTT_CLIENTID[] PROGMEM  = AIO_KEY __DATE__ __TIME__;
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>

// Setup feeds for temperature
const char TEMPERATURE_FEED[] PROGMEM = AIO_USERNAME "/feeds/temperature";
// Setup feeds for Humidity
const char HUMIDITY_FEED[] PROGMEM = AIO_USERNAME "/feeds/humidity";


Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, TEMPERATURE_FEED);
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, HUMIDITY_FEED);

/*************************** Sketch Code ************************************/
//-----start temperature variables
bool conv;
unsigned long oldtime,newtime;
float celsius,previus_celsius;
int hum,previus_hum;
//-----end temperature variables
//
void setup() {
  Serial.begin(115200);
//Wire.begin([SDA], [SCL])
Wire.begin(D8,D7);
delay(5000);
Serial.println();
Serial.println(F("+++++++++++++++++++++++++++++++++"));
Serial.println(F("+  MQTT weather station +"));
Serial.println(F("+++++++++++++++++++++++++++++++++"));

dht.begin();           // initialize temperature sensor
  // Set lamp pin to output
// Connect to WiFi access point.
  delay(10);
  Serial.print(F("Connecting to "));
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();

  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  // connect to server
  connect();

    previus_celsius=-999;
}

void loop() {

//++++++++++++++++++++++++++++++++START TEMP+++++++++++++++++++++++++++++++++

  if (conv == false) {
    conv=true;
    oldtime=millis();
     }

//
//++++++++++++++++++++++++++++++++END TEMP+++++++++++++++++++++++++++++++++

  Adafruit_MQTT_Subscribe *subscription;

  // ping server io a few times to make sure we remain connected
  if(! mqtt.ping(3)) {
    // reconnect to server io
    if(! mqtt.connected())
      connect();
  }



newtime=millis();
// Read Temperature   humidity pressure
if (conv & (newtime-oldtime)> 2000 ) {

  hum_f = dht.readHumidity();          // Read humidity (percent)
    temp_f = dht.readTemperature();     // Read temperature as Celsius

     if (isnan(hum_f) || isnan(temp_f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;}



  conv=false;
  celsius = temp_f;
  celsius=celsius+0.05;
  celsius=int (celsius*10) ;
  celsius=celsius/10.;
  if (celsius!=previus_celsius) {
                    if (! temperature.publish(celsius))
                    Serial.println(F("Failed to publish temperature"));
                      else
                      {
                         Serial.print(F("  Temperature = "));
                         Serial.print(celsius);
                         Serial.print(F(" Celsius, "));
                         Serial.println(F("published!"));
                         previus_celsius=celsius;
                    }
                    }
// humidity
  hum_f = (hum_f+0.5)*10;
  hum=hum_f/10;// round nearest int

  if (hum!=previus_hum) {
                    if (! humidity.publish(hum))
                    Serial.println(F("Failed to publish humidity"));
                      else
                      {
                         Serial.print(F("  humidity = "));
                         Serial.print(hum);
                         Serial.print(F("% "));
                         Serial.println(F("published!"));
                         previus_hum=hum;
                    }


                  }
// End Read Temperature
}




  }

// connect to server io via MQTT
void connect() {

  Serial.print(F("Connessione al server... "));

  int8_t ret;

  while ((ret = mqtt.connect()) != 0) {

    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(5000);

  }

  Serial.println(F("server Connected!"));

}
