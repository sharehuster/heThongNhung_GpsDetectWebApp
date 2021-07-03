#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <SoftwareSerial.h>
#include <SocketIoClient.h>
#include <TinyGPS.h>
TinyGPS gps;
#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;
SocketIoClient webSocket;

void event(const char * payload, size_t length) {
  payload = "abc";
  USE_SERIAL.printf("got message: %s\n", payload);
}


const byte RX = D1;   
const byte TX = D2;
SoftwareSerial gpsSerial(RX, TX); 

struct data_GPS{
  float LON;
  float LAT;
};

// hàm đọc gps
 struct data_GPS readDataGPS() {
  float flon; float flat;
  data_GPS data_LON_LAT;
  bool newData = false;
  
  flon = 105.84311241877799;
  flat = 21.00701423659785;       // ĐHBK HN

  
  // Mỗi 1s sẽ đọc GPS 
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (gpsSerial.available())
    {
      char c = gpsSerial.read();
       Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData)
  {
    //float flat, flon; //flat = LAT ; flon = LON;
    gps.f_get_position(&flat, &flon);

    data_LON_LAT.LON = flon;
    data_LON_LAT.LAT = flat;
 return data_LON_LAT;
  }
}




void setup() {

  
    USE_SERIAL.begin(9600);
    gpsSerial.begin(9600);
    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

      for(uint8_t t = 4; t > 0; t--) {
          USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
          USE_SERIAL.flush();
          delay(1000);
      }

    WiFiMulti.addAP("Happy House 2", "vuilenbannhe");

    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    webSocket.on("hi", event);
    webSocket.begin("192.168.2.112",4000);
    // use HTTP Basic Authorization this is optional remove if not needed
   // webSocket.setAuthorization("username", "password");
}



void loop() {
   webSocket.loop();
  data_GPS dataGPS;  
  dataGPS = readDataGPS();
  float LO = dataGPS.LON;
  float LA = dataGPS.LAT;

  
  Serial.print("LONG=");Serial.println(LO,6);
  Serial.print("LAT=");Serial.println(LA,6);
  
 //  {\"data\":{\"geometry\":{\"type\":\"Point\",\"coordinates\":[105.84311241,20.0070142]},\"type\":\"Feature\",\"properties\":{\"title\":\"Xe Bus 16\"}}}
  
  String str2 = String(LO,6);
  String str3 = String(LA,6);
  
  // This will send a string to the server
  Serial.println("sending data to server");
  String str1 = "\"data\":{\"geometry\":{\"type\":\"Point\",\"coordinates\":["         ;
  String str4 = "]},\"type\":\"Feature\",\"properties\":{\"title\":\"Xe Bus 16\"}}"     ;
  String str = "{"+str1 + str2+","+ str3 + str4+"}" ;
//  char* pl= strToChar(str);


// Length (with one extra character for the null terminator)
int str_len = str.length() + 1; 
Serial.println("data send:");
Serial.println(str);
// Prepare the character array (the buffer) 
char char_array[str_len];
str.toCharArray(char_array, str_len);
webSocket.emit("sendData", char_array);
delay(2000);
}
