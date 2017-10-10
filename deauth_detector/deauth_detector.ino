#include <ESP8266WiFi.h>
#include "Mac.h"
#include "images.h"
#include "SSD1306.h"

extern "C" {
  #include "user_interface.h"
}

//===== SETTINGS =====//
#define channel 1 //the channel it should scan on (1-14)
#define channelHopping true //scan on all channels
#define maxChannel 13 //US = 11, EU = 13, Japan = 14
#define ledPin 2 //led pin ( 2 = built-in LED)
#define inverted true // invert HIGH/LOW for the LED
#define packetRate 3 //min. packets before it gets recognized as an attack

#define scanTime 500 //scan time per channel in ms


//Mac from;
//Mac to;
unsigned long c = 0;
unsigned long prevTime = 0;
unsigned long curTime = 0;
int curChannel = channel;

// OLED display
SSD1306  display(0x3c, D1, D2);

void sniffer(uint8_t *buf, uint16_t len) {
  //if(len>27){
    //from.set(buf[16],buf[17],buf[18],buf[19],buf[20],buf[21]);
    //to.set(buf[22],buf[23],buf[24],buf[25],buf[26],buf[27]);

    if(buf[12] == 0xA0 || buf[12] == 0xC0){
      /*Serial.print("From ");
      from._println();
      Serial.print("To ");
      to._println();
      Serial.println();*/
      
      c++;
    }
    
  //}
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting wireless initialization");

  //wifi_set_opmode(SOFTAP_MODE);
  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(0);
  WiFi.disconnect();
  wifi_set_promiscuous_rx_cb(sniffer);
  wifi_set_channel(curChannel);
  wifi_promiscuous_enable(1);
  
  Serial.println("LED Pin setup");
  pinMode(ledPin, OUTPUT);

  Serial.println("Initializing OLED display");
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  
  Serial.println("Initialized...");

}

void loop() {
  curTime = millis();
  
  if(curTime - prevTime >= scanTime){

    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "DeAuth attack detector");
    display.drawString(25, 52, "Scanning channel: " + String(curChannel));
    
    prevTime = curTime;
    Serial.println((String)c);

    display.setFont(ArialMT_Plain_24);
    if(c >= packetRate){
      // Attack detected
      if(inverted) digitalWrite(ledPin, LOW);
      else digitalWrite(ledPin, HIGH);
      display.drawString(35, 20, "DeAuth!");
      display.drawXbm(5, 20, Exclamation_width, Exclamation_height, Exclamation_bits);
    }else{
      // No attack
      if(inverted) digitalWrite(ledPin, HIGH);
      else digitalWrite(ledPin, LOW);
      display.drawString(35, 20, "All clear");
      display.drawXbm(5, 20, Check_width, Check_height, Check_bits);
    }
    
    c = 0;
    if(channelHopping){
      curChannel++;
      if(curChannel > maxChannel) curChannel = 1;
      wifi_set_channel(curChannel);
    }

    display.display();
  }

}
