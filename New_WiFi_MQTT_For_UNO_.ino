#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <dht11.h>     //Library of the Humidity Sensor
#include <PubSubClient.h>


// Update these with values for your network.
byte mac[] = { 0xAE, 0xBD, 0xBA, 0xCE, 0xFE, 0xAD }; //Change for your unique MAC address
byte server[] = { 54, 191, 2, 249 };  //C2M MQTT Server
char ssid[] = "sreeshs"; // Replace for The Wireless network SSID (name) 
char pass[] = "123456789"; // Replace for The network password
char topic[] = "Arduino_UNO";  //Replace for the Topic of your specific project!
int status = WL_IDLE_STATUS; // the Wifi radio's status

char apikey[]="gEwsNreo/a@UCbxfaZX1rlVJMSz1dJHtz/STqutN9vE="; //Replace for your own API
char feedid[]="/xJRepEH3yqn1YUmZQecFOOnCsHuUWThbU1GsmTRsEw="; //Replace for your own FeedID

char str [201];
char buffer[5];

void callback(char* topic, byte* payload, unsigned int length) 
{
  // handle message arrived\
  int i = 0;
}

WiFiClient wfClient;
PubSubClient client(server, 5001, callback, wfClient); //check port 5001

// set pin numbers:
#define DHT11PIN 2 //DHT11 connect to PIN 7
dht11 DHT11;

//convert an unsigned int to a four character string
char* getInt(unsigned int num)
{
    memset(buffer,'\0',4);   //clear the buffer
    sprintf (buffer, "%04i", num); //Change for your data length 
    return buffer;                 //return buffer
}

void setup()
{
    Serial.begin(9600);
    if (WiFi.status() == WL_NO_SHIELD)  // check for the presence of the shield:
    {
        Serial.println("WiFi shield not present");
        while(true);// don't continue:
     }
      while ( status != WL_CONNECTED) 
      {
          Serial.print("Attempting to connect to WPA SSID: ");
          Serial.println(ssid);
          status = WiFi.begin(ssid, pass);
          delay(10000);
       }
      Serial.println("You've connected to the network");
      printWifiData();
      printCurrentNet();
}

void loop()
{

  Serial.println("Connecting to C2M_MQTT Server...");
  while(!client.connected())
  {
    client.connect(topic);
    delay(5000);
  }
   memset(str,'\0',200);
   strcat(str,"apikey:");
   strcat(str,apikey);
   strcat(str,",feedid:");
   strcat(str,feedid);
   strcat(str,",feed=pressure1,");
   strcat(str,getInt(readSensor(analogRead(A0))));
   strcat(str,"|pressure2,");
   strcat(str,getInt(readSensor(analogRead(A1))));
    strcat(str,"|pressure3,");
   strcat(str,getInt(readSensor(analogRead(A2))));
    strcat(str,"|pressure4,");
   strcat(str,getInt(readSensor(analogRead(A3))));
    strcat(str,"|pressure5,");
   strcat(str,getInt(readSensor(analogRead(A4))));
    strcat(str,"|pressure6,");
   strcat(str,getInt(readSensor(analogRead(A5))));   
  
    
      Serial.println(str);  //Check the Message 
       
    if(client.publish(topic,str)) 
    {
        Serial.println("Publish Success.");
    }
    else 
    {
         Serial.println("Publish Fail."); 
    }
    delay(300);
    client.loop();
}

/***************** WiFi Status *******************/
void printWifiData() 
{
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC address: ");
    Serial.print(mac[5],HEX);
    Serial.print(":");
    Serial.print(mac[4],HEX);
    Serial.print(":");
    Serial.print(mac[3],HEX);
    Serial.print(":");
    Serial.print(mac[2],HEX);
    Serial.print(":");
    Serial.print(mac[1],HEX);
    Serial.print(":");
    Serial.println(mac[0],HEX);
}

void printCurrentNet() 
{
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.println(rssi);
    byte encryption = WiFi.encryptionType();
    Serial.print("Encryption Type:");
    Serial.println(encryption,HEX);
}


/*********************** Sensor Code ***************************/
// convert raw light data from sensor to percentage
// to do this, we multiply the number by 1.1/1024, and
//take the percentage value out of 0.99 volts. 
int readSensor(unsigned int lt){     

  float tmpflt = (float)lt*1.1/1024;
  int percent;
  
    if(tmpflt > 1.10)  //error handling
      return 0;
    if(tmpflt < 0.00)
      return 0;  
  
    percent = 100*(0.99-tmpflt);
   
    if(percent > 100)
       percent = 100;
    if(percent < 0 )
       percent = 0;  
       
  return percent;
}

/************** DHT11 Humidity Sensor Function ****************/
int Sensor_DHT11(int selection)  // Select 1: show temperature, other number will calculate humidity.
{
    int temp,humi,fahrenheit; 
    DHT11.read(DHT11PIN);
    humi = DHT11.humidity;       
    temp = DHT11.temperature;
    fahrenheit = temp * 9/5 +32; 
    Serial.print(" Humidity: ");        
    Serial.print(humi);
    Serial.print("% ");
    Serial.print(" ( Temperature: ");
    Serial.print(temp);
    Serial.print(" C, "); 
    Serial.print(fahrenheit);
    Serial.println(" F )");
    if(selection == 1)
       return fahrenheit;
    else return humi;
        
}

