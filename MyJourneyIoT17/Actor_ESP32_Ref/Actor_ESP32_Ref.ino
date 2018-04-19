/*
IOT Actor Board Reference Application 1
for ESP32 DevKitC
Elektor / Jens Nickel
My Journey in the IoT
 */


#include <WiFi.h>


// Function headers for TCP library
int TCPClient_Receive(byte TCPBytesReceivedBuffer[]);
int TCPClient_Send(byte data[], int len);



// +++++++++++++++++++ Start of MQTT Library ++++++++++++++++++

// MQTT Library functions based on PubSubClient by Nick O'Leary: http://pubsubclient.knolleary.net/index.html
// MIT-License: https://opensource.org/licenses/mit-license.php


byte SendBuffer[128];
byte ReturnBuffer[128];

byte MQTTCONNECT = 16;
byte MQTTPUBLISH = 48;
byte MQTTSUBSCRIBE = 128;
byte MQTTPINGREQ = 192;

// buffer to store payload bytes
byte MQTTClient_PayloadBuffer[128];


int MQTTClient_WriteStringInBuffer(String MString, byte buf[], int offset);
int MQTTClient_Send(byte MessageType, byte buf[], int RemainingLengthByte);

// Send MQTT-connection-request to MQTT-Server via TCP
// returns 1 if accepted othwerwise 0
int MQTTClient_Connect(String clientid) 
{
    int result = 0;

    // fill the buffer with data to be sent for connection request
    int BufferIndex = 2;  //  we start at index 2 to leave room for message type byte and remaininglength byte

    // for MQTT version 3.1.1 you always have to sent the following bytes
    byte d[7] = { 0, 4, 77, 81, 84, 84, 4};

    // fill the buffer with these bytes
    for (int j = 0 ;j < 7; j++) 
    {
        SendBuffer[BufferIndex++] = d[j];
    }

    // next byte position is filled with flags, we only use the clean session flag
    SendBuffer[BufferIndex++] = 0x02;

    // next two bytes are keep alive time
    SendBuffer[BufferIndex++] = 0;
    SendBuffer[BufferIndex++] = 180;

    // then write the client-ID in the buffer, with length bytes at the beginning
    BufferIndex = MQTTClient_WriteStringInBuffer(clientid, SendBuffer, BufferIndex);

    // send the buffer via TCP/IP, put message type and remaining length byte at the beginning
    result = MQTTClient_Send(MQTTCONNECT, SendBuffer, BufferIndex - 2); 

    delay(1000);

    // get bytes back from TCPServer = MQTT Broker
    byte ReturnLength = TCPClient_Receive(ReturnBuffer);
    
    if (ReturnLength > 0)
    {
      if ((ReturnBuffer[0] != 32) || (ReturnBuffer[1] != 2))
      {
        result = 0; // answer of MQTT Broker is not 32, 2, ...
      }      
    }
    else
    {
      result = 0; // no answer of MQTT Broker        
    }
  
    return result;  
}



// Send MQTT-Publish-message to MQTT-Server via TCP
// the byte array payload contains the payload data, plength is the amount of payload bytes
int MQTTClient_Publish(String topic, byte payload[], int plength) 
{
    // fill the buffer with data to be sent for publish message
    int BufferIndex = 2;  //  we start at index 2 to leave room for message type byte and remaininglength byte

    // write the topic in the buffer, with topic length bytes at the beginning and offset = 2
    BufferIndex = MQTTClient_WriteStringInBuffer(topic, SendBuffer, BufferIndex);

    // then copy the payload bytes in the buffer
    for (int i = 0; i  < plength; i++)
    {
        SendBuffer[BufferIndex++] = payload[i];
    }

    // send the buffer via TCP/IP, put message type and remaining length byte at the beginning
    return MQTTClient_Send(MQTTPUBLISH, SendBuffer, BufferIndex - 2);
}



// Send out a Ping request to MQTT server, this must be done during the Keep alive time (here: 180 s)
// returns 1 if Ping response was received othwerwise 0 
int MQTTClient_Ping()
{
    int result = 1;
    
    // send the buffer (here: empty) via TCP/IP, put message type and remaining length byte at the beginning
    result = MQTTClient_Send(MQTTPINGREQ, SendBuffer, 0);

    delay(1000);

    // get bytes back from TCPServer = MQTT Broker
    byte ReturnLength = TCPClient_Receive(ReturnBuffer);

    if (ReturnLength > 0)
    {
      if ((ReturnBuffer[0] != 208) || (ReturnBuffer[1] != 0))
      {
        result = 0; // answer of MQTT Broker is not 208, 0
      }      
    }
    else
    {
      result = 0; //  no answer of MQTT Broker   
    }

    return result;
    
}



// Send MQTT-Subscribe-message to MQTT-Server via TCP
// returns 1 if accepted othwerwise 0 
int MQTTClient_Subscribe(String topicfilter) 
{
    int result = 1;
    
    // fill the buffer with data to be sent for publish message
    int BufferIndex = 2;  //  we start at index 2 to leave room for message type byte and remaininglength byte

    SendBuffer[BufferIndex++] = 0x00;
    SendBuffer[BufferIndex++] = 0x02;

    // write the topic in the buffer, with topic length bytes at the beginning and offset = 2
    BufferIndex = MQTTClient_WriteStringInBuffer(topicfilter, SendBuffer, BufferIndex);

    SendBuffer[BufferIndex++] = 0x00;  // Quality of service

    // send the buffer via TCP/IP, put message type and remaining length byte at the beginning
    result = MQTTClient_Send(MQTTSUBSCRIBE + 2, SendBuffer, BufferIndex - 2);
    
    delay(1000);

    // get bytes back from TCPServer = MQTT Broker
    byte ReturnLength = TCPClient_Receive(ReturnBuffer);

    if (ReturnLength > 0)
    {
      if ((ReturnBuffer[0] != 144) || (ReturnBuffer[1] != 3))
      {
        result = 0; // answer of MQTT Broker is not 144, 3, ...
      }      
    }
    else
    {
      result = 0; //  no answer of MQTT Broker   
    }

    return result;
}



// Polls if MQTT message from server was received (of course topic must match a subscribed topic filter)
// stores the payload in MQTTBytesPayloadBuffer which must be allocated by the calling function
// returns the number of payload bytes received, 0 when no bytes were received

int MQTTClient_Get(byte MQTTBytesPayloadBuffer[])
{   
    byte MQTTBytesReceivedBuffer[128];
    byte MQTTBytesReceivedCount = TCPClient_Receive(MQTTBytesReceivedBuffer);

    byte MQTTBytesPayloadCount = 0;

    // we received bytes from the broker
    if (MQTTBytesReceivedCount > 0)
    {
      // with a Publish command byte at the beginning
      // so we know this must be a message from some client
      if (MQTTBytesReceivedBuffer[0] == MQTTPUBLISH)
      {
         //byte RemainingLengthByte = MQTTBytesReceivedBuffer[1];
         
         // get the length of the Publish topic inside this message
         // remark: we do not process the topic here
         byte TopicLength = MQTTBytesReceivedBuffer[3];

         // get the payload bytes
         for (int c = 4 + TopicLength; c < MQTTBytesReceivedCount; c++)
         {
            // put it in MQTTBytesPayloadBuffer[]
            MQTTBytesPayloadBuffer[c - 4 - TopicLength] = MQTTBytesReceivedBuffer[c];
         }

         // calculate number of payload bytes
         MQTTBytesPayloadCount = MQTTBytesReceivedCount - 4 - TopicLength;
      }
       
    }

    return MQTTBytesPayloadCount;
}



// send out a MQTT message from client to server
// send the buffer via TCP/IP, put message type and remaining length byte at the beginning
// buf contains the bytes to be sent out, with 2 bytes left free at the beginning
// RemainingLengthBytes is the number of bytes, without the 2 bytes at the beginning
int MQTTClient_Send(byte MessageType, byte buf[], int RemainingLengthByte)
{   
    buf[0] = MessageType;        
    buf[1] = RemainingLengthByte & 255;
      
    // send out all bytes via TCP/IP
    int result = TCPClient_Send(buf, RemainingLengthByte + 2);

    return result;
}





// this function writes the bytes (ASCII) of a string in the buffer buf, with a given offset, and returns a new offset/index back
int MQTTClient_WriteStringInBuffer(String MString, byte buf[], int offset) 
{
  
    int Slength = MString.length();
    
    // copy the bytes of this array in the buffer, with the given offset, and leave 2 bytes free at the beginning
    for (int j = 0; j < Slength; j++)
    {
        buf[offset + 2 + j] = MString[j];
    }

    // fill the lenght of the string in the first 2 bytes (string length is lower than 128 in that case)
    buf[offset] = 0;
    buf[offset + 1] = Slength;

    // return new offset/index after the string
    return offset + 2 + Slength;
}




// +++++++++++++++++++ End of MQTT library ++++++++++++++++++++




// +++++++++++++++++++ Start of TCP Library +++++++++++++++++++

// as ESP32 Arduino Version

WiFiClient client;

// Closes TCP/IP-connection
// returns 1 when ok, 0 when error

int TCPClient_Close()
{
    int Result = 0;

    client.stop();

    // no detection of error in this implementation of function
    Result = 1;   
    
    return Result;
}




// Establish TCP/IP-connection to TCP/IP-Server
// returns 1 when ok, < 0 when error

int TCPClient_Connect(char* IPAddressString, int Port)
{
    // see https://www.arduino.cc/en/Reference/ClientConnect
    // Returns an int (1,-1,-2,-3,-4) indicating connection status :
    // SUCCESS 1
    // TIMED_OUT -1
    // INVALID_SERVER -2
    // TRUNCATED -3
    // INVALID_RESPONSE -4
    
    int Result = client.connect(IPAddressString, Port);

    return Result;
}




// Polls if bytes were received via TCP/IP from server
// stores it in TCPBytesReceivedBuffer which must be allocated by the calling function
// returns the number of bytes received, 0 when no bytes were received

int TCPClient_Receive(byte TCPBytesReceivedBuffer[])
{
    // see https://www.arduino.cc/en/Reference/StreamReadBytes
    // readBytes() read characters from a stream into a buffer. 
    // The function terminates if the determined length has been read, or it times out (see setTimeout()).

    // here we read 120 Bytes max.
    // Timeout is 1000 ms by default
    int TCPBytesReceivedCount = client.readBytes(TCPBytesReceivedBuffer, 120);

    // number of bytes received from server
    return TCPBytesReceivedCount;
}



// send out bytes via TCP/IP to server
// data contains the bytes to be sent out, len is the number of bytes to be sent out
// returns 1 when ok, 0 when error

int TCPClient_Send(byte data[], int len)
{
     int Result = 0;

     // see https://www.arduino.cc/en/Reference/ClientWrite
     // write() returns the number of bytes written. It is not necessary to read this value.
     int BytesWritten = client.write(data,len);

     // if all bytes were sent out, we assume everything is ok
     if (BytesWritten == len)
     {     
      Result = 1;   
     }

     return Result;
}



// +++++++++++++++++++ End of TCP library +++++++++++++++++++++




const char* txtSSID = "YOUR SSID";
const char* txtPassword = "YOUR PASSWORD";



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

#define MODE_TEST      0
#define MODE_SENSOR    1



#define NODESTATE_RESET                    3    // S3 = connection + subscription established after user reset
#define NODESTATE_PING                     2    // S2 = connection still open, successful ping
#define NODESTATE_OK                       1    // S1 = connection + subscription established
#define NODESTATE_NOTSPECIFIEDERROR        0    // E0 = unspecified error
#define NODESTATE_NOTCPCONNECTION          -1   // E1 = connection not established because of no tcp connection to test broker
#define NODESTATE_NOMQTTCONNECTION         -2   // E2 = connection not established because broker rejected MQTT connection attempt
#define NODESTATE_NOMQTTSUBSCRIPTION       -3   // E3 = broker rejected subscription attempt
#define NODESTATE_NOPING                   -4   // E4 = ping was not successful, connection lost



int DeviceMode = MODE_TEST;

int NodeStateLog = NODESTATE_OK;     // no error so far

byte MQTTClient_Connected = false;

boolean buttonState = LOW;
boolean oldbuttonState = LOW;
byte ButtonCount = 0;

byte CounterForPing = 0;


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





void setup() 
{
  
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BOARD, OUTPUT);
  pinMode(PIN_ACTOR, OUTPUT);

  pinMode(0, INPUT);

  boolean buttonState = LOW;
  boolean oldbuttonState = LOW;
  
  DeviceMode = MODE_TEST;
  NodeStateLog = NODESTATE_OK;  //  no error was logged

  SwitchRGBLED(LED_YELLOW); 
  SwitchActor(ACTOR_OFF);
  
  // Open serial communications and wait for port to open:
  Serial.begin(19200);
  Serial.println("Hello ESP32!"); 

  boolean success = true;

 
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
  
  if (WiFi.status() != WL_CONNECTED)
  {
    success == false;
  }

  
  if (success == true)
  {
    SwitchRGBLED(LED_GREEN); 
  }
  else
  {
    SwitchRGBLED(LED_RED);  
  }   


  // connection must be established (see loop function)
  MQTTClient_Connected = false;
  
  SwitchActor(ACTOR_OFF);

 
}



// Send the Ping Request to MQTT Broker and switch RGB-LEDs
int SendPingRequestToBroker()
{   
    int NodeState;
    
    // LED yellow
    SwitchRGBLED(LED_YELLOW); 

    // send out MQTT-Ping-Request
    int MQTT_Ping_Result = MQTTClient_Ping();

    if (MQTT_Ping_Result == 1)
    {
      // we received the Ping Response from Broker
      SwitchRGBLED(LED_GREEN); 
      NodeState = NODESTATE_PING;
    }
    else
    {
      // no Ping Response was received
      SwitchRGBLED(LED_RED); 
      NodeState = NODESTATE_NOPING;  
    }

    return NodeState; 
}


// Connect and subscribe to topic and switch RGB-LEDs
int ConnectAndSubscribeToTopic()
{   
    int NodeState;

    // closes TCP connection
    TCPClient_Close();
    
   
    // TCP-connect to HiveMQ MQTT Testserver
    int TCP_Connect_Result = TCPClient_Connect("broker.hivemq.com", 1883);
    
    // second option: Mosquitto testserver (other MQTT clients in your project must be adapted of course)
    // int TCP_Connect_Result = TCPClient_Connect("test.mosquitto.org", 1883);
    

    // Success?
    if (TCP_Connect_Result != 1)
    {
      // no = red LED
      SwitchRGBLED(LED_RED); 
      NodeState = NODESTATE_NOTCPCONNECTION;  
    }
    else
    {
      // yes = LED yellow
      SwitchRGBLED(LED_YELLOW); 

      // send out MQTT Connect request
      int MQTT_Connect_Result = MQTTClient_Connect("elektorIoT");
      
      if (MQTT_Connect_Result == 1)
      {
        // subscribe to topic
        int MQTT_Subscribe_Result = MQTTClient_Subscribe("/ElektorMyJourneyIoT/TestTopic/test");

        if (MQTT_Subscribe_Result == 1)
        {
          SwitchRGBLED(LED_GREEN);
          NodeState = NODESTATE_OK;
        }
        else
        {
          SwitchRGBLED(LED_RED); 
          NodeState = NODESTATE_NOMQTTSUBSCRIPTION;           
        }
      }
      else
      {
        // not connected
        SwitchRGBLED(LED_RED); 
        NodeState = NODESTATE_NOMQTTCONNECTION;  
      }
   }

   return NodeState;
}



// Publish node state via MQTT, as message for other client
void SendNodeStateOverMQTT(int NodeState)
{
    if (NodeState > 0)  // normal state
    {
      // first char of message is an 'S'
      MQTTClient_PayloadBuffer[0] = 'S'; 
      MQTTClient_PayloadBuffer[1] = 48 + NodeState;   // '1'..'3'
    }
    else                // error
    {
      // first char of message is an 'E'
      MQTTClient_PayloadBuffer[0] = 'E'; 
      MQTTClient_PayloadBuffer[1] = 48 + (NodeState * -1);   // '0'..'4'
    }

    // Publish the 2 char message with a special topic
    MQTTClient_Publish("/ElektorMyJourneyIoT/TestTopicBack/test", MQTTClient_PayloadBuffer, 2);
}



// Publish node state via serial interface, for local user
void SendNodeStateOverSerial(int NodeState)
{
    if (NodeState > 0)  // normal state
    {
      Serial.print("S");
      Serial.println(NodeState);
    }
    else               // error
    {
      Serial.print("E");
      Serial.println(NodeState * -1);  
    }
}




// Publish actor state via MQTT, as acknowledge message for other client
void SendActorStateOverMQTT(byte ActorState)
{
    // first char of message is an 'R'
    MQTTClient_PayloadBuffer[0] = 82;
  
    if (ActorState == ACTOR_OFF)
    {
      // second char is a '0'
      MQTTClient_PayloadBuffer[1] = 48;
    }
    else
    {      
      // second char is a '1'
      MQTTClient_PayloadBuffer[1] = 49;
    }

    // Publish the 2 char message with a special topic
    MQTTClient_Publish("/ElektorMyJourneyIoT/TestTopicBack/test", MQTTClient_PayloadBuffer, 2);
}




void loop() {
  
  // read state of button
  buttonState = digitalRead(0);
 
  // button was pressed
  if (buttonState == LOW && oldbuttonState == HIGH)
  {
    SwitchActor(ACTOR_OFF);

    // connection should be established once new
    MQTTClient_Connected = false;
    NodeStateLog = NODESTATE_RESET;     // store the reason why the connection should be established once new
  }

  oldbuttonState = buttonState;



  // connection should be established initially or once new
  if (MQTTClient_Connected == false)
  {
    int NodeState = ConnectAndSubscribeToTopic();

    if (NodeState > 0)   //  connection and subscription established successfully
    {
      SendNodeStateOverMQTT(NodeStateLog);    // send the reason why the connection was established (once new) via MQTT
      MQTTClient_Connected = true;            // we can go in normal mode 
    }

    SendNodeStateOverSerial(NodeStateLog);    // send the reason why the connection was established (once new) via serial interface
    NodeStateLog = NodeState;                 // store the "ok" or the connection error as reason why the connection should be established once new
    
    // see below
    CounterForPing = 0;
  }



  // we successfully connected with MQTT broker and subscribed to topic and can go in normal mode
  if (MQTTClient_Connected == true)
  {

    // the loop function is processed approx. every second
    // every 15 s we will send out a ping
    CounterForPing++;

    if (CounterForPing > 15)
    {
      int NodeState = SendPingRequestToBroker();
      
      if (NodeState > 0)   // ping response was received
      {
        SendNodeStateOverMQTT(NodeState);   // send the NODESTATE_PING via MQTT        
      }
      else                 // no ping response was received
      {
        MQTTClient_Connected = false;        // we have to establish the connection again
      }
      
      SendNodeStateOverSerial(NodeState);   // send the NODESTATE_PING or NODESTATE_NOPING via serial interface 
      NodeStateLog = NodeState;             // store the NODESTATE_PING or NODESTATE_NOPING in the buffer variable
            
      CounterForPing = 0;
    }


    // every second we look for a message which was forwarded by the broker with subscribed topic
    byte PayloadCountReceivedMessage = MQTTClient_Get(MQTTClient_PayloadBuffer);
    
    // we got a message with more than 2 chars payload
    if (PayloadCountReceivedMessage > 2)
    {         
        // "00FF" was received (we just test second and forth byte)
        if ((MQTTClient_PayloadBuffer[1] == 48) && (MQTTClient_PayloadBuffer[3] == 70))
        { 
          // Switch actor on and send back the acknowledge message with actor state ON
          SwitchActor(ACTOR_ON);
          SendActorStateOverMQTT(ACTOR_ON);
        }

        // "0000" was received (we just test second and forth byte)
        if ((MQTTClient_PayloadBuffer[1] == 48) && (MQTTClient_PayloadBuffer[3] == 48))
        { 
          // Switch actor off and send back the acknowledge message with actor state OFF
          SwitchActor(ACTOR_OFF);
          SendActorStateOverMQTT(ACTOR_OFF);
        }
      
    }
    
  }


  delay(1000);
  
}
