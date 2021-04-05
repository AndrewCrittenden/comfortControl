#ifndef SERVERCOMFORT_H
#define SERVERCOMFORT_H

#include <QObject>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <list>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <ctime>
#include "nodecomfort.h"
#include "TripleBuffer.h"

// Actual Pi includes
#include "./cryptopp850/aes.h"
#include "./cryptopp850/modes.h"
#include "./cryptopp850/filters.h"
#include "./cryptopp850/eccrypto.h"
#include "./cryptopp850/dh.h"
#include "./cryptopp850/dh2.h"
#include "./cryptopp850/asn.h"
#include "./cryptopp850/integer.h"
#include "./cryptopp850/oids.h"
#include "./cryptopp850/osrng.h"
#include "./cryptopp850/secblock.h"
#include "./cryptopp850/sha.h"
#include "./cryptopp850/aes.h"
#include "./cryptopp850/filters.h"
#include "./cryptopp850/hex.h"
#include "./cryptopp850/ccm.h"

using CryptoPP::SecByteBlock;
using CryptoPP::AutoSeededRandomPool;
using CryptoPP::AutoSeededX917RNG;
using namespace std;
using CryptoPP::ECP;
using namespace CryptoPP::ASN1;
using CryptoPP::ECDH;
using CryptoPP::AES;
using CryptoPP::Integer;
using CryptoPP::StreamTransformationFilter;
using CryptoPP::StringSink;
using CryptoPP::AES;
using CryptoPP::CBC_Mode;
using CryptoPP::OID;
using CryptoPP::SHA256;

struct dataIn {
    uint8_t occupancy;
    double indoor;
    double outdoor;
    double globe;
    double relHumidity;
};

struct sensorReady {
    bool occupancy;
    bool indoor;
    bool outdoor;
    bool globe;
    bool relHumidity;
};

struct dataOut {
    double output;
};

class serverCOMFORT : public QObject
{
    Q_OBJECT
public:
    explicit serverCOMFORT(QObject *parent = nullptr);
    ~serverCOMFORT();
    void serverOperation();
    const int timeDuration = 5000;
    const int authenticationTime = 10000;

    void authentication();
    // Requests data from the sensors, blocks for 2 seconds to allow sensor data to come in.
    void dataRequest();
    void sendData();
    void clearNodes();
    void authProcess(uint8_t * receiveBuf, size_t receiveBufSize);
    void printNodeCOMFORT();
    void checkSensorReady();

    // Sensor data public members:
    dataIn inData;
    dataIn inDataBuf;

	TripleBuffer<dataIn> buffIn;
	TripleBuffer<dataOut> buffOut;

    dataOut outData;
    dataOut outDataBuf;
    sensorReady sensorStatus;

    // Socket data and information:
    int udpFDSend;
    int udpFDReceive;
    int tcpFDstart;
    int newtcpFD;
    const int outPort = 20000;
    const int inPort = 20001;
    const int PORTTCP = 20003;

    // List for device nodes
    vector<nodeCOMFORT> nodeList;

    // Flags for server control
    volatile int gatherFrequency;
    volatile bool inData_received = false;
    volatile bool sensorsReady = false;
    volatile bool outData_toSend = false;
    volatile bool active = true;
    volatile bool authenticate = false;
    volatile bool clearNode = false;

Q_SIGNALS:
    void inData_receivedChanged(bool value);
    void statusIndoorChanged(bool value);
    void statusOutdoorChanged(bool value);
    void statusRelHumChanged(bool value);
    void statusGlobeChanged(bool value);
    void statusOccupancyChanged(bool value);

public Q_SLOTS:
    void setInData_received(bool value);
    void setStatusIndoor(bool value);
    void setStatusOutdoor(bool value);
    void setStatusRelHum(bool value);
    void setStatusGlobe(bool value);
    void setStatusOccupancy(bool value);

private:
    // AES Functions
    int Encrypt(uint8_t* plainText, vector<uint8_t> &cipherText, SecByteBlock key, CryptoPP::byte* IV, int size);
    int Decrypt(uint8_t* cipherText, vector<uint8_t>& plainText, SecByteBlock key, CryptoPP::byte* IV, int size);
    SecByteBlock SHA256Hash(SecByteBlock secretKey);
    bool setNetworkInterface();
    void cryptoInit();
    void genLocalKeys();
    bool genSharedKey(SecByteBlock &pKey, SecByteBlock &sharedKey);

    //NetworkInterface Stuff
    sockaddr_in addr;
    sockaddr_in broadcast;
    sockaddr_in constAddr;
    sockaddr_in constTCP;
    sockaddr_in cli_addr;

    // Crypto PEEPEE dependencies
    CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
    AutoSeededRandomPool rng;
    OID CURVE;
    ECDH<ECP>::Domain dh;
    SecByteBlock privKey;
    SecByteBlock pubKey;
};

#endif // SERVERCOMFORT_H
