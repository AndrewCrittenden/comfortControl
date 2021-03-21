#ifndef NODECOMFORT_H
#define NODECOMFORT_H

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

const string dataResponse = "Response:";
const string dataReceived = "DataReceived";
const string UniqueIdentifier = "COMFORTController V0.1";

enum class nodeTypeDef {
    Indoor,
    Outdoor,
    Test,
    Rel_Humidity,
    Globe,
    Occupancy,
    Output,
    Error
};

class nodeCOMFORT
{
public:
    nodeCOMFORT(string ID_new, uint32_t IP_new, nodeTypeDef type_new, uint32_t new_counter, SecByteBlock sharedKey);
    string NodeTypetoString();
    static nodeTypeDef stringToNodeType(string input);

    string ID;
    uint32_t IP;
    nodeTypeDef type;
    vector<uint8_t> data;
    bool connected;
    uint32_t counter;
    int connectionFD;
    sockaddr_in TCPConnection;
    SecByteBlock shared;

private:

};

#endif // NODECOMFORT_H
