#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
// OLED Library
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#include <RH_ASK.h>
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h> // Not actually used but needed to compile
#endif

RH_ASK Radio; // <---- HÄR BLIR DET FEL DEN KROCKAR MED Adafruit_SSD1306 display(-1);
//Adafruit_SSD1306 display(-1);
Adafruit_SSD1306 display(128, 32, &Wire, 4);

static DHT dht(7, DHT11);


void initDisplay() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(1000);
  display.clearDisplay();
}
int period = 10000;
unsigned long prevMillis = 0;
unsigned long currentMillis = millis();
                                        
void setup() {

  Serial.begin(115200);
  Serial.println("setup");
  dht.begin();
  
  initDisplay();

#ifdef RH_HAVE_SERIAL        
#endif
    if (!Radio.init())
#ifdef RH_HAVE_SERIAL
         Serial.println("init failed");
#else
  ;
#endif

}

 
void showTemp(float temp,float hud) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("T=");
  display.print(temp);
  display.println("C");
  display.print("H=");
  display.print(hud);
  display.println("%");
  display.display();
}

void receiver(){
  uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
  uint8_t buflen = sizeof(buf);
  
 
  if (Radio.recv(buf, &buflen)) {

    StaticJsonBuffer<255> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(buf);
    if(root["id"] == "F") {

      String text = "";
      
      for(int j = 0; j < buflen; j++){
        text += (char)buf[j]; 
      }


      Serial.println(text);
      Serial.println("Message Recieved");
  
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.print(text);
      display.display();
      
      Serial.println(text);
        
    }
  } 
}

void sendMessage(float temperature, float humidity) {
  char payload[255];

  StaticJsonBuffer<255> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["temperature"] = temperature;
  root["humidity"] = humidity;
  root.printTo(payload, 255);
  Serial.println(payload);
  Radio.send((uint8_t *)payload, 255);
  Radio.waitPacketSent();  
  Serial.println("meddelande skickat");
}


void loop() {
  currentMillis = millis();
  
  receiver();  //<--- här ska medelandet tas in.
    
  if((currentMillis - prevMillis) >= 30000){
    prevMillis = currentMillis;

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    sendMessage(temperature, humidity);// <--- Detta skickar meddelandet
    showTemp(temperature, humidity); // <--// 
  }
}
