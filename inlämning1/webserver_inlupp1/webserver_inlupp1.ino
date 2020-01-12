#include <ESP8266WebServer.h>
#include <SPI.h>
#include <RH_ASK.h>
#include <ArduinoJson.h>


ESP8266WebServer webserver(80);
RH_ASK driver(2000, 13, 16, 12);


char *WIFI_SSID = "IoT";
char *WIFI_PASS = "IoT2018!";

char *user_email = "user@domain.com";
char *user_password = "password";
bool emailCheck = false;
bool passwordCheck = false;


String receiver(){
  uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
  uint8_t buflen = sizeof(buf);
  String rcv;

  // Det som är nedanför här måste ändras så att den faktiskt skriver ut datat som kommer.
  // Titta på exempelkod på studentportalen samt exempelbiblioteken som finns i arduino.
 
  if (driver.recv(buf, &buflen)) {
    for (int i = 0; i<buflen; i++){
      rcv += (char)buf[i];
    }     
    driver.printBuffer("Got:", buf, buflen);
  }
  
  Serial.println(rcv);
  return rcv;
}


void initSerial(int baud) {
  Serial.begin(baud);
  delay(2000);
  Serial.println("Serial communication initiated.");
  
}

void initWifi() {
  Serial.println("Wifi communication initiated. Please wait...");

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while(WiFi.status() != WL_CONNECTED) { 
    Serial.print(".");
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void loginPage() {
  webserver.send(200, "text/html", "<!DOCTYPE html><html><head> <meta charset='utf-8'> <meta name='viewport' content='width=device-width, initial-scale=1, shrink-to-fit=no'> <title>Login - Webserver</title> <link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/4.2.1/css/bootstrap.min.css'> <link rel='stylesheet' href='https://www.epndata.com/iot18/demo/css/style.css'></head><body class='text-center'> <form action='/validatelogin' method='post' class='form-signin'> <h1 class='h3 mb-3 font-weight-normal'>Please sign in</h1> <label for='inputEmail' class='sr-only'>Email address</label> <input type='email' name='inputEmail' id='inputEmail' class='form-control' placeholder='Email address' required autofocus> <label for='inputPassword' class='sr-only'>Password</label> <input type='password' name='inputPassword' id='inputPassword' class='form-control' placeholder='Password' required> <button class='btn btn-lg btn-primary btn-block' type='submit'>Sign in</button> <button formaction='/weather' class='btn btn-lg btn-primary btn-block' type='sensor'>Weather Sensor</button> <p class='mt-5 mb-3 text-muted'>&copy; 2019 IoT18 InlämningsUppgift1.</p></form></body></html>"); 
}

void weatherPage() {
  
  // Här får du in både temperatur och luftfuktighet i en sträng. Du måste göra en split eller nått
  // baserat på , -tecknet så meddelandet som du skickar ska se ut "TEMP,HUM" dvs "23,43"
  
  String dhtData = receiver();
  String arrayreciver[255];
 

  for(int i = 0; i < 255; i++){
    arrayreciver[i] = String(dhtData);
    
  }

  String temperature = arrayreciver[0];
  String humidity = arrayreciver[1];
  
  
   
  webserver.send(200, "text/html", "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\"><title>Document</title></head><body> <a href=\"/text\">Gå till textsidan</a><h1>Väderdata</h1><p>Temperature: " + temperature + "</p><p>Luftfuktighet: " + humidity + "</p></body></html>"); 
}

void textPage() {
  webserver.send(200, "text/html", "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\"><title>Text Meddelande</title></head><body><a href=\"/weather\">Gå till vädersidan</a><h1>Skicka Meddelande</h1><form action=\"/sendtext\" method=\"POST\"> <input type=\"text\" name=\"message\" id=\"message\" placeholder=\"skriv ett meddelande\"> <input type=\"submit\" value=\"Skicka\" name=\"submit\" id=\"submit\"></form></body></html>");  
}

void sendText() {
  if(webserver.args() > 0) {
    for(uint8_t i = 0; i < webserver.args(); i++) {      
      if(webserver.argName(i) == "message") {
        
          Serial.println("Meddelandet är: " + webserver.arg(i));
          String message = webserver.arg(i);


      char msg[32];
      StaticJsonBuffer<255> jsonBuffer;
      JsonObject &root = jsonBuffer.createObject();
      root["id"] = "F";
      root["message"] = message;
      root.printTo(msg);
      Serial.println(msg);
      driver.send((uint8_t *)msg, 32);
      driver.waitPacketSent();
      Serial.println();
      Serial.println("Medelandet är skickat!");
      

          //char msg[32];
          //message.toCharArray(msg, sizeof(msg)); 
          
          // HÄR SKA RF-SKICKA DELEN IN
          //driver.send((uint8_t *)msg, strlen(msg)); 
          //Serial.println("msg är: " + String(msg));
          //driver.waitPacketSent();//<-- fel
          //Serial.println("msg är: " + String(msg));
          //delay(200);

          //Serial.println("Medelandet är skickat!");
      }
    }
  }
  webserver.sendHeader("Location", "/text");
  webserver.send(303);
}

void validateLogin() {
  if(webserver.args() > 0) {
    for(uint8_t i = 0; i < webserver.args(); i++) {
      
      if(webserver.argName(i) == "inputEmail") {
        if(webserver.arg(i) == user_email) {
          Serial.println("E-postadressen stämmer");
          emailCheck = true;
        }
      }
      
      if(webserver.argName(i) == "inputPassword") {
        if(webserver.arg(i) == user_password) {
          Serial.println("Lösenordet stämmer");
          passwordCheck = true;
        }
      }
    }
  }
  
  if(emailCheck && passwordCheck) {
    webserver.sendHeader("Location", "/text");
    webserver.send(303);
  } else {
    webserver.sendHeader("Location", "/");
    webserver.send(303);   
  } 
}



void initWebServer() {
  
  webserver.on("/", loginPage);
  webserver.on("/weather", weatherPage);
  webserver.on("/text", textPage);
  webserver.on("/sendtext", HTTP_POST, sendText);
  webserver.on("/validatelogin", HTTP_POST, validateLogin);
  webserver.begin();
  
  Serial.println("Webserver initiated and started.");
}









void setup() {
  initSerial(115200);           
  initWifi();                   
  initWebServer(); 
  if (!driver.init())
         Serial.println("init failed");

}

void loop() {
  webserver.handleClient();
}
