#include <types.h>
#include <uECC.h>
#include <uECC_vli.h>
#include <time.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <Crypto.h>
#include <SHA256.h>

// 1) Uncomment the type of node you are using
#define TEST_SENSOR
//#define GLOBE_SENSOR
//#define REL_HUMIDITY
//#define INDOOR
//#define OUTDOOR
//#define OCCUPANCY
//#define OUTPUTNODE

// 2) Define your pins for the WiFi Module (See comments on each define)
/* 
#define SPIWIFI       SPI  // The SPI port
#define SPIWIFI_SS    9   // Chip select pin
#define ESP32_RESETN  5   // Reset pin
#define SPIWIFI_ACK   7   // a.k.a BUSY or READY pin
#define ESP32_GPIO0   -1 
*/
 #define SPIWIFI       SPI  // The SPI port
#define SPIWIFI_SS    13   // Chip select pin
#define ESP32_RESETN  12   // Reset pin
#define SPIWIFI_ACK   11   // a.k.a BUSY or READY pin
#define ESP32_GPIO0   -1 


// 3) Change DEBUG to 0 when using without SERIAL printouts (i.e. in normal use)
#define DEBUG 1

// 4) Ensure DEVICE_TYPE String is set to be the DEFINED name of your sensor (Caps sensitive, make sure it matches perfectly)
// Capped at 16 bytes, the rest will get cut off in runtime if you go over
#ifdef REL_HUMIDITY
  #define DEVICE_TYPE "Rel. Humidity"
#endif

#ifdef INDOOR
  #define DEVICE_TYPE "Indoor"
#endif

#ifdef OUTDOOR
  #define DEVICE_TYPE "Outdoor"
#endif

#ifdef OCCUPANCY
  #define DEVICE_TYPE "Occupancy"
#endif

#ifdef OUTPUTNODE
  #define DEVICE_TYPE "Output"
#endif

#ifdef TEST_SENSOR
  #define DEVICE_TYPE "Test"
#endif

#ifdef GLOBE_SENSOR
  #define DEVICE_TYPE "Globe"
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Structs to populate

// 5) Ensure the data being sent is correct for your type of sensor

#if defined(INDOOR) || defined(OUTDOOR)
  // Definition of temperatureData
  struct responseData {
    double temp;
  };
#endif

#ifdef TEST_SENSOR
  // Definition of temperatureData
  struct responseData {
    double test_x;
  };
#endif

#ifdef GLOBE_SENSOR
  // Definition of temperatureData
  struct responseData {
    double globeReading;
  };
#endif

#ifdef REL_HUMIDITY
  struct responseData {
    double humidity;
  };
#endif

#ifdef OCCUPANCY
  struct responseData {
    bool isOccupied;
  };
#endif

#ifdef OUTPUTNODE
  struct responseData {
    double powerIn;
    double tempIn;
  };
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// YOUR CODE HERE

// place code here that you would like to run when the node turns on
void setupCOMFORT() {
  
}

// number of SECONDS between each running of this function
#define LOOP_TIMER 1

// must set LOOP_TIMER for time of which this function should run again
// optional function which should run regularly
int onTimerLoop() {
  
  return LOOP_TIMER;
}

// Function that runs after the data is pos edge (good for "runs once per data send" functions)
void onDataEdge() {
  
}

//--------------------------------FUNCTIONS FOR INPUT NODES------------------------------------------------------///

 // must return a responseData containing the data to send to the server
 // Populate struct members here
#ifndef OUTPUTNODE

responseData onDataRequest() {
    responseData data;

    //data.temp = (double)RTC->MODE2.CLOCK.bit.SECOND;
  return data;
}

#endif

//--------------------------------FUNCTIONS FOR OUTPUT NODES------------------------------------------------------///
#ifdef OUTPUTNODE

// Function for Output Node code -- will run after receiving a new value from the centralized interface
// inData contains the received data, as per the struct given above for OUTPUTNODE
void onDataReceived(responseData inData) {
  // Code for what to do with incoming data here
  
  //Serial.print("Received Value: ");
  //Serial.println(inData.powerIn);
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////

#define HASH_SIZE 32
#define BLOCK_SIZE 64
#define SERIAL_NUMBER ((uint8_t *)0x008061FC)

#include "arduino_secrets.h"



#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)


#define UL unsigned long
#define znew  ((z=36969*(z&65535)+(z>>16))<<16)
#define wnew  ((w=18000*(w&65535)+(w>>16))&65535)
#define MWC   (znew+wnew)
#define SHR3  (jsr=(jsr=(jsr=jsr^(jsr<<17))^(jsr>>13))^(jsr<<5))
#define CONG  (jcong=69069*jcong+1234567)
#define KISS  ((MWC^CONG)+SHR3)

#define trand (TRNG->DATA.reg)


///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
const byte UniqueIdentifier[] = "COMFORTController V0.1";
const byte dataRequest[] = "DataRequest:";
const byte dataResponse[] = "Response:";
const byte dataOutput[] = "Output Data:";
const byte dataReceived[] = "DataReceived";
const int PACKET_SIZE = 128;
const int AUTH_EXACT_SIZE = 118;
const int PUBKEY_SIZE = 64;
const int RETURN_PACKET_SIZE = 0;
uint32_t seed;
uint32_t counter;

#ifdef OUTPUTNODE
responseData dataBuffer;
#endif

int keyIndex = 0;            // your network key Index number (needed only for WEP)
unsigned int localPort = 20000;
unsigned int outwardPort = 20001;

IPAddress serverIP;
int status = WL_IDLE_STATUS;
//IPAddress broadcast(255, 255, 255, 255); // IP address for broadcasting to the Raspberry Pi (might need tweaking, ask Joseph)

// Encryption Keys
byte publicKey[64]; //public key of server
byte nodePubKey[64]; // public key of this node
byte sharedKey[32];
byte privateKey[32];
byte IV[16]; // Populated by 2 64 bit numbers sent by Central Interface
/*  Global static variables:
    (the seed changes on every minute) */
static int timeValue = ((int32_t)((*(int16_t *) & (__DATE__[9])) & 0xF) << 28) | ((int32_t)((*(int16_t *) & (__DATE__[10])) & 0xF) << 24) | ((int32_t)((*(int16_t *) & (__DATE__[4])) & 0xF) << 20) | ((int32_t)((*(int16_t *) & (__DATE__[5])) & 0xF) << 16) | (((*(int16_t *) & (__TIME__[3])) & 0xF) << 12) | (((*(int16_t *) & (__TIME__[4])) & 0xF) << 8) | (((*(int16_t *) & (__TIME__[6])) & 0xF) << 4) | ((*(int16_t *) & (__TIME__[7])) & 0xF);
static UL z = 362436069 * (int)timeValue, w = 521288629 * (int)timeValue, \
              jsr = 123456789 * (int)timeValue, jcong = 380116160 * (int)timeValue;


// Device ID and Information
byte deviceID[8];
byte sensorType[16];

WiFiClient Tcp;
WiFiUDP Udp;
SHA256 hash;

void setup() {
  char Stringy[] = DEVICE_TYPE;
  comfortWiFiSetup(Stringy, sizeof(Stringy)-1);
  setupCOMFORT();
  authenticate();
}

void loop() {
    unsigned int timerDelay = 0;
    unsigned int lastRunTime = 0;
    time_t lastRun;
    uint32_t clockTime = 0;
    tm tme;
    
    clockTime = RTC->MODE2.CLOCK.reg;
    
    tme.tm_year = 116 + (clockTime >> 26 & 0x1F); // - 1?
    tme.tm_mon = (clockTime >> 22 & 0xF) - 1;
    tme.tm_mday = (clockTime >> 17 & 0x1F);
    tme.tm_hour = (clockTime >> 12 & 0x1F);
    tme.tm_min = (clockTime >> 6 & 0x3F);
    tme.tm_sec = (clockTime & 0x3F);
    lastRun = mktime((tm *)&tme);
        
    onDataEdge();
    #ifdef OUTPUTNODE
      while(!(dataRefreshReceived(Tcp))) {
        clockTime = RTC->MODE2.CLOCK.reg;
        tme.tm_year = 116 + (clockTime >> 26 & 0x1F); // - 1?
        tme.tm_mon = (clockTime >> 22 & 0xF) - 1;
        tme.tm_mday = (clockTime >> 17 & 0x1F);
        tme.tm_hour = (clockTime >> 12 & 0x1F);
        tme.tm_min = (clockTime >> 6 & 0x3F);
        tme.tm_sec = (clockTime & 0x3F);
        time_t unix = mktime((tm *)&tme);
        
        int diff = (lastRun > unix) ? ((int)lastRun - (int)unix) : ((int)unix - (int)lastRun);
        if (diff >= timerDelay) {
          #if (DEBUG == 1)
             Serial.print(RTC->MODE2.CLOCK.bit.MONTH);
            Serial.print("/");
            Serial.print(RTC->MODE2.CLOCK.bit.DAY);
            Serial.print("/");
            Serial.print(RTC->MODE2.CLOCK.bit.YEAR+2016);
            Serial.print(" ");
            Serial.print(RTC->MODE2.CLOCK.bit.HOUR);
            Serial.print(":");
            Serial.print(RTC->MODE2.CLOCK.bit.MINUTE);
            Serial.print(":");
            Serial.println(RTC->MODE2.CLOCK.bit.SECOND);
          #endif
          timerDelay = onTimerLoop();
          if (timerDelay == 0) {
            timerDelay++;
          }
          lastRun = unix;
        }
      }
      
  #else
    while(!(dataRequestReceived(Tcp))) {
        clockTime = RTC->MODE2.CLOCK.reg;
        tme.tm_year = 116 + (clockTime >> 26 & 0x1F); // - 1?
        tme.tm_mon = (clockTime >> 22 & 0xF) - 1;
        tme.tm_mday = (clockTime >> 17 & 0x1F);
        tme.tm_hour = (clockTime >> 12 & 0x1F);
        tme.tm_min = (clockTime >> 6 & 0x3F);
        tme.tm_sec = (clockTime & 0x3F);
        time_t unix = mktime((tm *)&tme);
        
        int diff = (lastRun > unix) ? ((int)lastRun - (int)unix) : ((int)unix - (int)lastRun);
        if (diff >= timerDelay) {
          #if (DEBUG == 1)
             Serial.print(RTC->MODE2.CLOCK.bit.MONTH);
            Serial.print("/");
            Serial.print(RTC->MODE2.CLOCK.bit.DAY);
            Serial.print("/");
            Serial.print(RTC->MODE2.CLOCK.bit.YEAR+2016);
            Serial.print(" ");
            Serial.print(RTC->MODE2.CLOCK.bit.HOUR);
            Serial.print(":");
            Serial.print(RTC->MODE2.CLOCK.bit.MINUTE);
            Serial.print(":");
            Serial.println(RTC->MODE2.CLOCK.bit.SECOND);
          #endif
          timerDelay = onTimerLoop();
          if (timerDelay == 0) {
            timerDelay++;
          }
          lastRun = unix;
        }
      }
  #endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void getSerialNumber(uint8_t * dest, size_t getLength) {
  if (getLength > 16) { return; }
  for (int i = 0; i < getLength; i++) {
    dest[i] = 3;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void init_TRNG() {
  MCLK->APBCMASK.reg |= MCLK_APBCMASK_TRNG;
  TRNG->CTRLA.reg |= TRNG_CTRLA_ENABLE;
  
  while((TRNG->INTFLAG.reg & TRNG_INTFLAG_DATARDY) == 0) {
      //seed = trand;
    }
  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int getTRNGValue() {
  if (seed != trand) {
    srand(trand);
    seed = trand;
  }
  return rand();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void comfortWiFiSetup(const char * inputType, size_t sensorTypeLength) {
  char newID[8];
  makeDeviceID((char *)newID);
  comfortWiFiSetup(newID, inputType, sensorTypeLength);
}

void comfortWiFiSetup(const char * D_ID, const char * inputType, size_t sensorTypeLength) {
  memcpy(deviceID, D_ID, sizeof(deviceID));
  memset(sensorType,0,sizeof(sensorType));
  memcpy(sensorType,inputType,sensorTypeLength);
  // If not serial.
  if (!Serial) {
    Serial.begin(9600);
  }

  #if (DEBUG)
    while (!Serial) {}
    Serial.println();
  #endif

   clkInit();
  for(int i = 0; i < 8; i++) {
    Serial.print((char)deviceID[i]);
    Serial.print(" ");
  }
  Serial.println("");
  uint32_t t = 0;
 // t = RTC->MODE2.CLOCK.reg;
  init_TRNG();
  delay(2000);
  
  /*while(true) {
    
    //uint8_t month;
    //static uint32_t prev = 0;
    
    Serial.print(RTC->MODE2.CLOCK.bit.MONTH);
    Serial.print("/");
    Serial.print(RTC->MODE2.CLOCK.bit.DAY);
    Serial.print("/");
    Serial.print(RTC->MODE2.CLOCK.bit.YEAR+2016);
    Serial.print(" ");
    Serial.print(RTC->MODE2.CLOCK.bit.HOUR);
    Serial.print(":");
    Serial.print(RTC->MODE2.CLOCK.bit.MINUTE);
    Serial.print(":");
    Serial.println(RTC->MODE2.CLOCK.bit.SECOND);
    //prev = t;
    delay(1000);
    
  } */
  
  uint32_t * privateKeyInitializer = (uint32_t *)privateKey;
  for (int i = 0; i < 8; i++) {
    privateKeyInitializer[i] = getTRNGValue();
  }
  
  // Make uECC key
  uECC_set_rng(&getRNGValues);
  uECC_compute_public_key(privateKey, nodePubKey, uECC_secp256r1());
  counter = getTRNGValue();

  // Private Key
  Serial.print("Private Key: ");
  for (int i = 0; i < sizeof(privateKey); i++) {
    Serial.print(privateKey[i], HEX);
    Serial.print(" ");
  }
  Serial.println("");

  // Public Key
  Serial.print("Public Key: ");
  for (int i = 0; i < sizeof(nodePubKey); i++) {
    Serial.print("0x");
    Serial.print(nodePubKey[i], HEX);
    Serial.print(", ");
  }
    Serial.println("");


  //byte testPub[] = {0x5f, 0xab, 0x7d, 0x6c, 0x4e, 0x9d, 0xb8, 0xd7, 0x6b, 0x8c, 0xb, 0x40, 0xc0, 0xa8, 0xec, 0x39, 0x7, 0x77, 0xd7, 0x69, 0x9e, 0x89, 0x9c, 0x74, 0x1d, 0xe1, 0xb5, 0x77, 0x9a, 0x36, 0x6c, 0x40, 0x14, 0xf3, 0x99, 0xae, 0xf2, 0x4c, 0x23, 0x4a, 0xa3, 0x61, 0xff, 0xd8, 0xbe, 0xc1, 0xf3, 0x91, 0x71, 0xa4, 0xca, 0x4a, 0x1c, 0xf, 0x73, 0x93, 0xd9, 0xaa, 0x4e, 0x58, 0x43, 0xeb, 0x51, 0x65 };
  //uECC_shared_secret(testPub, privateKey, sharedKey, uECC_secp256r1());
  
  WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true) {}
  }

  String fv = WiFi.firmwareVersion();

  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware...");
  }


  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);

    status = WiFi.begin(ssid, pass);
    printStatus(status);
    delay(500);
  }

  Serial.println("Connected to wifi");
  Serial.println(WiFi.status());

  Serial.println("\nStarting connection to server...");
  Udp.begin(localPort);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void updateRTC(byte year, byte month, byte day, byte hour, byte minute, byte second) {
  uint32_t t;
  t |= (year << 26) | (month << 22) | (day << 17) | (hour << 12) | (minute << 6) | (second);
  RTC->MODE2.CLOCK.reg = t;
  while(RTC->MODE2.SYNCBUSY.bit.CLOCK);
  delay(100);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void hashFunc(Hash *hash, const uint8_t * data, uint8_t *hashOut, size_t dataSize) {
  hash->reset();
  hash->update(data,dataSize);
  hash->finalize(hashOut,HASH_SIZE);
  // Hopefully this works
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void printStatus(int status) {
  switch(status) {
      case WL_NO_SHIELD:
        Serial.print(status);
        Serial.println(": No WiFi device present");
        break;
      case WL_NO_SSID_AVAIL:
        Serial.print(status);
        Serial.println(": No SSID Available");
      break;
      case WL_CONNECT_FAILED:
        Serial.print(status);
        Serial.println(": Connection failed for all attempts");
      break;
      case WL_DISCONNECTED:
        Serial.print(status);
        Serial.println(": Disconnected from Network");
      break;
      case WL_CONNECTION_LOST:
        Serial.print(status);
        Serial.println(": Connection Lost");
      break;
      case WL_IDLE_STATUS:
        Serial.print(status);
        Serial.println(": Idle Status...");
      break;
      case WL_AP_FAILED:
        Serial.print(status);
        Serial.println(": AP Failed");
      break;
      case WL_AP_CONNECTED:
        Serial.print(status);
        Serial.println(": AP Connected");
      break;
      default:
        Serial.print(status);
        Serial.println(": Unknown");
       break;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void makeDeviceID(char * dest) {
  char newID[8];
  for (int i = 0; i < 8; i++) {
    char temp = (char)KISS ^ (char)SERIAL_NUMBER[i];
    temp %= 36;
    if (temp >= 10) {
      temp -= 10;
      temp += 0x41;
    }
    else {
      temp %= 10;
      temp += 0x30;
    }
    dest[i] = (char)temp;
  }
  
  Serial.print("\n-------\n");
  return;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void authenticate() {
  int authSize;
  byte packetBuffer[PACKET_SIZE];
  bool packetReceived = false;
  memset(packetBuffer, 0, PACKET_SIZE);
  strcpy((char *)&packetBuffer, WiFi.SSID());

  
  delay(1000);

  while (!packetReceived) {
    if ((authSize = Udp.parsePacket()) != 0) {
      serverIP = Udp.remoteIP();
      Udp.read(packetBuffer, authSize);
      packetReceived = true;
    }
    delay(500);
    Serial.print("Heartbeat: ");
    Serial.println(WiFi.localIP());
  }

  // Check packet size:
  if (authSize == AUTH_EXACT_SIZE) {
    Serial.println("packetSize is of the correct length...");
  }
  else {
    Serial.println("Error: Incoming authorization packet not of correct size");
    return;
  }

  // Check packet integrity:
  //Serial.println((char *)UniqueIdentifier);
  //Serial.println((char *)packetBuffer);
  if (memcmp(UniqueIdentifier, packetBuffer, sizeof(UniqueIdentifier) - 1)) {
    Serial.print("Unique Identifier incorrect, size:  ");
    Serial.println(sizeof(UniqueIdentifier)-1);
    return;
  }
  else {
    for (int i = 0; i < sizeof(UniqueIdentifier) - 1; i++) {
      Serial.print(packetBuffer[i], HEX);
    }
    Serial.println();
    Serial.print("Unique Identifier correct, size:  ");
    Serial.println(sizeof(UniqueIdentifier));
  }

  // Save Public IP
  memcpy(publicKey, &packetBuffer[sizeof(UniqueIdentifier) - 1], PUBKEY_SIZE);
  Serial.print("Public Key: 0x");
  for (int i = 0; i < sizeof(publicKey); i++) {
    Serial.print(publicKey[i], HEX);
  }
  Serial.println("");

  // Double check w/ XOR-checksum system
  byte checkSum[16];
  memset(checkSum, 0, sizeof(checkSum));

  for (int i = 0; i < sizeof(UniqueIdentifier) - 1; i++)
  {
    checkSum[i & 15] ^= packetBuffer[i];
  }
  for (int i = 0; i < sizeof(publicKey); i++)
  {
    checkSum[i & 15] ^= publicKey[i];
  }

  if (memcmp(checkSum, &packetBuffer[sizeof(UniqueIdentifier) - 1 + sizeof(publicKey)], sizeof(checkSum))) {
    Serial.print("Checksum of : ");
    for (int i = 0; i < sizeof(checkSum); i++) {
      Serial.print(checkSum[i], HEX);
    }
    Serial.println(" not matching:");
    for (int i = 0; i < sizeof(checkSum); i++) {
      Serial.print(packetBuffer[sizeof(UniqueIdentifier) - 1 + sizeof(publicKey) + i], HEX);
    }
    while(true);
  }
  else {
    Serial.println("Checksum complete...");
  }

  memcpy(IV, &packetBuffer[sizeof(UniqueIdentifier) - 1 + sizeof(publicKey) + sizeof(checkSum)], sizeof(IV));
  Serial.print("IV Received: ");
  for (int i = 0; i < sizeof(IV); i++) {
    Serial.print(IV[i], HEX);
  }
  Serial.print("\n");

  // Create shared key:
  uint8_t hashOut[HASH_SIZE];
  uECC_shared_secret(publicKey, privateKey, sharedKey, uECC_secp256r1());
  hashFunc(&hash,sharedKey,hashOut,sizeof(sharedKey));
  memcpy(sharedKey,hashOut,sizeof(hashOut));
  aes_init(sharedKey, sizeof(sharedKey));
  Serial.print("Shared Key: ");
  for(int i = 0; i < sizeof(sharedKey); i++) {
    Serial.print(sharedKey[i],HEX);
    Serial.print(" ");
  }
    Serial.println();
  // Begin UDP transmission back of information and open TCP socket to server from serverKey
  // Copy publicKey in
  byte plaintextIn[sizeof(deviceID) + sizeof(sensorType) + sizeof(UniqueIdentifier) - 1 + sizeof(counter)];
  byte returnTransmission[sizeof(nodePubKey) + (sizeof(plaintextIn)/16+1)*16];
  Serial.print("Size of transmission is ");
  Serial.print(sizeof(returnTransmission));
  memcpy(returnTransmission, nodePubKey, sizeof(nodePubKey));
  // Encrypt plaintext and append to returnTransmission

  // Device ID
  memcpy(plaintextIn, deviceID, sizeof(deviceID));
  Serial.print("plainText ASCII: ");
  for(int i = 0; i < sizeof(deviceID); i++) {
    if ((char)deviceID[i] != 0) {
      Serial.print((char)deviceID[i]);
    }
    else {
      Serial.print("\0");
    }
  }
  
  // Sensor Type (ASCII)
  memcpy(&plaintextIn[sizeof(deviceID)], sensorType, sizeof(sensorType));
  Serial.print(" ");
  for(int i = 0; i < sizeof(sensorType); i++) {
    if ((char)sensorType[i] != 0) {
      Serial.print((char)sensorType[i]);
    }
    else {
      Serial.print("\0");
    }
  }
  
  // Unique Identifier
  memcpy(&plaintextIn[sizeof(deviceID) + sizeof(sensorType)], UniqueIdentifier, sizeof(UniqueIdentifier) - 1);
  Serial.print(" ");
  for(int i = 0; i < sizeof(UniqueIdentifier)-1; i++) {
    Serial.print((char)UniqueIdentifier[i]);
  }
  
  // Counter init value
  memcpy(&plaintextIn[sizeof(deviceID)+sizeof(sensorType)+sizeof(UniqueIdentifier)-1],&counter,4);
  Serial.print(" Counter: ");
  Serial.print(counter);
  Serial.println("");
  
  int returnSize = AES_Encrypt((uint8_t *)plaintextIn,(uint8_t *)&returnTransmission[sizeof(nodePubKey)],sizeof(plaintextIn));
  Serial.print("Plaintext:");
  for (int i = 0; i < sizeof(plaintextIn); i++) {
    Serial.print(" ");
    Serial.print(plaintextIn[i],HEX);
  }
  Serial.println();
  Serial.print("Cipher:");
  for (int i = 0; i < returnSize; i++) {
    Serial.print(" ");
    Serial.print(returnTransmission[sizeof(nodePubKey)+i],HEX);
  }
  Serial.println();
  Udp.beginPacket(serverIP,outwardPort);
  Udp.write(returnTransmission,sizeof(returnTransmission));
  Udp.endPacket();

  packetReceived = false;
  while (!packetReceived) {
    if ((authSize = Udp.parsePacket()) != 0) {
      serverIP = Udp.remoteIP();
      Udp.read(packetBuffer, authSize);
      packetReceived = true;
    }
    delay(500);
    Serial.print("Waiting for TCP_GO: ");
    Serial.println(WiFi.localIP());
  }

  bool connected = false;
  // Create TCP Thing
  while(connected == false) {
    connected = Tcp.connect(serverIP,20003);
    Serial.println("Let me in");
    delay(300);
  }
  
  byte cipher[16];
  byte a[16];
    while(!Tcp.available()) {}
      for (int i = 0; i < sizeof(cipher); i++) {
        cipher[i] = Tcp.read();
      }
  //Tcp.stop();
  AES_Decrypt(cipher,a,sizeof(cipher));
  updateRTC(a[2],a[1],a[0],a[3],a[4],a[5]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef OUTPUTNODE
bool dataRefreshReceived(WiFiClient cl) {
  byte buffer[128];
  int bytesReceived = cl.available() & 127;

  if (!(bytesReceived)) {
    return false;
  }
  // Verify that the ping is correct:
  // Receives: Encrypted(Unique Identifier + "Output Data:" + responseData + counter + timestamp)
  for(int i = 0; i < bytesReceived; i++) {
      buffer[i] = (byte)cl.read();
  }
  
  Serial.print("before decrypt: ");
  for (int i = 0; i < bytesReceived; i++) {
          Serial.print(buffer[i],HEX);
          Serial.print(" ");
         }
         Serial.println();
         Serial.print("Shared Key: ");
         for (int i = 0; i < sizeof(sharedKey); i++) {
          Serial.print(sharedKey[i],HEX);
          Serial.print(" ");
         }
         Serial.println();
         Serial.print("IV: ");
         for (int i = 0; i < sizeof(IV); i++) {
          Serial.print(IV[i],HEX);
          Serial.print(" ");
         }
         Serial.println();
  int pingSize = AES_Decrypt((uint8_t *)buffer, (uint8_t *)buffer, bytesReceived);
  byte testTime[6];
  
  if(pingSize != sizeof(UniqueIdentifier)-1+sizeof(dataOutput)-1+sizeof(counter)+sizeof(responseData)+sizeof(testTime)) {
    Serial.println("Incorrect size.");
    return false;
  }
  
  // Compare: Counter, Identifier, & dataOutput
  if (memcmp(UniqueIdentifier,buffer,sizeof(UniqueIdentifier)-1) ||
     memcmp(dataOutput,&buffer[sizeof(UniqueIdentifier)-1],sizeof(dataOutput)-1) ||
      (*((uint32_t *)&buffer[sizeof(UniqueIdentifier)+sizeof(dataOutput)-2+sizeof(responseData)]) != counter)) {
         Serial.println("Invalid dataRequest ping.");
         Serial.println();
         Serial.print("Counter: ");
         Serial.println(counter);
         Serial.print("Received Counter: ");
         Serial.println(*((uint32_t *)&buffer[sizeof(UniqueIdentifier)+sizeof(dataOutput)-2+sizeof(responseData)]));
         return false;
  }

  // Compare the time stamp... if it is greater than 5 seconds, scrap it. If it is greater than 2, but less than 5 seconds, replace it.
  memcpy(testTime,&buffer[sizeof(UniqueIdentifier)-1+sizeof(dataOutput)-1+sizeof(counter)+sizeof(responseData)],sizeof(testTime));
  tm tme;
  tme.tm_year = 116 + testTime[2];
  tme.tm_mon = testTime[1] - 1;
  tme.tm_mday = testTime[0];
  tme.tm_hour = testTime[3];
  tme.tm_min = testTime[4];
  tme.tm_sec = testTime[5];
  time_t unix  = mktime((tm *)&tme);

  uint32_t clockTime = RTC->MODE2.CLOCK.reg;
  tme.tm_year = 116 + (clockTime >> 26 & 0x1F); // - 1?
  tme.tm_mon = (clockTime >> 22 & 0xF) - 1;
  tme.tm_mday = (clockTime >> 17 & 0x1F);
  tme.tm_hour = (clockTime >> 12 & 0x1F);
  tme.tm_min = (clockTime >> 6 & 0x3F);
  tme.tm_sec = (clockTime & 0x3F);
  time_t internalUnix = mktime((tm *)&tme);
  int diff = (internalUnix > unix) ? ((int)internalUnix - (int)unix) : ((int)unix - (int)internalUnix);

  Serial.print("Difference between clocks: ");
  Serial.println(diff);

  if (diff > 5) {
    // Reject monke
    Serial.println("Error: Timestamp deviates too far from held value.");
    return false;
  }
  
  else if (diff > 3) {
    Serial.println("Greater than 3 seconds difference, refreshing clock to last time stamp.");
    updateRTC(testTime[2],testTime[1],testTime[0],testTime[3],testTime[4],testTime[5]);
  }
  
  else {
    Serial.print("Time difference only ");
    Serial.print(diff);
    Serial.println(" deviated. No refresh needed.");
  }

  dataBuffer = (*((responseData*)&buffer[sizeof(UniqueIdentifier)+sizeof(dataOutput)-2]));  // (*((uint32_t *)&buffer[sizeof(UniqueIdentifier)+sizeof(dataOutput)-2+sizeof(responseData)])
  // 2 1 0 3 4 5
  // void updateRTC(byte year, byte month, byte day, byte hour, byte minute, byte second)
  // Convert to unix time, from 2016 because why not:
  // TODO: parse the incoming refresh data
  // TODO: Send back confirmation of sent data
  if (!cl.connected()) {
    Serial.println("Not connected.");
    return false;
  }

  clockTime = RTC->MODE2.CLOCK.reg;
  
  // Send response: Response should be -- UniqueIdentifier + "DataReceived" + counter + timestamp
  // Can clear buffer as it's not needed anymore:
  byte sendbuffer[sizeof(UniqueIdentifier)+sizeof(dataReceived)+sizeof(uint32_t)+sizeof(uint32_t)-2];
  memset(sendbuffer,0,sizeof(sendbuffer));
  memcpy(sendbuffer,UniqueIdentifier,sizeof(UniqueIdentifier)-1); // Copy uniqueidentifier
  memcpy(&sendbuffer[sizeof(UniqueIdentifier)-1],dataReceived,sizeof(dataReceived)-1); // copy datareceived
  memcpy(&sendbuffer[sizeof(UniqueIdentifier)+sizeof(dataReceived)-2],(byte *)&counter,sizeof(uint32_t)); // copy counter
  memcpy(&sendbuffer[sizeof(UniqueIdentifier)+sizeof(dataReceived)-2+sizeof(uint32_t)],(byte *)&clockTime,sizeof(uint32_t)); // copy timestamp

  byte writeBuffer[sizeof(sendbuffer)+16];
  int sizey = AES_Encrypt(sendbuffer,writeBuffer,sizeof(sendbuffer));
  Serial.print(cl.write(writeBuffer,sizey));
  Serial.println(" Byte Sized Pieces Sent.");
  
  counter++;
  onDataReceived(dataBuffer);
  
  return true;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t passedMinutes = 0;
uint8_t lastMinute = 0;

#ifndef OUTPUTNODE
bool dataRequestReceived(WiFiClient cl) {
  byte buffer[128];
  int bytesReceived = cl.available() & 127;
  // If the dataRequest was not received
  if (!(bytesReceived)) {
    return false;
  }
  // Verify that the ping is correct:
  // Ping should contain the following: Encrypted(UniqueIdentifier + "dataRequest" + counter)
  for(int i = 0; i < bytesReceived; i++) {
    buffer[i] = (byte)cl.read();
  }
  Serial.print("before decrypt: ");
  for (int i = 0; i < bytesReceived; i++) {
          Serial.print(buffer[i],HEX);
          Serial.print(" ");
         }
         Serial.println();
         Serial.print("Shared Key: ");
         for (int i = 0; i < sizeof(sharedKey); i++) {
          Serial.print(sharedKey[i],HEX);
          Serial.print(" ");
         }
         Serial.println();
         Serial.print("IV: ");
         for (int i = 0; i < sizeof(IV); i++) {
          Serial.print(IV[i],HEX);
          Serial.print(" ");
         }
         Serial.println();
  int pingSize = AES_Decrypt((uint8_t *)buffer, (uint8_t *)buffer, bytesReceived);
  
  if(pingSize != sizeof(UniqueIdentifier)-1+sizeof(dataRequest)-1+6+sizeof(counter)) {
    Serial.println("Incorrect size.");
    return false;
  }
  
  if (memcmp(UniqueIdentifier,buffer,sizeof(UniqueIdentifier)-1) ||
     memcmp(dataRequest,&buffer[sizeof(UniqueIdentifier)-1],sizeof(dataRequest)-1) ||
      (*((uint32_t *)&buffer[sizeof(UniqueIdentifier)+sizeof(dataRequest)-2]) != counter)) {
         Serial.println("Invalid dataRequest ping.");
         Serial.println();
         Serial.print("Counter: ");
         Serial.println(counter);
         Serial.print("Received Counter: ");
         Serial.println(*((uint32_t *)&buffer[sizeof(UniqueIdentifier)+sizeof(dataRequest)-2]));
         return false;
  }

  byte testTime[6];
  memcpy(testTime,&buffer[sizeof(UniqueIdentifier)-1+sizeof(dataRequest)-1+sizeof(counter)],sizeof(testTime));
  
  // Assumed that dataRequest was found:
  responseData toSend = onDataRequest();
  uint32_t clockTime = RTC->MODE2.CLOCK.reg;
  #if DEBUG == 1
    Serial.print("Time of Request: ");
    Serial.print((clockTime >> 26 & 0x1F)+2016);
    Serial.print("/");
    Serial.print((clockTime >> 22 & 0xF));
    Serial.print("/");
    Serial.print((clockTime >> 17 & 0x1F));
    Serial.print(" ");
    Serial.print(clockTime >> 12 & 0x1F);
    Serial.print(":");
    Serial.print(clockTime >> 6 & 0x3F);
    Serial.print(":");
    Serial.println(clockTime & 0x3F);
  #endif
  
  if (clockTime >> 6 & 0x3F != lastMinute) passedMinutes++;
  
  if (passedMinutes >= 5) {
    Serial.print("Updated RTC to: ");
    for (int i = 0; i < sizeof(testTime); i++) {
      Serial.print(testTime[i]);
      Serial.print(" ");
    }
    passedMinutes = 0;
    Serial.println();
    updateRTC(testTime[2],testTime[1],testTime[0],testTime[3],testTime[4],testTime[5]);
  }
  else {
    Serial.print("Only ");
    Serial.print(passedMinutes);
    Serial.println(" since last clock refresh... did not update.");
  }
  lastMinute = clockTime >> 6 & 0x3F;
  byte toSendBuff[sizeof(dataResponse)+sizeof(toSend)-1+sizeof(counter)+4];
  memcpy(toSendBuff,dataResponse,sizeof(dataResponse)-1); // "DataResponse:"
  memcpy(&toSendBuff[sizeof(dataResponse)-1],&toSend,sizeof(toSend));
  *((uint32_t *)&toSendBuff[sizeof(dataResponse)-1+sizeof(toSend)]) = counter;
  *((uint32_t *)&toSendBuff[sizeof(dataResponse)-1+sizeof(toSend)+sizeof(counter)]) = clockTime;
  if (!cl.connected()) {
    Serial.println("Not connected.");
  }

  byte writeBuffer[sizeof(toSendBuff)+16];
 
  int sizey = AES_Encrypt(toSendBuff,writeBuffer,sizeof(toSendBuff));
  
  Serial.print(cl.write(writeBuffer,sizey));
  Serial.println(" Byte Sized Pieces Sent.");
  counter++;
  return true;
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void clkInit() {
  delay(1000);
  MCLK->APBAMASK.reg |= MCLK_APBAMASK_RTC;
  OSC32KCTRL->RTCCTRL.reg = 0;  // 5 32khz crystal  1 ulp oscillator
  RTC->MODE2.CTRLA.reg &= ~RTC_MODE2_CTRLA_ENABLE;  // disable
  while (RTC->MODE2.SYNCBUSY.bit.ENABLE); // sync
  RTC->MODE2.CTRLA.reg |= RTC_MODE2_CTRLA_SWRST; // software reset
  while (RTC->MODE2.SYNCBUSY.bit.SWRST); // sync
  
  RTC->MODE2.CTRLA.reg |= RTC_MODE2_CTRLA_MODE(0x2);
  RTC->MODE2.CTRLA.reg &= ~RTC_MODE2_CTRLA_CLKREP;
  RTC->MODE2.CTRLA.reg |=  RTC_MODE2_CTRLA_CLOCKSYNC | RTC_MODE2_CTRLA_PRESCALER(0xB) | RTC_MODE2_CTRLA_ENABLE; // enable
  while (RTC->MODE2.SYNCBUSY.bit.ENABLE); // sync
  //RTC->MODE2.FREQCORR.bit.SIGN |= 0;
  //RTC->MODE2.FREQCORR.bit.VALUE |= 1;
  //while(RTC->MODE2.SYNCBUSY.bit.FREQCORR);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void aes_init(uint8_t * AESKey, size_t AESKeySize) {
  MCLK->APBCMASK.reg |= MCLK_APBCMASK_AES;
  set_aes_key(AESKey, AESKeySize);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int set_aes_key(const uint8_t *key, size_t keySize) {
  if (keySize != 32) {
    Serial.print("Invalid key size! Please provide a key of 256bits...");
    return 1;
  }
  memcpy((uint8_t *)&REG_AES_KEYWORD0, key, keySize);
  return 0;
}

int getRNGValues(uint8_t * dest, unsigned size) {
  for (int i = 0; i < size; i++) {
    dest[i] = (int8_t)getTRNGValue();
  }
  return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns length of cipherout
uint32_t AES_Encrypt(const uint8_t* plaintext, uint8_t * cipherout, size_t sizeT) {
  //byte buffer[1024];
  //memset(buffer,0,sizeT+16);
  memcpy(cipherout, plaintext, sizeT);

  int paddingSize = 16 - sizeT % 16;
  if (paddingSize == 0) {
    paddingSize = 16;
  }
  
  for (int i = 0; i < paddingSize; i++) {
    cipherout[sizeT + i] = paddingSize;
  }
  
  aes_cbc_encrypt_256b(cipherout, cipherout, sizeT + paddingSize, IV);

  
  return sizeT + paddingSize;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns length of cipherout
uint32_t AES_Decrypt(const uint8_t* ciphertext, uint8_t * output, size_t size) {
  //byte buffer[1024];
  //memset(buffer,0,sizeT+16);
  //memcpy(cipherout, plaintext, sizeT);
  if (size & 15 || size == 0) {
    Serial.println("Invalid Ciphertext Size!");
    return 0;
  }
  aes_cbc_decrypt_256b(ciphertext, output, size, IV);
  return size - (output[size-1]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void aes_cbc_encrypt_256b(const uint8_t *plaintext, uint8_t *ciphertext, size_t sizeT, const uint8_t iv[16]) {
  int i;
  // Steps found in SAMD51 manual: https://ww1.microchip.com/downloads/en/DeviceDoc/SAM_D5x_E5x_Family_Data_Sheet_DS60001507G.pdf
  // Further assistance provided by:  https://github.com/manitou48/samd51/blob/master/aes.ino

  memcpy((uint8_t *)&REG_AES_INTVECTV0, iv, 16);
  memcpy((uint8_t *)&REG_AES_KEYWORD0, sharedKey, sizeof(sharedKey));
  REG_AES_CTRLA = 0;
  REG_AES_CTRLA = AES_CTRLA_AESMODE_CBC | AES_CTRLA_CIPHER_ENC | AES_CTRLA_ENABLE | AES_CTRLA_KEYSIZE_256BIT;
  REG_AES_CTRLB = AES_CTRLB_NEWMSG;
  
  //PRREG(REG_AES_CTRLA);

  uint32_t *wp = (uint32_t *) plaintext;
  uint32_t *wc = (uint32_t *) ciphertext;

  // Ensure size is a multiple of 16
  if (sizeT % 16) {
    Serial.println("Invalid size of plaintext/ciphertext...");
    return;
  }

  int word = 0;
  while (sizeT > 0) {
    for ( i = 0; i < 4; i++) REG_AES_INDATA = wp[i + word];
    REG_AES_CTRLB |= AES_CTRLB_START;
    while ((REG_AES_INTFLAG & AES_INTENCLR_ENCCMP) == 0);
    for (i = 0; i < 4; i++) wc[i + word] = REG_AES_INDATA;
    sizeT -= 16;
    word += 4;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void aes_cbc_decrypt_256b(const uint8_t *ciphertext, uint8_t *plaintext, size_t sizeT, const uint8_t iv[16]) {
  int i;
  uint32_t *wp = (uint32_t *) plaintext;
  uint32_t *wc = (uint32_t *) ciphertext;
  int word = 0;
  // Steps found in SAMD51 manual: https://ww1.microchip.com/downloads/en/DeviceDoc/SAM_D5x_E5x_Family_Data_Sheet_DS60001507G.pdf
  // Further assistance provided by:  https://github.com/manitou48/samd51/blob/master/aes.ino

  // Ensure size is a multiple of 16
  if (sizeT & 15) {
    Serial.println("Invalid size of plaintext/ciphertext...");
    return;
  }
    
  memcpy((uint8_t *)&REG_AES_INTVECTV0, iv, 16);
  memcpy((uint8_t *)&REG_AES_KEYWORD0, sharedKey, sizeof(sharedKey));
  REG_AES_CTRLA = 0;
  REG_AES_CTRLA = AES_CTRLA_AESMODE_CBC | AES_CTRLA_CIPHER_DEC | AES_CTRLA_ENABLE | AES_CTRLA_KEYSIZE_256BIT;
  REG_AES_CTRLB |= AES_CTRLB_NEWMSG;

  while (sizeT > 0) {
    for (i = 0;  i < 4; i++) REG_AES_INDATA = wc[i + word];

    REG_AES_CTRLB |= AES_CTRLB_START;
    while ((REG_AES_INTFLAG & AES_INTENCLR_ENCCMP) == 0);  // wait for done
    for (i = 0;  i < 4; i++) wp[i + word] = REG_AES_INDATA;
    sizeT -= 16;
    word += 4;
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
