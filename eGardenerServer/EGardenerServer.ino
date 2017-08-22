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
//WiFiServer server(80);

 
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
 

void setup(void)
{
  // You can open the Arduino IDE Serial Monitor window to see what the code is doing
  Serial1.begin(115200);  // Serial connection from ESP-01 via 3.3v console cable
  Serial.begin(115200);

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
  Serial1.println("DHT Moisture Reading Server");
  Serial1.print("Connected to ");
  Serial1.println(ssid);//If you have errors which mention DHT then your Adafruit DHT library was not recognized (go back and follow the install steps again).  
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
//  String html_string = MAIN_PAGE;
  
  read_serial_data();

  //htmlString += "HTTP/1.1 200 OK" ;
  //htmlString += "<br>";
  
  //htmlString += "Content-Type: text/html" ;
  //htmlString += "<br>";

  //htmlString += "Connection: close" ;

  htmlString += "<!DOCTYPE html>" ;
  htmlString += "<html>" ;
  htmlString += "<br>";

  htmlString += "<head>" ;
  htmlString += "<title>Arduino Web Page</title>" ;
  htmlString += "</head>" ; 
  htmlString += "<body>" ;
  htmlString += "<h1>Hello from Arduino!</h1>"  ;
  htmlString += "<p>A web page from my Arduino server</p>"  ;

  if (moisture_level == "0")
  {
    htmlString += "Moisture: "+LEVEL_LOW ; 
  } else if (moisture_level == "1")
  {
     htmlString += "Moisture: "+LEVEL_NORMAL;
  }
  
  htmlString += "<p>";
  
 
  htmlString += "Water: "+water_level  ;   
  htmlString += "<p>";
  htmlString += "</body>" ;
  htmlString += "</html>" ;
  
  
  Serial1.println("\n html string is:  \n " +htmlString);
  server.send(200, "text/html", htmlString);

  delay(100);
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
    read_val[i] = Serial.read();
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
        //Serial1.print("\n Moisture Level char: ");
        //Serial1.print(moisture_level[j]);
    }
    temp_moisture_level[k] ='\0';
    moisture_level = temp_moisture_level;

    /*
    if (*temp_moisture_level== '0') {
      moisture_level = LEVEL_LOW;
    } else if (*temp_moisture_level== '1') {
      moisture_level = LEVEL_NORMAL;
    }
    */
    
    Serial1.print("\n Moisture Level: ");
    Serial1.print(moisture_level); 
  
    for (j= water_start+1,k=0 ; j< water_end, k< water_len-1; j++, k++) {
        temp_water_level[k] = read_val[j];
        //Serial1.print("\n Moisture Level char: ");
        //Serial1.print(moisture_level[j]);
    }
    temp_water_level[k] ='\0';
    water_level = temp_water_level;

    /*
    if (*temp_water_level== '0') {
      water_level = LEVEL_LOW;
    } else if (*temp_moisture_level=='1') {
      water_level = LEVEL_NORMAL;
    }
    */
  
    Serial1.print("\n Water Level: ");
    Serial1.print(water_level); 
  }
}


/*

String print_data_on_page() {
                String htmlString = "";
                htmlString += "HTTP/1.1 200 OK";
                htmlString += "Content-Type: text/html" ;
                htmlString += "Connection: close" ;
                htmlString += "<!DOCTYPE html>" ;
                htmlString += "<html>" ;
                htmlString += "<head>" ;
                htmlString += "<title>Arduino Web Page</title>" ;
                htmlString += "</head>" ;
                htmlString += "<body>" ;
                htmlString += "<h1>Hello from Arduino!</h1>" ;
                htmlString += "<p>A web page from my Arduino server</p>" ;
                htmlString += "Moisture: "+moisture_level + "<br>";   
                htmlString += "Water: "+water_level  + "<br>";   
                htmlString += "</body>" ;
                htmlString += "</html>" ;
}


void print_data_on_page(){
  
EthernetClient client = server.available();  // try to get client
  if(client) // got client?
  {          
    boolean currentLineIsBlank = true;
    while (client.connected()) {
        if (client.available()) {   // client data available to read
            char c = client.read(); // read 1 byte (character) from client
            // last line of client request is blank and ends with \n
            // respond to client only after last line received
            if (c == '\n' && currentLineIsBlank) {
                // send a standard http response header
                client.println("HTTP/1.1 200 OK");
                client.println("Content-Type: text/html");
                client.println("Connection: close");
                client.println();
                // send web page
                client.println("<!DOCTYPE html>");
                client.println("<html>");
                client.println("<head>");
                client.println("<title>Arduino Web Page</title>");
                client.println("</head>");
                client.println("<body>");
                client.println("<h1>Hello from Arduino!</h1>");
                client.println("<p>A web page from my Arduino server</p>");
                client.println("</body>");
                client.println("</html>");
                break;
            }
            // every line of text received from the client ends with \r\n
            if (c == '\n') {
                // last character on line of received text
                // starting new line with next character read
                currentLineIsBlank = true;
            } 
            else if (c != '\r') {
                // a text character was received from client
                currentLineIsBlank = false;
            }
        } // end if (client.available())
    } // end while (client.connected())
    delay(1);      // give the web browser time to receive the data
    client.stop(); // close the connection
  } // end if (client)
}


void setup(void)
{
  //  setup1();
  moisture_level = LOW_STATE;
  water_level = NORMAL_STATE; 

  // You can open the Arduino IDE Serial Monitor window to see what the code is doing
  Serial1.begin(115200);  // Serial connection from ESP-01 via 3.3v console cable  
  //Serial.begin(115200);
  
  //Serial1.swap();
  
  dht.begin();           // initialize temperature sensor
 
  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial1.print("\n\r \n\r Working to connect 1");
  Serial1.print("\n\r \n\rSerial1 connect");

 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial1.print(".");
  }
  Serial1.println("");
  Serial1.println("DHT Moisture Reading Server");
  Serial1.print("Connected to ");
  Serial1.println(ssid);//If you have errors which mention DHT then your Adafruit DHT library was not recognized (go back and follow the install steps again).  
  Serial1.print("IP address: ");
  Serial1.println(WiFi.localIP());
   
  server.on("/", handle_root);
  
  server.on("/moisture", [](){  // if you add this subdirectory to your webserver call, you get text below :)
    getmoisture();       // read sensor
    webString="Hi, its working";   // Arduino has a hard time with float to string
    server.send(200, "text/plain", webString);            // send to someones browser when asked
  });
   
  server.begin();
  Serial1.println("HTTP server started");
}


void parse_data() {
//  char input_data[] = "1moisture=low:water=normal,2moisture=normal:water=normal,3moisture=normal:water=high";
//  const char delimiter = ',' ;
char input_data[] = "1moisture=low:water=normal,2moisture=normal:water=normal,3moisture=normal:water=high";
//char delimiter = ",";
//parse_data(input_data, delimiter);

  char *ptr_input = input_data;
  char *tokenized_string;

  //Serial.begin(9600);
  while ((tokenized_string = strtok_r(ptr_input, ",", &ptr_input)) != NULL) // delimiter is the semicolon
  {
    Serial1.println(tokenized_string);
  }
} 

void setup1() {
  // You can open the Arduino IDE Serial Monitor window to see what the code is doing
  Serial.begin(115200);  // Serial connection from ESP-01 via 3.3v console cable  
  Serial1.begin(115200);
  
  dht.begin();           // initialize temperature sensor
 
  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.print("\n\r \n\r Working to connect 1");
  Serial.print("\n\r \n\rSerial1 connect");

 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("DHT Moisture Reading Server");
  Serial.print("Connected to ");
  Serial.println(ssid);//If you have errors which mention DHT then your Adafruit DHT library was not recognized (go back and follow the install steps again).  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
   
  server.on("/", handle_root);
  
  server.on("/moisture", [](){  // if you add this subdirectory to your webserver call, you get text below :)
    getmoisture();       // read sensor
    webString="Hi, its working";   // Arduino has a hard time with float to string
    server.send(200, "text/plain", webString);            // send to someones browser when asked
  });
   
  server.begin();
  Serial.println("HTTP server started");
}

 
void working_code() {
  char read_val[100];
  int i = 0;

  Serial.flush();
  while(Serial.peek() != -1)
  {
    read_val[i] = Serial.read();
    i++;
  }
  
  read_val[i] = '\0';
  
  //int value =   Serial.read();
  Serial1.print("\n\r \n\r value read: ");
  Serial1.print(read_val);
}

void parse_data(char* input_data, char delimiter) {
  char input_data[] = "1moisture=low:water=normal,2moisture=normal:water=normal,3moisture=normal:water=high";
  char delimiter = "," ;

  char *ptr_input = input_data;
  char *tokenized_string;

  //Serial.begin(9600);
  while ((tokenized_string = strtok_r(ptr_input, ";", &ptr_input)) != NULL) // delimiter is the semicolon
  {
    Serial1.println(tokenized_string);
  }
} 

*/

  
