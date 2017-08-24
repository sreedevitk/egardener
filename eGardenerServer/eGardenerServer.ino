#include <string.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Ethernet.h>
#include <DHT.h>
#include "egardener.h"  


#define DHTTYPE DHT22
#define DHTPIN  2
#define LOW_STATE "low"
#define NORMAL_STATE "normal"
  
const char* ssid     = "efh";
const char* password = "srisrigay";
const String LEVEL_LOW = "LOW";
const String LEVEL_NORMAL = "NORMAL";

ESP8266WebServer server(80);

 
// Initialize DHT sensor 
// NOTE: For working with a faster than ATmega328p 16 MHz Arduino chip, like an ESP8266,
// you need to increase the threshold for cycle counts considered a 1 or 0.
// You can do this by passing a 3rd parameter for this threshold.  It's a bit
// of fiddling to find the right value, but in general the faster the CPU the
// higher the value.  The default for a 16mhz AVR is a value of 6.  For an
// Arduino Due that runs at 84mhz a value of 30 works.
// This is for the ESP8266 processor on ESP-01 
DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266
 
String moisture_level ;  // Values read from sensor
String water_level ;  // Values read from sensor

String webString="";     // String to display
char buf[2000];

// Generally, you should use "unsigned long" for variables  that hold time
unsigned long previousMillis = 0;        // will store lasttemp was read
const long interval = 2000;              // interval at which to read sensor
const int PREFERRED_BAUD_RATE = 115200; //9600;
 

void setup(void)
{
  // You can open the Arduino IDE Serial Monitor window to see what the code is doing
  Serial1.begin(PREFERRED_BAUD_RATE);  // Serial connection from ESP-01 via 3.3v console cable
  Serial.begin(PREFERRED_BAUD_RATE);

  Serial.swap();

  Serial.println("test printing");
 
  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial1.print("\n\r \n\r Working to connect");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial1.print(".");
  }
  
  Serial1.println("");
  Serial1.println("Moisture Sensor Server");
  Serial1.print("Connected to ");
  Serial1.println(ssid);//If you have errors which mention DHT then your Adafruit DHT library was not recognized
  Serial1.print("IP address: ");
  Serial1.println(WiFi.localIP());
   
  server.on("/", handle_root);
  
  server.on("/moisture", [](){  // if you add this subdirectory to your webserver call, you get text below :)
    read_serial_data();
    webString="Moisture: "+moisture_level;   // Arduino has a hard time with float to string
    webString +="\n Water: "+water_level;   // Arduino has a hard192.168.225.222 time with float to string
    server.send(200, "text/plain", webString);            // send to someones browser when asked
  });
  
 
  server.begin();
  Serial1.println("HTTP server started");

  //parse_data();
  
}

void loop(void)
{
  server.handleClient();

} 

void handle_root() {
  String htmlString = "";
  
  read_serial_data();


  htmlString += "<!DOCTYPE html>" ;
  htmlString += "<html>" ;
  htmlString += "<br>";

  htmlString += "<head>" ;
  htmlString += "<title> Moisture Sensor Web Page</title>" ;
  htmlString += "</head>" ; 
  htmlString += "<body>" ;
  htmlString += "<h1> Hello there !</h1>"  ;
  htmlString += "<p> So, here's what you were looking for </p>"  ;

  if (moisture_level == "0")
  {
    htmlString += "Moisture: "+LEVEL_LOW ; 
  } else if (moisture_level == "1")
  {
     htmlString += "Moisture: "+LEVEL_NORMAL;
  }
  
  htmlString += "<p>";
  
  if (water_level == "0")
  {
    htmlString += "Water: "+LEVEL_LOW ; 
  } else if (water_level == "1")
  {
     htmlString += "Water: "+LEVEL_NORMAL;
  }
    
  htmlString += "<p>";
  htmlString += "</body>" ;
  htmlString += "</html>" ;
  
  Serial1.println("\n html string is:  \n " +htmlString);
  server.send(200, "text/html", htmlString);
}

String construct_html() {

}

void read_serial_data() {
  
  char read_val[100];
  int i = 0;
  int serial_data_len = 100;
  
  const int moisture_len = 2;
  const int water_len = 2;
  int moisture_start;
  int moisture_end;
  int water_start;
  int water_end; 

  char temp_moisture_level[moisture_len];
  char temp_water_level[water_len];  

  
  while(Serial.peek() != -1)
  {
    // read serial data
    read_val[i] = Serial.read();

    /*
    if (i==0) {
      if (read_val[i] != 'M' || read_val[i] != 'W')
      {
          i++;
          continue;
      }
    } 
    */
    i++;

    read_val[i] = '\0';
    
    //int value =   Serial.read();
    Serial1.print("\n\r \n\r value read: ");
    Serial1.print(read_val);
  
    serial_data_len = strlen(read_val);
    
    Serial1.print("Size of serial:  " );
    Serial1.print(serial_data_len);
    
    water_end = serial_data_len-1 ;
    water_start = water_end - water_len +1;
    
    moisture_end = water_start - 1;
    moisture_start = moisture_end - moisture_len +1;
  
    int j, k;
    
    for (j= moisture_start+1,k=0 ; j< moisture_end, k<moisture_len-1; j++, k++) {
        temp_moisture_level[k] = read_val[j];
    }
    temp_moisture_level[k] ='\0';
    moisture_level = temp_moisture_level;

    Serial1.print("\n Moisture Level: ");
    Serial1.print(moisture_level); 
  
    for (j= water_start+1,k=0 ; j< water_end, k< water_len-1; j++, k++) {
        temp_water_level[k] = read_val[j];
    }
    temp_water_level[k] ='\0';
    water_level = temp_water_level;

    Serial1.print("\n Water Level: ");
    Serial1.print(water_level); 
  }
}

  
