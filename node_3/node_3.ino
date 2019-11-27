#include <SDS011.h>
//#include <MiCS6814-I2C.h>
#include "DHT.h"
#include "MutichannelGasSensor.h"
#include "ThingSpeak.h"
#include "secrets.h"
#include <ESP8266WiFi.h>

#define DHTPIN D3     
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

#define ADDR_I2C 0x04



float p10,p25;
int error;

SDS011 my_sds;

float f1,f2,f3,f4,f5,f6,f7;

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

String myStatus = "";



String CSE_IP      = "onem2m.iiit.ac.in";   //ch
// #######################################################

int WIFI_DELAY  = 100; //ms

// oneM2M : CSE params
int   CSE_HTTP_PORT = 443;
String CSE_NAME    = "in-name";
String CSE_M2M_ORIGIN  = "admin:admin";

// oneM2M : resources' params
String DESC_CNT_NAME = "DESCRIPTOR";
String DATA_CNT_NAME = "DATA";
String CMND_CNT_NAME = "COMMAND";
int TY_AE  = 2;
int TY_CNT = 3;
int TY_CI  = 4;
int TY_SUB = 23;

// HTTP constants
int LOCAL_PORT = 9999;
char* HTTP_CREATED = "HTTP/1.1 201 Created";
char* HTTP_OK    = "HTTP/1.1 200 OK\r\n";
int REQUEST_TIME_OUT = 5000; //ms


String doPOST(String url, int ty, String rep) {

  String postRequest = String() + "POST " + url + " HTTP/1.1\r\n" +
                       "Host: " + CSE_IP + ":" + CSE_HTTP_PORT + "\r\n" +
                       "X-M2M-Origin: " + CSE_M2M_ORIGIN + "\r\n" +
                       "Content-Type: application/json;ty=" + ty + "\r\n" +
                       "Content-Length: " + rep.length() + "\r\n"
                       "Connection: close\r\n\n" +
                       rep;

  // Connect to the CSE address

//  Serial.println("connecting to " + CSE_IP + ":" + CSE_HTTP_PORT + " ...");

  // Get a client
  WiFiClient client2;
  if (!client2.connect(CSE_IP, CSE_HTTP_PORT)) {
    Serial.println("Connection failed !");
    return "error";
  }

  // if connection succeeds, we show the request to be send
#ifdef DEBUG
//  Serial.println(postRequest);
#endif

  // Send the HTTP POST request
  client2.print(postRequest);

  // Manage a timeout
  unsigned long startTime = millis();
  while (client2.available() == 0) {
    if (millis() - startTime > REQUEST_TIME_OUT) {
//      Serial.println("Client Timeout");
      client2.stop();
      return "error";
    }
  }

  // If success, Read the HTTP response
  String result = "";
  if (client2.available()) {
    result = client2.readStringUntil('\r');
        Serial.println(result);
  }
  while (client2.available()) {
    String line = client2.readStringUntil('\r');
//    Serial.print(line);
  }
  Serial.println();
  Serial.println("closing connection...");
  return result;
}


String createCI(String ae, String cnt, String ciContent) {
  String ciRepresentation =
    "{\"m2m:cin\": {"
    "\"con\":\"" + ciContent + "\""
    "}}";
  return doPOST("/" + CSE_NAME + "/" + ae + "/" + cnt, TY_CI, ciRepresentation);
}


void dhtReading(){
    // Wait a few seconds between measurements.

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  f1 = h;
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  f2 = t;
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

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  Serial.println(" ");

}

void sdsReading(){
  error = my_sds.read(&p25,&p10);
  f3 = p25;
  f4 = p10;
  
  if (! error) {
    Serial.println("P2.5: "+String(p25));
    Serial.println("P10:  "+String(p10));
  }
  delay(100);

  Serial.println(" ");
}

void groveReading(){
    f5 = gas.measure_CO();
    Serial.print("The concentration of CO is ");
    if(f5>=0) Serial.print(f5);
    else Serial.print("invalid");
    Serial.println(" ppm");
    
    
    f6 = gas.measure_NO2();
    Serial.print("The concentration of NO2 is ");
    if(f6>=0) Serial.print(f6);
    else Serial.print("invalid");
    Serial.println(" ppm");
    

     f7 = gas.measure_NH3();
    Serial.print("The concentration of NH3 is ");
    if(f7>=0) Serial.print(f7);
    else Serial.print("invalid");
    Serial.println(" ppm");
    

}

void setup() {
  Serial.begin(115200);  // Initialize serial

  //Serial.begin(9600);

  // DHT setup
  Serial.println(F("DHTxx test!"));
  dht.begin();
  
  // SDS setup
  my_sds.begin(D5,D6);

  // Grove setup
  gas.begin(ADDR_I2C);
  gas.powerOn();  

  // Initialize serial connection

  // Connect to sensor using default I2C address (0x04)
  // Alternatively the address can be passed to begin(addr)
//  sensorConnected = sensor.begin();
//
//  if (sensorConnected == true) {
//    // Print status message
//    Serial.println("Connected to MiCS-6814 sensor");
//
//    // Turn heater element on
//    sensor.powerOn();
//    
//    // Print header for live values
//    Serial.println("Current concentrations:");
//    Serial.println("CO\tNO2\tNH3\tC3H8\tC4H10\tCH4\tH2\tC2H5OH");
//  } else {
//    // Print error message on failed connection
//    Serial.println("Couldn't connect to MiCS-6814 sensor");
//  }
  
  WiFi.mode(WIFI_STA); 
  groveReading();
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }

  
 
  

  // set the fields with the values
  dhtReading();
  ThingSpeak.setField(1, f1);
  ThingSpeak.setField(2, f2);
   sdsReading();
  ThingSpeak.setField(3, f3);
  ThingSpeak.setField(4, f4);
  groveReading();
  ThingSpeak.setField(5, f5);
  ThingSpeak.setField(6, f6);
  ThingSpeak.setField(7, f7);
  
  // set the status
  ThingSpeak.setStatus(myStatus);
  
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  String sensor_value_string;
  sensor_value_string = String(f2) + String(",") + String(f1) + String(",") + String(f3) + String(",") + String(f4) + String(",") + String(f5) + String(",") + String(f6) + String(",") + String(f7) ;
//  createCI("Team20_Outdoor_air_pollution_2", "node_3", sensor_value_string);    //ch
  
  delay(20000); // Wait 20 seconds to update the channel again
}
