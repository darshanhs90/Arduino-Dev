        /*****************************************************************
        Phant_CC3000.ino
        Post data to SparkFun's data stream server system (phant) using
        an Arduino and the CC3000 Shield.
        Jim Lindblom @ SparkFun Electronics
        Original Creation Date: July 3, 2014
        
        This sketch uses an Arduino Uno to POST sensor readings to 
        SparkFun's data logging streams (http://data.sparkfun.com). A post
        will be initiated whenever pin 3 is connected to ground.
        
        Before uploading this sketch, there are a number of global vars
        that need adjusting:
        1. WiFi Stuff: Fill in your SSID, WiFi Passkey, and encryption
           setting using three variables available.
        2. Phant Stuff: Fill in your data stream's public, private, and 
        data keys before uploading!
        
        This sketch requires that you install this library:
        * SFE_CC3000: https://github.com/sparkfun/SFE_CC3000_Library
        
        Hardware Hookup:
          * These components are connected to the Arduino's I/O pins:
            * D3 - Active-low momentary button (pulled high internally)
            * A0 - Photoresistor (which is combined with a 10k resistor
                   to form a voltage divider output to the Arduino).
            * D5 - SPST switch to select either 5V or 0V to this pin.
          * A CC3000 Shield sitting comfortable on top of your Arduino.
        
        Development environment specifics:
            IDE: Arduino 1.0.5
            Hardware Platform: RedBoard & CC3000 Shield (v10)
        
        This code is beerware; if you see me (or any other SparkFun 
        employee) at the local, and you've found our code helpful, please 
        buy us a round!
        
        Much of this code is largely based on Shawn Hymel's WebClient
        example in the SFE_CC3000 library.
        
        Distributed as-is; no warranty is given.
        *****************************************************************/
        // SPI and the pair of SFE_CC3000 include statements are required
        // for using the CC300 shield as a client device.
        #include <SPI.h>
        #include <SFE_CC3000.h>
        #include <SFE_CC3000_Client.h>
        // Progmem allows us to store big strings in flash using F().
        // We'll sacrifice some flash for extra DRAM space.
        #include <Progmem.h>
        #include <Servo.h> 
         
        
        ////////////////////////////////////
        // CC3000 Shield Pins & Variables //
        ////////////////////////////////////
        // Don't change these unless you're using a breakout board.
        #define CC3000_INT      2   // Needs to be an interrupt pin (D2/D3)
        #define CC3000_EN       7   // Can be any digital pin
        #define CC3000_CS       10  // Preferred is pin 10 on Uno
        #define IP_ADDR_LEN     4   // Length of IP address in bytes
        
        ////////////////////
        // WiFi Constants //
        ////////////////////
        char ap_ssid[] = "DEEPTHA";                // SSID of network
        char ap_password[] = "Sonu@0905";        // Password of network
        unsigned int ap_security = WLAN_SEC_WPA2; // Security of network
        // ap_security can be any of: WLAN_SEC_UNSEC, WLAN_SEC_WEP, 
        //  WLAN_SEC_WPA, or WLAN_SEC_WPA2
        unsigned int timeout = 30000;             // Milliseconds
        char server[] = "data.sparkfun.com";      // Remote host site
        char str [201];
        char buffer[5];
        boolean checker=true;
        Servo myservo;  // create servo object to control a servo 
                        // a maximum of eight servo objects can be created 
         
        int pos = 0;    // variable to store the servo position 
         int RF=0;
        // Initialize the CC3000 objects (shield and client):
        SFE_CC3000 wifi = SFE_CC3000(CC3000_INT, CC3000_EN, CC3000_CS);
        SFE_CC3000_Client client = SFE_CC3000_Client(wifi);
        
        /////////////////
        // Phant Stuff //
        /////////////////
        const String publicKey = "jqE4oYRz1ACyKKgApvdp";
        const String privateKey = "zzWPKgX0wYI6jjaPodwo";
        const byte NUM_FIELDS = 6;
        const String fieldNames[NUM_FIELDS] = {"pressure1", "pressure2", "pressure3","pressure4","pressure5","pressure6"};
        String fieldData[NUM_FIELDS];
        
        //convert an unsigned int to a four character string
        char* getInt(unsigned int num)
        {
            memset(buffer,'\0',4);   //clear the buffer
            sprintf (buffer, "%04i", num); //Change for your data length 
            return buffer;                 //return buffer
        }
        
        
        void setup()
        {
          Serial.begin(115200);
           myservo.attach(10);
          // Set Up WiFi:
          setupWiFi();
        
          Serial.println(F("=========== Ready to Stream ==========="));
        }
        
        void loop()
        {
        
            // Gather data:
            fieldData[0] = getInt(readSensor(analogRead(A0)));
            fieldData[1] = getInt(readSensor(analogRead(A1)));
            fieldData[2] = getInt(readSensor(analogRead(A2)));
            fieldData[3] = getInt(readSensor(analogRead(A3)));
            fieldData[4] = getInt(readSensor(analogRead(A4)));
            fieldData[5] = getInt(readSensor(analogRead(A5)));
               if(fieldData[0]=="1" and fieldData[1]=="1" and fieldData[2]=="1" and fieldData[3]=="1" and checker==true){
                checker=false;
                 RF= RfidGen();
                 if(RF==345){
                   Serial.println("kulla");
                   myservo.writeMicroseconds(700);
                 }
                 else if(RF==667){
                    Serial.println("medium");
                    myservo.writeMicroseconds(1500); 
                   }
              else{
                   Serial.println("udda");
                   myservo.writeMicroseconds(2000);
                   }
                   myservo.detach();
                 }
            if(fieldData[0]=="0" and fieldData[1]=="0" and fieldData[2]=="0" and fieldData[3]=="0"){
                checker=true;
                myservo.attach(10);
            }
            // Post data:
            Serial.println("Posting!");
            postData(); // the postData() function does all the work, 
                        // check it out below.
            delay(1000);
         
        }
        
        int RfidGen(){
      int  randNumber = random(0,3);
      Serial.println(randNumber);
      int RFID[]={345,667,223};// 345=>kulla, 667=>medium, 223=>udda
      return RFID[randNumber];
     }
        
        
        
        
        void postData()
        {
        
          // Make a TCP connection to remote host
          if ( !client.connect(server, 80) )
          {
            // Error: 4 - Could not make a TCP connection
            Serial.println(F("Error: 4"));
          }
        
          // Post the data! Request should look a little something like:
          // GET /input/publicKey?private_key=privateKey&light=1024&switch=0&time=5201 HTTP/1.1\n
          // Host: data.sparkfun.com\n
          // Connection: close\n
          // \n
          client.print("GET /input/");
          client.print(publicKey);
          client.print("?private_key=");
          client.print(privateKey);
          for (int i=0; i<NUM_FIELDS; i++)
          {
            client.print("&");
            client.print(fieldNames[i]);
            client.print("=");
            client.print(fieldData[i]);
          }
          client.println(" HTTP/1.1");
          client.print("Host: ");
          client.println(server);
          client.println("Connection: close");
          client.println();
        
          while (client.connected())
          {
            if ( client.available() )
            {
              char c = client.read();
              Serial.print(c);
            }      
          }
          Serial.println();
        }
        
        void setupWiFi()
        {
          ConnectionInfo connection_info;
          int i;
        
          // Initialize CC3000 (configure SPI communications)
          if ( wifi.init() )
          {
            Serial.println(F("CC3000 Ready!"));
          }
          else
          {
            // Error: 0 - Something went wrong during CC3000 init!
            Serial.println(F("Error: 0"));
          }
        
          // Connect using DHCP
          Serial.print(F("Connecting to: "));
          Serial.println(ap_ssid);
          if(!wifi.connect(ap_ssid, ap_security, ap_password, timeout))
          {
            // Error: 1 - Could not connect to AP
            Serial.println(F("Error: 1"));
          }
        
          // Gather connection details and print IP address
          if ( !wifi.getConnectionInfo(connection_info) ) 
          {
            // Error: 2 - Could not obtain connection details
            Serial.println(F("Error: 2"));
          }
          else
          {
            Serial.print(F("My IP: "));
            for (i = 0; i < IP_ADDR_LEN; i++)
            {
              Serial.print(connection_info.ip_address[i]);
              if ( i < IP_ADDR_LEN - 1 )
              {
                Serial.print(".");
              }
            }
            Serial.println();
          }
        }
        
        /*********************** Sensor Code ***************************/
        // convert raw light data from sensor to percentage
        // to do this, we multiply the number by 1.1/1024, and
        //take the percentage value out of 0.99 volts. 
        int readSensor(unsigned int lt){     
          float tmpflt = (float)lt*1.0;
          int percent;
          Serial.println(lt);
            if(tmpflt > 60.0)
            {  
              return 1;
            }
            else{
              return 0;
            }
        }
