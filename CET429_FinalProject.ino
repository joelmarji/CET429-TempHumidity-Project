/*Program written by Joel Marji and Calvin Jantz for CET 429 final project
Displays temperature and humidity measured by DHT11 temp sensor and displays
it on an LCD, as well as sending it to a basic webpage/Thingspeak when the arduino is 
connected to the internet*/

//LCD & Sensor initialization
#include <LiquidCrystal.h>
#include "dht.h"
#define dht_apin A0 // Analog Pin sensor is connected to
#define fan_pin 1
dht DHT;
int tim = 500; 
int inputDelay = 1000;
long int fanDelay = 5000;
long int wait = 0; 
long int fanWait = 0;

// Initialize the library with the numbers of the interface pins
LiquidCrystal lcd(4, 6, 10, 11, 12, 13);

//WiFi initialization 
#include <SPI.h>
#include "WiFiS3.h"
char ssid[] = "Hotspot_SSID";//Connect to WiFi 
char pass[] = "Hotspot_Password";
int keyIndex = 0;
int status = WL_IDLE_STATUS;
WiFiServer server(80);


//START
void setup()
{
  
  pinMode(fan_pin, OUTPUT);  

  //LCD Setup
  lcd.begin(16, 2);  // Set up the LCD's number of columns and rows: 

  //WiFi Setup
  while (status != WL_CONNECTED) { //Connect to network
    
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    lcd.setCursor(0,0);
    lcd.print("Connecting to");
    lcd.setCursor(0,1);
    lcd.print("WiFi...");
    delay(5000);
  }
  server.begin();
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connected!");
  lcd.setCursor(0,1);
  lcd.print("IP:");
  lcd.setCursor(3, 1);
  lcd.print(ip);
  delay(3000);
  lcd.clear();
}
void loop() 
{ 

  //LCD & Sensor

  DHT.read11(dht_apin);
  double temp = toFarenheit(DHT.temperature);
  double humidity = DHT.humidity;

  //LCD 
  displayTemp(temp);
  displayHumidity(humidity);

  //WiFi
  displayToWeb(temp, humidity);
  sendToThingSpeak(temp, humidity); 

  if((temp >= 79 ) & (fanWait < millis())) {
    digitalWrite(fan_pin, HIGH);
    fanWait = fanDelay + millis(); 
  }
  else if(fanWait < millis()){
    digitalWrite(fan_pin, LOW);
    fanWait = inputDelay + millis();
  }


  delay(2000);//delay to ensure temp sensor function

}

double toFarenheit(double tempF){//Convert C to F with DHT11 temp sensor
  tempF = ((DHT.temperature * 9/5) + 32);
  return tempF;
}

void displayTemp(double tempF){//Function to display temp on LCD. Makes sure spacing is correct when temps are 1 2 or 3 digits
  double tempC = DHT.temperature;
  lcd.setCursor(0,0);
  lcd.print("Temp:");
  lcd.setCursor(5,0);

  if (tempF<10){
  lcd.print((int)round(tempF)); 
  lcd.setCursor(6,0);
  lcd.print("F");
  lcd.setCursor(7,0);
  lcd.print("/");
  lcd.setCursor(8,0);
  lcd.print((int)round(tempC));//Display celcius alongside farenheit
  lcd.setCursor(10, 0);
  lcd.print("C");
  }
  
  else if (tempF>=10 && tempF<100){
  lcd.print((int)round(tempF)); 
  lcd.setCursor(7,0);
  lcd.print("F");
  lcd.setCursor(8,0);
  lcd.print("/");
  lcd.setCursor(9,0);
  lcd.print((int)round(tempC));
  lcd.setCursor(11, 0);
  lcd.print("C");
  }

  else if (tempF>=100){
  lcd.print((int)round(tempF)); 
  lcd.setCursor(8,0);
  lcd.print("F");
  lcd.setCursor(9,0);
  lcd.print("/");
  lcd.setCursor(10,0);
  lcd.print((int)round(tempC));
  lcd.setCursor(12, 0);
  lcd.print("C");
  }
  
}

void displayHumidity(double humidity){//Function to display humidity. Accounts for single digits to ensure spacing for '%' is correct
    lcd.setCursor(0,1);
    lcd.print("Humidity:");
    lcd.setCursor(9,1);

    if (round(humidity)<10){
      lcd.setCursor(9,1);
      lcd.print((int)round(humidity));
      lcd.setCursor(10,1);
      lcd.print("%");
      lcd.setCursor(11, 1);
      lcd.print(" ");
    }
    
    else if (round(humidity)>=10){
      lcd.setCursor(9,1);
      lcd.print((int)round(humidity));
      lcd.setCursor(11, 1);
      lcd.print("%");
    }
}

void displayToWeb(double tempF, double humidity) {//Function to display temp/humidity to a basic webpage using HTML. Type in device IP address in browser to view
  double tempC = DHT.temperature;
  WiFiClient client = server.available();   
  if (client) {                             
    Serial.println("New client");           
    String currentLine = "";                
    
    while (client.connected()) {            
      if (client.available()) {             
        char c = client.read();             
        Serial.write(c);                    
        
        if (c == '\n') {
          if (currentLine.length() == 0) {

            // Send HTTP response
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println(); // End of headers
            
            // HTML content
            client.println("<!DOCTYPE html>");
            client.println("<html>");
            client.println("<head><title>CET429 Project</title></head>");
            //client.println("<meta http-equiv=\"refresh\" content=\"5\">");//refresh page every 5 seconds
            client.println("<body>");
            client.println("<center>");
            client.println("<h1>CET 429 Project</h1>");
            client.println("<hr>");
            client.println("<p>This website displays the temperature and humidity gathered by the Arduino. Project by Joel Marji and Calvin Jantz</p>");
            
            client.print("<p><b>Temperature: ");
            client.print((int)round(tempF));
            client.print("F");
            client.print("/");
            client.print((int)round(tempC));
            client.println("C</b></p>");
            
            client.print("<p><b>Humidity: ");
            client.print((int)round(humidity));
            client.println("%</b></p>");
            
            client.println("<p>Amazon links to Materials used: </p>");
            client.println("<a href=\"https://a.co/d/6ughzle\">External devices & basic board |</a>");
            client.println("<a href=\"https://a.co/d/iTk8UqR\"> WiFi Capable Arduino Board</a>");
            client.println("</center>");
            client.println("</body>");
            client.println("</html>");
            
            break;
          } 
          else {
            currentLine = "";
          }
        } 
        else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}

void sendToThingSpeak(double tempF, double humidity) {//Send temp and humidity data to ThingSpeak for real time graphs and MATLAB integration. Link: https://thingspeak.mathworks.com/channels/2920816
  WiFiClient client;
  const char* server = "api.thingspeak.com";
  String apiKey = "XXXXXXXXXXXXXX"; //ThingSpeak Write API key

  if (client.connect(server, 80)) {
    String url = "/update?api_key=" + apiKey;
    url += "&field1=" + String(tempF);
    url += "&field2=" + String(humidity);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" + 
                 "Connection: close\r\n\r\n");

    Serial.println("Data sent to ThingSpeak");
  } 
  else {
    Serial.println("Connection to ThingSpeak failed");
  }

  client.stop();
}
