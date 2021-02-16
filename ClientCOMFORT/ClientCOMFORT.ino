#include <types.h>
#include <uECC.h>
#include <uECC_vli.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <Crypto.h>
#include <SHA256.h>

#define HASH_SIZE 32
#define BLOCK_SIZE 64
#define DEBUG 0
#define SERIAL_NUMBER ((uint8_t *)0x008061FC)


#define SPIWIFI       SPI  // The SPI port
#define SPIWIFI_SS    9   // Chip select pin
#define ESP32_RESETN  5   // Reset pin
#define SPIWIFI_ACK   7   // a.k.a BUSY or READY pin
#define ESP32_GPIO0   -1

/*
#define SPIWIFI       SPI  // The SPI port
#define SPIWIFI_SS    13   // Chip select pin
#define ESP32_RESETN  12   // Reset pin
#define SPIWIFI_ACK   11   // a.k.a BUSY or READY pin
#define ESP32_GPIO0   -1
*/

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
/*
// Configure the pins used for the ESP32 connection
#if defined(ADAFRUIT_FEATHER_M4_EXPRESS) || \
  defined(ADAFRUIT_FEATHER_M0_EXPRESS) || \
  defined(ARDUINO_AVR_FEATHER32U4) || \
  defined(ARDUINO_NRF52840_FEATHER) || \
  defined(ADAFRUIT_ITSYBITSY_M0) || \
  defined(ADAFRUIT_ITSYBITSY_M4_EXPRESS) || \
  defined(ARDUINO_AVR_ITSYBITSY32U4_3V) || \
  defined(ARDUINO_NRF52_ITSYBITSY)
// Configure the pins used for the ESP32 connection
#define SPIWIFI       SPI  // The SPI port
#define SPIWIFI_SS    13   // Chip select pin
#define ESP32_RESETN  12   // Reset pin
#define SPIWIFI_ACK   11   // a.k.a BUSY or READY pin
#define ESP32_GPIO0   -1
#elif defined(ARDUINO_AVR_FEATHER328P)
#define SPIWIFI       SPI  // The SPI port
#define SPIWIFI_SS     4   // Chip select pin
#define ESP32_RESETN   3   // Reset pin
#define SPIWIFI_ACK    2   // a.k.a BUSY or READY pin
#define ESP32_GPIO0   -1
#elif defined(TEENSYDUINO)
#define SPIWIFI       SPI  // The SPI port
#define SPIWIFI_SS     5   // Chip select pin
#define ESP32_RESETN   6   // Reset pin
#define SPIWIFI_ACK    9   // a.k.a BUSY or READY pin
#define ESP32_GPIO0   -1
#elif defined(ARDUINO_NRF52832_FEATHER)
#define SPIWIFI       SPI  // The SPI port
#define SPIWIFI_SS    16   // Chip select pin
#define ESP32_RESETN  15   // Reset pin
#define SPIWIFI_ACK    7   // a.k.a BUSY or READY pin
#define ESP32_GPIO0   -1
#elif !defined(SPIWIFI_SS)   // if the wifi definition isnt in the board variant
// Don't change the names of these #define's! they match the variant ones
#define SPIWIFI       SPI
#define SPIWIFI_SS    10   // Chip select pin
#define SPIWIFI_ACK    7   // a.k.a BUSY or READY pin
#define ESP32_RESETN   5   // Reset pin
#define ESP32_GPIO0   -1   // Not connected
#endif
*/
#define UL unsigned long
#define znew  ((z=36969*(z&65535)+(z>>16))<<16)
#define wnew  ((w=18000*(w&65535)+(w>>16))&65535)
#define MWC   (znew+wnew)
#define SHR3  (jsr=(jsr=(jsr=jsr^(jsr<<17))^(jsr>>13))^(jsr<<5))
#define CONG  (jcong=69069*jcong+1234567)
#define KISS  ((MWC^CONG)+SHR3)

#define trand (TRNG->DATA.reg)

#include "arduino_secrets.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
const byte UniqueIdentifier[] = "COMFORTController V0.1";
const int PACKET_SIZE = 128;
const int AUTH_EXACT_SIZE = 118;
const int PUBKEY_SIZE = 64;
const int RETURN_PACKET_SIZE = 0;
uint32_t seed;
uint32_t counter;

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

WiFiUDP Udp;
SHA256 hash;

void setup() {
  char Stringy[] = "Temperature";
  comfortWiFiSetup(Stringy, sizeof(Stringy)-1);
}

void loop() {
  authenticate();
}

void getSerialNumber(uint8_t * dest, size_t getLength) {
  if (getLength > 16) { return; }
  for (int i = 0; i < getLength; i++) {
    dest[i] = 3;
  }
}

void init_TRNG() {
  MCLK->APBCMASK.reg |= MCLK_APBCMASK_TRNG;
  TRNG->CTRLA.reg |= TRNG_CTRLA_ENABLE;
  
  while((TRNG->INTFLAG.reg & TRNG_INTFLAG_DATARDY) == 0) {
      //seed = trand;
    }
  
}

int getTRNGValue() {
  if (seed != trand) {
    srand(trand);
    seed = trand;
  }
  return rand();
}

// DO NOT RECOMMEND USING THIS ONE, PLEASE SPECIFY YOUR DEVICEID!!!
void comfortWiFiSetup(const char * inputType, size_t sensorTypeLength) {
  char newID[8];
  makeDeviceID((char *)newID);
  comfortWiFiSetup(newID, inputType, sensorTypeLength);
}

void comfortWiFiSetup(const char * D_ID, const char * inputType, size_t sensorTypeLength) {
  memcpy(deviceID, D_ID, sizeof(deviceID));
  memset(sensorType,0,sizeof(sensorType));
  memcpy(sensorType,inputType,sensorTypeLength);

  uint32_t * privateKeyInitializer = (uint32_t *)privateKey;
  for (int i = 0; i < 8; i++) {
    privateKeyInitializer[i] = getTRNGValue();
  }

  // If not serial.
  if (!Serial) {
    Serial.begin(9600);
  }

  #if (DEBUG == 1)
    while (!Serial) {}
    Serial.println();
  #endif
  
  for(int i = 0; i < 8; i++) {
    Serial.print((char)deviceID[i]);
    Serial.print(" ");
  }
  Serial.println("");
  
  init_TRNG();
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
    Serial.print(nodePubKey[i], HEX);
    Serial.print(" ");
  }
    Serial.println("");
  
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

    delay(500);
  }

  Serial.println("Connected to wifi");
  Serial.println(WiFi.status());

  Serial.println("\nStarting connection to server...");
  Udp.begin(localPort);
}

void hashFunc(Hash *hash, const uint8_t * data, uint8_t *hashOut, size_t dataSize) {
  hash->reset();
  hash->update(data,dataSize);
  hash->finalize(hashOut,HASH_SIZE);
  // Hopefully this works
}

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
    Serial.print("Heartbeat is cool: ");
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

  Udp.beginPacket(serverIP,outwardPort);
  Udp.write(returnTransmission,sizeof(returnTransmission));
  Udp.endPacket();
  while(true);
  return;
  //
}

void aes_init(uint8_t * AESKey, size_t AESKeySize) {
  MCLK->APBCMASK.reg |= MCLK_APBCMASK_AES;
  set_aes_key(AESKey, AESKeySize);
}

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

void aes_cbc_encrypt_256b(const uint8_t *plaintext, uint8_t *ciphertext, size_t sizeT, const uint8_t iv[16]) {
  int i;
  // Steps found in SAMD51 manual: https://ww1.microchip.com/downloads/en/DeviceDoc/SAM_D5x_E5x_Family_Data_Sheet_DS60001507G.pdf
  // Further assistance provided by:  https://github.com/manitou48/samd51/blob/master/aes.ino

  memcpy((uint8_t *)&REG_AES_INTVECTV0, iv, 16);

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

void aes_cbc_decrypt_256b(const uint8_t *ciphertext, uint8_t *plaintext, size_t sizeT, const uint8_t iv[16]) {
  int i;
  uint32_t *wp = (uint32_t *) plaintext;
  uint32_t *wc = (uint32_t *) ciphertext;
  int word = 0;
  // Steps found in SAMD51 manual: https://ww1.microchip.com/downloads/en/DeviceDoc/SAM_D5x_E5x_Family_Data_Sheet_DS60001507G.pdf
  // Further assistance provided by:  https://github.com/manitou48/samd51/blob/master/aes.ino

  // Ensure size is a multiple of 16
  if (sizeT % 16) {
    Serial.println("Invalid size of plaintext/ciphertext...");
    return;
  }

  memcpy((uint8_t *)&REG_AES_INTVECTV0, iv, 16);
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
