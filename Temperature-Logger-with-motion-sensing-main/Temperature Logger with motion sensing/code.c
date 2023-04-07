#include <DHT.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>

#define TRIGGER_PIN D3
#define ECHO_PIN D4
#define PIR_PIN D5

String apiKey = "H38TEGNC0XKW43BB";

const char *ssid =  "how2electronics";
const char *pass =  "alhabibi";
const char* server = "api.thingspeak.com";

#define DHTPIN 0

DHT dht(DHTPIN, DHT11);

WiFiClient client;

// Initialize the LCD with I2C interface
LiquidCrystal_I2C lcd(0x3F, 16, 2);

void setup() 
{
  Serial.begin(115200);
  delay(10);
  dht.begin();

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);

  lcd.init();                      // Initialize the LCD
  lcd.backlight();                 // Turn on the backlight
  lcd.print("Initializing...");

  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");

  lcd.clear();                     // Clear the LCD
  lcd.print("WiFi connected");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());       // Display the local IP address on the LCD
}

void loop() 
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int pirStatus = digitalRead(PIR_PIN);
  
  if (isnan(h) || isnan(t)) 
  {
    Serial.println("Failed to read from DHT sensor!");
    lcd.clear();
    lcd.print("Sensor error");
    return;
  }

  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  float distance = pulseIn(ECHO_PIN, HIGH) / 58.0;

  if (client.connect(server,80))  
  {  
    String postStr = apiKey;
    postStr +="&field1=";
    postStr += String(t);
    postStr +="&field2=";
    postStr += String(h);
    postStr +="&field3=";
    postStr += String(distance);
    postStr +="&field4=";
    postStr += String(pirStatus);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" degrees Celcius, Humidity: ");
    Serial.print(h);
    Serial.print("%, Distance: ");
    Serial.print(distance);
    Serial.print("cm, PIR: ");
    Serial.println(pirStatus);
    Serial.println("Send to ThingSpeak.");

    lcd.clear();
    lcd.print("Temp: ");
    lcd.print(t);
    lcd.print((char)223);           // Display degree symbol
    lcd.print("C");
    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    lcd.print(h);
    lcd.setCursor(0,2);
    lcd.print("%,Distance is");
    lcd.print(distance);
    lcd.setCursor(1,1);
    lcd.print("cm,PIR: ");
    lcd.print(pirStatus);
  }

  client.stop();

  Serial.println("Waiting...");

  delay(15000);
}