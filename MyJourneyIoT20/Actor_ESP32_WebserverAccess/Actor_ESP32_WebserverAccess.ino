/*
IOT Actor Board Webserver Demo with configuration webpage
for ESP32 DevKitC
Elektor / Jens Nickel
My Journey in the IoT
*/


#include <WiFi.h>

#include <Preferences.h>   // this library is used to get access to Non-volatile storage (NVS) of ESP32
// see https://github.com/espressif/arduino-esp32/blob/master/libraries/Preferences/examples/StartCounter/StartCounter.ino



// +++++++++++++++++++ Start of Webserver Library +++++++++++++++++++

// as ESP32 Arduino Version

WiFiClient myclient;
WiFiServer server(80);


void Webserver_Start()
{
  server.begin();     // Start TCP/IP-Server on ESP32
}



String WebRequestHostAddress;     // global variable used to store Server IP-Address of HTTP-Request


//  Call this function regularly to look for client requests
//  template see https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/SimpleWiFiServer/SimpleWiFiServer.ino
//  returns empty string if no request from any client
//  returns GET Parameter: everything after the "/?" if ADDRESS/?xxxx was entered by the user in the webbrowser
//  returns "-" if ADDRESS but no GET Parameter was entered by the user in the webbrowser
//  remark: client connection stays open after return
String Webserver_GetRequestGETParameter()
{
  String GETParameter = "";
  
  myclient = server.available();   // listen for incoming clients

  //Serial.print(".");
  
  if (myclient) {                            // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                 // make a String to hold incoming data from the client
    
    while (myclient.connected()) {           // loop while the client's connected
      
      if (myclient.available()) {            // if there's bytes to read from the client,
        
        char c = myclient.read();            // read a byte, then
        Serial.write(c);                     // print it out the serial monitor

        if (c == '\n') {                     // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request
          if (currentLine.length() == 0) {
            
            if (GETParameter == "") {GETParameter = "-";};    // if no "GET /?" was found so far in the request bytes, return "-"
            
            // break out of the while loop:
            break;
        
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
          
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        if (c=='\r' && currentLine.startsWith("GET /?")) 
        // we see a "GET /?" in the HTTP data of the client request
        // user entered ADDRESS/?xxxx in webbrowser, xxxx = GET Parameter
        {
          
          GETParameter = currentLine.substring(currentLine.indexOf('?') + 1, currentLine.indexOf(' ', 6));    // extract everything behind the ? and before a space
                    
        }


        if (c=='\r' && currentLine.startsWith("Host:")) 
        // we see a "Host:" in the HTTP data of the client request
        // user entered ADDRESS or ADDRESS/... in webbrowser, ADDRESS = Server IP-Address of HTTP-Request
        {
          int IndexOfBlank = currentLine.indexOf(' ');
          WebRequestHostAddress = currentLine.substring(IndexOfBlank + 1, currentLine.length());    // extract everything behind the space character and store Server IP-Address of HTTP-Request
          
        }
        
      }
      
    }
    
  }

  return GETParameter;
}



// Send HTML page to client, as HTTP response
// client connection must be open (call Webserver_GetRequestGETParameter() first)
void Webserver_SendHTMLPage(String HTMLPage)
{
   String httpResponse = "";

   // begin with HTTP response header
   httpResponse += "HTTP/1.1 200 OK\r\n";
   httpResponse += "Content-type:text/html\r\n\r\n";

   // then the HTML page
   httpResponse += HTMLPage;

   // The HTTP response ends with a blank line:
   httpResponse += "\r\n";
    
   // send it out to TCP/IP client = webbrowser 
   myclient.println(httpResponse);

   // close the connection
   myclient.stop();
    
   Serial.println("Client Disconnected.");
   
};



// +++++++++++++++++++ End of Webserver library +++++++++++++++++++++




// +++++++++++++++++++ Start of WiFi Library ++++++++++++++++++++++++


// Connect to router network and return 1 (success) or -1 (no success)
int WiFi_RouterNetworkConnect(char* txtSSID, char* txtPassword)
{
  int success = 1;
  
  // connect to WiFi network
  // see https://www.arduino.cc/en/Reference/WiFiBegin
  
  WiFi.begin(txtSSID, txtPassword);
  
  // we wait until connection is established
  // or 10 seconds are gone
  
  int WiFiConnectTimeOut = 0;
  while ((WiFi.status() != WL_CONNECTED) && (WiFiConnectTimeOut < 10))
  {
    delay(1000);
    WiFiConnectTimeOut++;
  }

  // not connected
  if (WiFi.status() != WL_CONNECTED)
  {
    success = -1;
  }

  // print out local address of ESP32 in Router network (LAN)
  Serial.println(WiFi.localIP());

  return success;
}



// Disconnect from router network and return 1 (success) or -1 (no success)
int WiFi_RouterNetworkDisconnect()
{
  int success = -1;
  
  WiFi.disconnect();
  

  int WiFiConnectTimeOut = 0;
  while ((WiFi.status() == WL_CONNECTED) && (WiFiConnectTimeOut < 10))
  {
    delay(1000);
    WiFiConnectTimeOut++;
  }

  // not connected
  if (WiFi.status() != WL_CONNECTED)
  {
    success = 1;
  }
  
  Serial.println("Disconnected.");

  return success;
}


// Initialize Soft Access Point with ESP32
// ESP32 establishes its own WiFi network, one can choose the SSID
int WiFi_AccessPointStart(char* AccessPointNetworkSSID)
{ 
  WiFi.softAP(AccessPointNetworkSSID);

  // printout the ESP32 IP-Address in own network, per default it is "192.168.4.1".
  Serial.println(WiFi.softAPIP());

  return 1;
}


// Put ESP32 in both modes in parallel: Soft Access Point and station in router network (LAN)
void WiFi_SetBothModesNetworkStationAndAccessPoint()
{
  WiFi.mode(WIFI_AP_STA);
}


// Get IP-Address of ESP32 in Router network (LAN), in String-format
String WiFi_GetOwnIPAddressInRouterNetwork()
{
  return WiFi.localIP().toString();
}



// +++++++++++++++++++ End of WiFi Library +++++++++++++++++++





Preferences preferences;   // we must generate this object of the preference library

#define PIN_LED_BOARD  12
#define PIN_ACTOR      12   // the digital output we want to control
#define PIN_LED_GREEN  27
#define PIN_LED_RED    26

#define LED_YELLOW  3
#define LED_GREEN   2
#define LED_RED     1

#define LED_ON    1
#define LED_OFF   0

#define ACTOR_ON    1
#define ACTOR_OFF   0



// Switch Actor Output
// ActorState must be ACTOR_OFF or ACTOR_ON
void SwitchActor(byte ActorState)
{
    if (ActorState == ACTOR_OFF)
    {
      digitalWrite(PIN_ACTOR, LOW);
    }
    else
    {      
      digitalWrite(PIN_ACTOR, HIGH);
    }
}


// Switch LED on the board
// LEDState must be LED_OFF or LED_ON
void SwitchBoardLED(byte LEDState)
{
    if (LEDState == LED_OFF)
    {
      digitalWrite(PIN_LED_BOARD, LOW);
    }
    else
    {      
      digitalWrite(PIN_LED_BOARD, HIGH);
    }
}


// Switch LEDs of RGBLED
// LEDColor must be LED_GREEN, LED_RED, LED_YELLOW or LED_OFF
void SwitchRGBLED(byte LEDColor)
{
    switch(LEDColor)
    {     
      case LED_GREEN:
         digitalWrite(PIN_LED_GREEN, HIGH);
         digitalWrite(PIN_LED_RED, LOW);
      break;
      
      case LED_RED:
         digitalWrite(PIN_LED_GREEN, LOW);
         digitalWrite(PIN_LED_RED, HIGH);
      break;
      
      case LED_YELLOW:
         digitalWrite(PIN_LED_GREEN, HIGH);
         digitalWrite(PIN_LED_RED, HIGH);
      break;
      
      case LED_OFF:
         digitalWrite(PIN_LED_GREEN, LOW);
         digitalWrite(PIN_LED_RED, LOW);
      break;     
    }
}



// this section handles configuration values which can be configured via webpage form in a webbrowser

// 8 configuration values max
String ConfigName[8];     // name of the configuration value
String ConfigValue[8];    // the value itself (String)
int    ConfigStatus[8];   // status of the value    0 = not set    1 = valid   -1 = not valid


// Initalize the values 
void InitializeConfigValues()
{
  for (int count = 0; count < 8; count++)
  {
    ConfigName[count] = "";
    ConfigValue[count] = "";
    ConfigStatus[count] = 0;
  }
}


// Build a HTML page with a form which shows textboxes to enter the values
// returns the HTML code of the page
String EncodeFormHTMLFromConfigValues(String TitleOfForm, int CountOfConfigValues)
{
   // Head of the HTML page
   String HTMLPage = "<!DOCTYPE html><html><body><h2>" + TitleOfForm + "</h2><form><table>";

   // for each configuration value
   for (int c = 0; c < CountOfConfigValues; c++)
   {
    // set background color by the status of the configuration value
    String StyleHTML = "";
    if (ConfigStatus[c] == 0) { StyleHTML = " Style =\"background-color: #FFE4B5;\" " ;};   // yellow
    if (ConfigStatus[c] == 1) { StyleHTML = " Style =\"background-color: #98FB98;\" " ;};   // green
    if (ConfigStatus[c] == -1) { StyleHTML = " Style =\"background-color: #FA8072;\" " ;};  // red

    // build the HTML code for a table row with configuration value name and the value itself inside a textbox   
    String TableRowHTML = "<tr><th>" + ConfigName[c] + "</th><th><input name=\"" + ConfigName[c] + "\" value=\"" + ConfigValue[c] + "\" " + StyleHTML + " /></th></tr>";

    // add the table row HTML code to the page
    HTMLPage += TableRowHTML;
   }

   // add the submit button
   HTMLPage += "</table><br/><input type=\"submit\" value=\"Submit\" />";

   // footer of the webpage
   HTMLPage += "</form></body></html>";
   
   return HTMLPage;
}


// Decodes a GET parameter (expression after ? in URI (URI = expression entered in address field of webbrowser)), like "Country=Germany&City=Aachen"
// and set the ConfigValues
int DecodeGETParameterAndSetConfigValues(String GETParameter)
{
   
   int posFirstCharToSearch = 1;
   int count = 0;
   
   // while a "&" is in the expression, after a start position to search
   while (GETParameter.indexOf('&', posFirstCharToSearch) > -1)
   {
     int posOfSeparatorChar = GETParameter.indexOf('&', posFirstCharToSearch);  // position of & after start position
     int posOfValueChar = GETParameter.indexOf('=', posFirstCharToSearch);      // position of = after start position
  
     ConfigValue[count] = GETParameter.substring(posOfValueChar + 1, posOfSeparatorChar);  // extract everything between = and & and enter it in the ConfigValue
      
     posFirstCharToSearch = posOfSeparatorChar + 1;  // shift the start position to search after the &-char 
     count++;
   }

   // no more & chars found
   
   int posOfValueChar = GETParameter.indexOf('=', posFirstCharToSearch);       // search for =
   
   ConfigValue[count] = GETParameter.substring(posOfValueChar + 1, GETParameter.length());  // extract everything between = and end of string
   count++;

   return count;  // number of values found in GET parameter
}








// +++++++++++++++++++ Start of Application Code +++++++++++++++++++




void setup() 
{
  
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BOARD, OUTPUT);
  pinMode(PIN_ACTOR, OUTPUT);

  pinMode(25, INPUT_PULLUP);

  
  preferences.begin("MyJourney", false);   // see https://github.com/espressif/arduino-esp32/blob/master/libraries/Preferences/examples/StartCounter/StartCounter.ino 


  WiFi_SetBothModesNetworkStationAndAccessPoint();


  SwitchRGBLED(LED_YELLOW); 
 
  
  // Open serial communications and wait for port to open:
  Serial.begin(19200);
  Serial.println("Hello ESP32!");


  // takeout 2 Strings out of the Non-volatile storage
  String strSSID = preferences.getString("SSID", "");
  String strPassword = preferences.getString("Password", "");

  // convert it to char*
  char* txtSSID = const_cast<char*>(strSSID.c_str());
  char* txtPassword = const_cast<char*>(strPassword.c_str());   // https://coderwall.com/p/zfmwsg/arduino-string-to-char 

  // try to connect to the LAN
  int success = WiFi_RouterNetworkConnect(txtSSID, txtPassword);
    
  if (success == 1)
  {
      SwitchRGBLED(LED_GREEN);        
  }
  else
  {
      SwitchRGBLED(LED_RED);  
  }   


  // Start access point with SSID "ESP32_MyJourneyIoT"
  int success2 = WiFi_AccessPointStart("ESP32_MyJourneyIoT");

  // switch LED off
  SwitchActor(ACTOR_OFF);

  // initialize config values and set first 3 names
  InitializeConfigValues();
  ConfigName[0] = "LED";
  ConfigName[1] = "SSID";
  ConfigName[2] = "PASSWORD";

  // put the NVS stored values in RAM for the program
  ConfigValue[1] = strSSID;
  ConfigValue[2] = strPassword;
 
  // start the webserver to listen for request of clients (in LAN or own ESP32 network)
  Webserver_Start();

}



// check the ConfigValues and set ConfigStatus
// process the first ConfigValue to switch something
void ProcessAndValidateConfigValues(int countValues)
{
  if (countValues > 8) {countValues = 8;};

  // if we have more than 1 value, store the second and third value in non-volatile storage
  if (countValues > 2)
  {
    preferences.putString("SSID", ConfigValue[1]);
    preferences.putString("Password", ConfigValue[2]);
  }

  // in our application the first value must be "00" or "FF" (as text string)
  if ((ConfigValue[0].equals("00")) || (ConfigValue[0].equals("FF")))
  {
    ConfigStatus[0] = 1;    // Value is valid
  }
  else
  {
    ConfigStatus[0] = -1;   // Value is not valid
  }

  // first config value is used to switch LED ( = Actor)
  if (ConfigValue[0].equals("00"))   
  {
    SwitchActor(ACTOR_OFF);
  }
  
  if (ConfigValue[0].equals("FF"))
  {
    SwitchActor(ACTOR_ON);
  }
  
}





void loop() {
   
    String GETParameter = Webserver_GetRequestGETParameter();   // look for client request
  
    if (GETParameter.length() > 0)        // we got a request, client connection stays open
    {
      if (GETParameter.length() > 1)      // request contains some GET parameter
      {
          int countValues = DecodeGETParameterAndSetConfigValues(GETParameter);     // decode the GET parameter and set ConfigValues
  
  
          if (WebRequestHostAddress == "192.168.4.1")                               // the user entered this address in browser, with GET parameter values for configuration
          {
    
                ProcessAndValidateConfigValues(3);                                  // check and process 3 ConfigValues, switch the LED, store SSID and Password in non-volatile storage
        
        
                String strSSID = preferences.getString("SSID", "");                 // takeout SSID and Password out of non-volatile storage  
                String strPassword = preferences.getString("Password", "");              
              
                char* txtSSID = const_cast<char*>(strSSID.c_str());                 // convert to char*
                char* txtPassword = const_cast<char*>(strPassword.c_str());         // https://coderwall.com/p/zfmwsg/arduino-string-to-char
  
                int successDisconnect = WiFi_RouterNetworkDisconnect();             // disconnect from router network
  
                SwitchRGBLED(LED_YELLOW); 
                
                int successConnect = WiFi_RouterNetworkConnect(txtSSID, txtPassword);   // then try to connect once new with new login-data
                  
                if (successConnect == 1)
                {
                  SwitchRGBLED(LED_GREEN); 
                }
                else
                {
                  SwitchRGBLED(LED_RED);                            
                }   
  
          }
          else                                                     // the user entered the ESP32 address in router network in his browser, with a GET parameter to switch the LED
          {
                ProcessAndValidateConfigValues(1);                 // then only the LED must be switched
  
          }
      }

      String HTMLPageWithConfigForm;
       
      if (WebRequestHostAddress == "192.168.4.1")                  //   the user entered this address in the browser, to get the configuration webpage               
      {
          HTMLPageWithConfigForm = EncodeFormHTMLFromConfigValues("ESP32 Webserver CONFIG", 3) + "<br>IP Address: " + WiFi_GetOwnIPAddressInRouterNetwork();       // build a new Configuration webpage with form and new ConfigValues entered in textboxes
      }
      else                                                         // the user entered the ESP32 address in router network in the browser, to get normal webpage
      {
          HTMLPageWithConfigForm = EncodeFormHTMLFromConfigValues("ESP32 Webserver Demo", 1);                                       // build a new webpage to control the LED
      }
      
      Webserver_SendHTMLPage(HTMLPageWithConfigForm);    // send out the webpage to client = webbrowser and close client connection
      
  }
  
  delay(50);
}
