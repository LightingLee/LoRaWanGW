/*
  LoRa Simple Yun Server :
  Support Devices: LG01. 
  
  Example sketch showing how to create a simple messageing server, 
  with the RH_RF95 class. RH_RF95 class does not provide for addressing or
  reliability, so you should only use RH_RF95 if you do not need the higher
  level messaging abilities.
  It is designed to work with the other example LoRa Simple Client
  User need to use the modified RadioHead library from:
  https://github.com/dragino/RadioHead
  modified 16 11 2016
  by Edwin Chen <support@dragino.com>
  Dragino Technology Co., Limited
*/
//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

//If you use Dragino Yun Mesh Firmware , uncomment below lines. 
//#define BAUDRATE 250000

#include <Console.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <Base64.h>
#include <String.h>
#include <FileIO.h>
  
// Singleton instance of the radio driver
RH_RF95 rf95;

float frequency = 923.2;

static const char *server = "58.157.42.117";
static const int port = 19999;
//Set Debug = 1 to enable Console Output;
int debug=0; 

void setup() 
{
  Bridge.begin(BAUDRATE);
  Console.begin();
  while (!Console) ; // Wait for console port to be available

  Console.println("Start Sketch");
  if (!rf95.init())
  Console.println("init failed"); // Setup ISM frequency
  rf95.setFrequency(frequency); // Setup Power,dBm
  rf95.setTxPower(13);
  
  Console.print("Listening on frequency: ");
  Console.println(frequency);
}

void loop()
{
  if (rf95.available())
  {
    Console.println("Get Data! "); // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len))
    {
      
      Console.println("Data is ");
      for(int i=0; i<len; i++){
        Console.println(buf[i]);    
      }

      int fdjzs = buf[0];
      float shidu = (float)(buf[1] + buf[2]/100);
      float jcwd = (float)(buf[3] + buf[4]/100);

      String  data = "{\"code\":\"WG73\", \"jcwd\":\"" + String(jcwd) + "\", \"fdjzs\":\"" + String(fdjzs) + "\", \"shidu\":\"" + String(shidu) + "\"}";
      
      int datalength = data.length() + 1;
      char inputString[datalength];
      data.toCharArray(inputString, datalength);
      Console.println(inputString);
      Console.println(datalength);
/*
      Console.print("Input string is:\t");
      Console.println(inputString);
    
      int encodedLength = Base64.encodedLength(inputStringLength);
      char encodedString[encodedLength];
      Base64.encode(encodedString, inputString, inputStringLength);
      Console.print("Encoded string is:\t");
      Console.println(encodedString);
*/


      int tosendlength = sizeof(inputString);
      FileSystem.begin();
      File dataFile = FileSystem.open("/var/iot/data", FILE_WRITE);
      for(int j=0;j<tosendlength;j++){
        dataFile.print(inputString[j]);
      }
      dataFile.close();
      Console.println("Save data");
      Console.println(tosendlength);
      delay(200);

      
      //send the messages
      Console.println("Send Start");
      Process p;    // Create a Linux Process
      p.begin("/usr/bin/lora_udp_fwd");  // Process that launch the "lora_udp_fwd" command
      p.addParameter("/var/iot/data");
      p.addParameter(String(rf95.lastRssi())); // Send stat
      p.addParameter(String());
      p.run();    // Run the process and wait for its termination
      Console.println("Already Send");
      
      while (p.available() > 0) {
        char c = p.read();
        if ( debug > 0 ) Console.print(c);
      }
      // Ensure the last bit of data is sent.
      if ( debug > 0 ) Console.flush(); 

/*
      char *tosend = "{\"rxpk\":[{\"tmst\":1510104664,\"chan\":0,\"rfch\":0,\"freq\":923.20,\"stat\":1,\"modu\":\"LORA\",\"datr\":\"SF7BW125\",\"codr\":\"4/5\",\"lsnr\":9,\"rssi\":-24,\"size\":5,\"data\":\"";
      strcat(tosend, encodedString);
      strcat(tosend, "\"}]}");
      int tosendlength = sizeof(tosend);
      Console.print("ToSendsize is ");
      Console.println(tosendlength);
    
      Process p;
      Console.println("Start Ping");
      p.begin("python");
      p.addParameter("--version");
      p.run();
      Console.println("End");
      while(p.running());
      while (p.available()) {
        char c = p.read();
        Console.print(c);
      }
      Console.flush();
*/ 
    }
    else
    {
      Console.println("recv failed");
    }
  }
}


