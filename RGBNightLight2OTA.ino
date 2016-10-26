 #include <Dns.h>
#include <EthernetClient.h>
#include <Ethernet.h>
#include <EthernetServer.h>
#include <Dhcp.h>
#include <EthernetUdp.h>

#include <Adafruit_DotStar.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

#include <ArduinoOTA.h>
#include <SimpleTimer.h>

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
 
#define NUMPIXELS 10
 
#define DATAPIN    5 // GPIO15 - MISO
#define CLOCKPIN  4 // GPIO14 - CLK


//how many clients should be able to telnet to this ESP8266
#define MAX_SRV_CLIENTS 1



// -------- Telnet debug comments
WiFiServer server(23);
WiFiClient serverClients[MAX_SRV_CLIENTS];
// ------------------------------


const boolean invert = false; // set true if common anode, false if common cathode
int R, G, B;  // the Red Green and Blue colour components
int colour = 0x000000;
byte colourbyte = 0;
int brightness = 255; //start at full brightnes



int i = 0;
byte count = 0;
byte numpix=1;

Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN);
char auth[] = "4915df8376924887a8aa7801889462d9";

//byte Refreshtime = 100; //delay time between transitions
int Refreshtime = 100; //delay time between transitions

SimpleTimer timer;


void updatepixels()
{
  
  hueToRGB( colourbyte, brightness);  // call function to convert hue to RGB

colour <<= 8; //shift 8 bits to the left to clear out previous value
colour |= G;
colour <<=8;
colour |= B;



  strip.setPixelColor(i, colour);
colour <<=8;
colour |= R;

strip.show();

count++;
if(count>numpix) {
    count = 0;
colourbyte++;
}

if (colourbyte > 255) 
    {
    colourbyte = 0;
    brightness--;
    }
i++;
if(i>NUMPIXELS) i = 0;

timer.setTimeout(Refreshtime, updatepixels);
  }


 
// function to convert a colour to its Red, Green, and Blue components.
void hueToRGB( int hue, int brightness)
{
    unsigned int scaledHue = (hue * 6);
    unsigned int segment = scaledHue / 256; // segment 0 to 5 around the
                                            // colour wheel
    unsigned int segmentOffset =
      scaledHue - (segment * 256); // position within the segment

    unsigned int complement = 0;
    unsigned int prev = (brightness * ( 255 -  segmentOffset)) / 256;
    unsigned int next = (brightness *  segmentOffset) / 256;

    if(invert)
    {
      brightness = 255-brightness;
      complement = 255;
      prev = 255-prev;
      next = 255-next;
    }

    switch(segment ) {
    case 0:      // red
        R = brightness;
        G = next;
        B = complement;
    break;
    case 1:     // yellow
        R = prev;
        G = brightness;
        B = complement;
    break;
    case 2:     // green
        R = complement;
        G = brightness;
        B = next;
    break;
    case 3:    // cyan
        R = complement;
        G = prev;
        B = brightness;
    break;
    case 4:    // blue
        R = next;
        G = complement;
        B = brightness;
    break;
   case 5:      // magenta
    default:
        R = brightness;
        G = complement;
        B = prev;
    break;
    }
}


void setup() {
    Serial.begin(115200);


WiFiManager wifiManager;
wifiManager.setConnectTimeout(30);
wifiManager.autoConnect();
   
  Serial.println("Booting");
  Serial.println("Firmware 0.38 25/10/2016");
WiFi.mode(WIFI_STA);
//  Blynk.begin(auth, "HoffHouse", "unevenbutter269");
//    Blynk.begin(auth);
  Blynk.config(auth); //test wifi
  strip.begin();
  strip.show();

  server.begin();

//ArduinoOTA.setPassword((const char *)"123");
  
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
    ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  timer.setTimeout(1200, updatepixels);
  
  //headlamp
  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);   // sets the LED off
}
BLYNK_WRITE(1) { //in blynk app, large RED slider set to virtual pin 1 with minimum = 0 and maximum = 255
     brightness = param.asInt();
}
BLYNK_WRITE(2) { //in blynk app, large RED slider set to virtual pin 1 with minimum = 0 and maximum = 255
     Refreshtime = param.asInt();
}
BLYNK_WRITE(3) { //in blynk app, large RED slider set to virtual pin 1 with minimum = 0 and maximum = 255
     numpix = param.asInt();
}
BLYNK_WRITE(4) { //in blynk app, on / off for lamp
    if (param.asInt() == 0)
     {
      serverClients[0].println("OFF");
     brightness = param.asInt();
     Refreshtime = 100;
     digitalWrite(14, LOW);   // sets the LED off
     }
     else
     {
      serverClients[0].println("ON");
      digitalWrite(14, HIGH);   // sets the LED on
      brightness = 180;
      Refreshtime = 100;
     }
}


void loop() {
 // strip.setPixelcolor(head, colour);
  //strip.setPixelcolor(tail, 0);
    ArduinoOTA.handle();
  Blynk.run();
  timer.run();
//for (int i = 0; i < NUMPIXELS; i++)
//    {



// ---------- Trial on telnet debug messages
 if (server.hasClient()){
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()){
        if(serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        Serial1.print("New client: "); Serial1.print(i);
        continue;
       serverClients[0].println("Firmware 0.381 25/10/2016");
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
// -----------------

    }

 

 

