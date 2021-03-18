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

/*
#include "./cpp fucking thing/sha.h"
#include "./cpp fucking thing/aes.h"
#include "./cpp fucking thing/modes.h"
#include "./cpp fucking thing/filters.h"
#include "./cpp fucking thing/eccrypto.h"
#include "./cpp fucking thing/dh.h"
#include "./cpp fucking thing/dh2.h"
#include "./cpp fucking thing/asn.h"
#include "./cpp fucking thing/integer.h"
#include "./cpp fucking thing/oids.h"
#include "./cpp fucking thing/osrng.h"
#include "./cpp fucking thing/secblock.h"
#include "./cpp fucking thing/aes.h"
#include "./cpp fucking thing/filters.h"
#include "./cpp fucking thing/hex.h"
#include "./cpp fucking thing/ccm.h"
*/

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

class nodeCOMFORT {
public:

	nodeCOMFORT(string ID_new, uint32_t IP_new, nodeTypeDef type_new, uint32_t new_counter, SecByteBlock sharedKey) {
		ID = ID_new;
		IP = IP_new;
		type = type_new;
		shared = sharedKey;
		counter = new_counter;
		connected = false;
		connectionFD = -1;
		switch (type_new) {
			case nodeTypeDef::Indoor:
			case nodeTypeDef::Outdoor:
			case nodeTypeDef::Test:
			case nodeTypeDef::Rel_Humidity:
			case nodeTypeDef::Globe:
			case nodeTypeDef::Output:
				data = vector<uint8_t>(8, 0);
				break;
			case nodeTypeDef::Occupancy:
				data = vector<uint8_t>(1, 0);
				break;
			default:
				data = vector<uint8_t>();
		}
	}

	string NodeTypetoString() {
		switch (type) {
		case nodeTypeDef::Indoor:
			return "Indoor";
			break;
		case nodeTypeDef::Outdoor:
			return "Outdoor";
			break;
		case nodeTypeDef::Test:
			return "Test";
			break;
		case nodeTypeDef::Rel_Humidity:
			return "Rel. Humidity";
			break;
		case nodeTypeDef::Globe:
			return "Globe";
			break;
		case nodeTypeDef::Output:
			return "Output";
			break;
		case nodeTypeDef::Occupancy:
			return "Occupancy";
			break;
		default:
			break;
		};
		return "Error";
	}

	static nodeTypeDef stringToNodeType(string input) {
		if (input.length() == 0) { return nodeTypeDef::Error; }
		switch (input[0]) {
		case 'G':
			if (input == "Globe") {
				return nodeTypeDef::Globe;
			}
			break;
		case 'I':
			if (input == "Indoor") {
				return nodeTypeDef::Indoor;
			}
			break;
		case 'O':
			if (input == "Occupancy") {
				return nodeTypeDef::Occupancy;
			}
			else if (input == "Outdoor") {
				return nodeTypeDef::Outdoor;
			}
			else if (input == "Output") {
				return nodeTypeDef::Output;
			}
			break;
		case 'R':
			if (input == "Rel. Humidity") {
				return nodeTypeDef::Rel_Humidity;
			}
			break;
		case 'T':
			if (input == "Test") {
				return nodeTypeDef::Test;
			}
			break;
		default:
			break;
		}
		return nodeTypeDef::Error;
	}

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

struct dataIn {
	uint8_t occupancy;
	double indoor;
	double outdoor;
	double globe;
	double relHumidity;
};

struct dataOut {
	double output;
};


class serverCOMFORT {

public:
	serverCOMFORT() {
		
	}

	void serverOperation() {
		if (setNetworkInterface() == false) return;
		cryptoInit();
		genLocalKeys();

		cout << "Private Key, size of " << (int)privKey.size() << ": " << endl;
		for (int i = 0; i < (int)privKey.size(); i++)
			cout << std::hex << "0x" << (int)privKey[i] << ", ";
		cout << std::dec << endl;

		cout << "Public Key, size of " << pubKey.size() << ": " << endl;
		for (int i = 0; i < pubKey.size(); i++)
			cout << std::hex << "0x" << (int)pubKey[i] << ", ";
		cout << std::dec << endl;

		cout << "Also public key length " << dh.PublicKeyLength() << endl;
		// Disabling this will end the server operation
		while (active) {

			// Timer before new data refresh
			// Check if authentication bool is pressed:
			chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
			int ms = 0;
			int last_print_time = 0;
			while (ms < timeDuration) {
				//Authentication flag:
				if (authenticate) {
					authentication();
					authenticate = false;
				}

				// Clear Devices flag:
				if (clearNode) {
					clearNodes();
				}

				ms = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count();
				if (last_print_time < (ms/1000)) {
					cout << ms << endl;
					last_print_time = (ms/1000);
				}
			}

			cout << "Timer done." << endl;
			dataRequest();
			authentication();
			printNodeCOMFORT();
		}
		return;
	}
	
	const int timeDuration = 5000;
	const int authenticationTime = 10000;

	void authentication() {
		// Reminder that before transmitting and receiving keys, 0x4 should be appended and removed from the start

		chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
		int ms = 0;
		int last_broadcast_activation = 0;
		int last_print_time = 0;
		int ok = 1;

		// Initialize sockets
		udpFDSend = socket(AF_INET,SOCK_DGRAM,0);
		setsockopt(udpFDSend, SOL_SOCKET, SO_BROADCAST, &ok, sizeof(int));
		if (udpFDSend < 0) {
			cout << "Could not initialize socket for SendFD." << endl;
			return;
		}

		//bzero(&broadcast, sizeof(broadcast));
		//broadcast.sin_addr.s_addr = inet_addr("192.168.1.255");
		cout << "BroadcastADRRDRR: ";
		for (int i = 0; i < sizeof(int); i++) cout << (int)((uint8_t *)&broadcast.sin_addr.s_addr)[i] << ", ";
		cout << endl;
		cout << "IP ADDR: ";
		for (int i = 0; i < sizeof(int); i++) cout << (int)((uint8_t *)&addr.sin_addr.s_addr)[i] << ", ";
		cout << endl;

		broadcast.sin_port = htons(outPort);
		broadcast.sin_family = AF_INET;

		udpFDReceive = socket(AF_INET, SOCK_DGRAM, 0);
		if (udpFDReceive < 0) {
			cout << "Could not create socket." << endl;
			return;
		}
		setsockopt(udpFDReceive, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(int));

		cout << "TCP Socket Descr: " << udpFDReceive << endl;
		bzero(&constAddr, sizeof(constAddr));
		constAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		constAddr.sin_port = htons(inPort);
		constAddr.sin_family = AF_INET;

		if (bind(udpFDReceive, (struct sockaddr *)&constAddr, sizeof(constAddr)) < 0) {
			cout << "Could not bind FDReceive." << endl;
			cout << strerror(errno) << endl;
			return;
		}

		tcpFDstart = socket(AF_INET, SOCK_STREAM, 0);
		if (tcpFDstart < 0) {
			cout << "Could not create socket." << endl;
			return;
		}
		setsockopt(tcpFDstart, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(int));
		fcntl(tcpFDstart, F_SETFL, O_NONBLOCK);
		cout << "TCP Socket Descr: " << tcpFDstart << endl;
		bzero(&constTCP, sizeof(constTCP));
		constTCP.sin_family = AF_INET;
		constTCP.sin_addr.s_addr = htonl(INADDR_ANY);
		constTCP.sin_port = htons(PORTTCP);

		if (bind(tcpFDstart, (sockaddr *)&constTCP, sizeof(constTCP)) < 0) {
			cout << "Could not bind TCP FD." << endl;
			cout << strerror(errno) << endl;
			return;
		}
		listen(tcpFDstart, 6);

		while (ms < authenticationTime) {

			// Broadcast tick:
			if (last_broadcast_activation < (ms/2000)) {

				// broadcast here
				uint8_t checkSum[16];
				memset(checkSum, 0, sizeof(checkSum));
				for (int i = 0; i < UniqueIdentifier.length(); i++)
				{
					checkSum[i & 15] ^= UniqueIdentifier[i];
				}
				for (int i = 1; i < pubKey.SizeInBytes(); i++)
				{
					checkSum[(i-1) & 15] ^= pubKey[i];
				}
				

				CryptoPP::byte broadcastMessage[UniqueIdentifier.length() + pubKey.SizeInBytes()-1 + sizeof(checkSum) + sizeof(iv)];
				memcpy(&broadcastMessage, UniqueIdentifier.c_str(), UniqueIdentifier.length());
				memcpy(&broadcastMessage[UniqueIdentifier.length()], &(pubKey.BytePtr()[1]), pubKey.SizeInBytes()-1);
				memcpy(&broadcastMessage[UniqueIdentifier.length() + pubKey.SizeInBytes() - 1], checkSum, sizeof(checkSum));
				memcpy(&broadcastMessage[sizeof(broadcastMessage) - sizeof(iv)], &iv[0], sizeof(iv));
				cout << "Broadcast size: " << sizeof(broadcastMessage) << endl;

				//cout << "Broadcast Message: ";
				//for (int i = 0; i < sizeof(broadcastMessage); i++) cout << std::hex << "0x" << (int)broadcastMessage[i] << ", ";
				//cout << std::dec << endl;

				// Time to learn how to send this out... yay....
				int sentSize = sendto(udpFDSend, broadcastMessage, sizeof(broadcastMessage), MSG_CONFIRM, (sockaddr *)&broadcast, sizeof(broadcast));
				if (sentSize != sizeof(broadcastMessage)) {
					cout << "Warning: Broadcast message not fully sent. Size " << sentSize << endl;
					cerr << strerror(errno) << endl;
				}
				else {
					cout << "Broadcast sent." << endl;
				}
				last_broadcast_activation = (ms/2000);
			}

			// UDP Receive
			uint8_t receiveBuf[128];
			socklen_t len = sizeof(addr);

			if (recvfrom(udpFDReceive, receiveBuf, sizeof(receiveBuf), MSG_DONTWAIT, (struct sockaddr *)&addr, &len) > 0) {
				authProcess(receiveBuf,sizeof(receiveBuf));

				// reset the receive UDP
				bzero(&addr, sizeof(addr));
				addr.sin_addr.s_addr = htonl(INADDR_ANY);
				addr.sin_port = htons(inPort);
				addr.sin_family = AF_INET;
			}


			// TCP Connection establishment
			socklen_t tcpLen = sizeof(constTCP);
			newtcpFD = accept(tcpFDstart, (sockaddr *)&cli_addr, &tcpLen);
			if (newtcpFD != -1) {
				for (std::vector<nodeCOMFORT>::iterator it = nodeList.begin(); it != nodeList.end(); ++it) {
					if (it->IP == cli_addr.sin_addr.s_addr) {
						it->connectionFD = newtcpFD;
						it->TCPConnection = cli_addr;
						
						uint8_t RTCPlain[6];
						time_t RTCTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
						
						tm * RTCtm = localtime(&RTCTime);
						cout << asctime(RTCtm) << endl;
						RTCPlain[0] = (uint8_t)RTCtm->tm_mday;
						RTCPlain[1] = (uint8_t)RTCtm->tm_mon + 1;
						RTCPlain[2] = (uint8_t)RTCtm->tm_year - 116;
						RTCPlain[3] = (uint8_t)RTCtm->tm_hour;
						RTCPlain[4] = (uint8_t)RTCtm->tm_min;
						RTCPlain[5] = (uint8_t)RTCtm->tm_sec;
						vector<uint8_t> RTCEncrypt;
						int outSize = Encrypt(RTCPlain, RTCEncrypt, it->shared, iv, sizeof(RTCPlain));
						int size = write(newtcpFD, RTCEncrypt.data(), RTCEncrypt.size());
						it->connected = true;
					}
				}
			}

			ms = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count();

			if (last_print_time < (ms/1000)) {
				cout << ms << endl;
				last_print_time = (ms / 1000);
			}
		}
		close(udpFDReceive);
		close(udpFDSend);
		close(tcpFDstart);
	}

	// Requests data from the sensors, blocks for 2 seconds to allow sensor data to come in.
	void dataRequest() {
		cout << "Gathering sensor data..." << endl;

		for (std::vector<nodeCOMFORT>::iterator it = nodeList.begin(); it != nodeList.end(); ++it) {
			if (it->connected == false) {
				cout << "Skipping node " << it->ID << " (Disabled)." << endl;
				continue;
			}
			else if (it->type == nodeTypeDef::Output) {
				cout << "Skipping node " << it->ID << " (Output)." << endl;
				continue;
			}

			uint8_t RTCPlain[6];
			time_t RTCTime = chrono::system_clock::to_time_t(chrono::system_clock::now());

			tm * RTCtm = localtime(&RTCTime);
			cout << asctime(RTCtm) << endl;
			RTCPlain[0] = (uint8_t)RTCtm->tm_mday;
			RTCPlain[1] = (uint8_t)RTCtm->tm_mon + 1;
			RTCPlain[2] = (uint8_t)RTCtm->tm_year - 116;
			RTCPlain[3] = (uint8_t)RTCtm->tm_hour;
			RTCPlain[4] = (uint8_t)RTCtm->tm_min;
			RTCPlain[5] = (uint8_t)RTCtm->tm_sec;

			const string dataRequest = "DataRequest:";

			CryptoPP::byte sendBuff[UniqueIdentifier.length() + dataRequest.length() + sizeof(uint32_t) + sizeof(RTCPlain)];
			memcpy(sendBuff, UniqueIdentifier.c_str(), UniqueIdentifier.length());
			memcpy(&sendBuff[UniqueIdentifier.length()], dataRequest.c_str(), dataRequest.length());
			*(uint32_t *)&sendBuff[UniqueIdentifier.length() + dataRequest.length()] = it->counter;
			memcpy(&sendBuff[sizeof(sendBuff) - sizeof(RTCPlain)], RTCPlain, sizeof(RTCPlain));

			vector<uint8_t> cipherText;

			int cipherSize = Encrypt(sendBuff, cipherText, it->shared, iv, sizeof(sendBuff));
			int retSize = write(it->connectionFD, &cipherText[0],cipherText.size());
		}


		chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
		while (chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < 2000);


		// Get stuff back
		for (std::vector<nodeCOMFORT>::iterator it = nodeList.begin(); it != nodeList.end(); ++it) {
			if (it->connected == false || it->type == nodeTypeDef::Output || it->type == nodeTypeDef::Error) {
				continue;
			}
			const string dataResponse = "Response:";
			CryptoPP::byte recvBuff[it->data.size() + dataResponse.length() + sizeof(uint32_t) + sizeof(uint32_t) + 16];
			int recvSize = read(it->connectionFD, recvBuff, sizeof(recvBuff));

			if (recvSize == 0) {
				it->connected = false;
				return;
			}

			vector<uint8_t> plainText;
			int plainSize = Decrypt(recvBuff, plainText, it->shared, iv, recvSize);
			if (plainSize != sizeof(recvBuff) - 16) {
				cout << "Node " << it->ID << " sent incorrect packet size." << endl;
				it->connected = false;
				continue;
			}
			if (memcmp(&plainText[0], dataResponse.c_str(), dataResponse.length())) {
				cout << "Node " << it->ID << " sent incorrect Identifier." << endl;
				it->connected = false;
				continue;
			}

			if (*(uint32_t *)&plainText[dataResponse.length()+it->data.size()] != it->counter) {
				cout << "Counter " << *(uint32_t *)&plainText[dataResponse.length()] << "does not match " << it->counter << endl;
				cout << "Disabling node " << it->ID << endl;
			}

			uint32_t clockTime = *(uint32_t *)&plainText[dataResponse.length() + it->data.size() + sizeof(uint32_t)];
			tm tme;
			tme.tm_year = 116 + (clockTime >> 26 & 0x1F); // - 1?
			tme.tm_mon = (clockTime >> 22 & 0xF) - 1;
			tme.tm_mday = (clockTime >> 17 & 0x1F);
			tme.tm_hour = (clockTime >> 12 & 0x1F);
			tme.tm_min = (clockTime >> 6 & 0x3F);
			tme.tm_sec = (clockTime & 0x3F);
			time_t unixTime = mktime((tm *)&tme);

			time_t RTCTime = chrono::system_clock::to_time_t(chrono::system_clock::now());

			uint32_t difference = (unixTime > RTCTime) ? unixTime - RTCTime : RTCTime - unixTime;
			if (difference > 32) {
				cout << "Node " << it->ID << " sent invalid timestamp." << endl;
				it->connected = false;
				continue;
			}

			// If all checks out, populate data.
			memcpy(&(it->data[0]), &plainText[dataResponse.length()], it->data.size());
			it->counter++;
			// Make sure to switch on the thing to put into inData struct
			switch (it->type) {
			case nodeTypeDef::Occupancy:
				inDataBuf.occupancy = *(it->data.data());
				break;
			case nodeTypeDef::Indoor:
				inDataBuf.indoor = *(double *)(it->data.data());
				break;
			case nodeTypeDef::Outdoor:
				inDataBuf.outdoor = *(double *)(it->data.data());
				break;
			case nodeTypeDef::Rel_Humidity:
				inDataBuf.relHumidity = *(double *)(it->data.data());
				break;
			case nodeTypeDef::Globe:
				inDataBuf.globe = *(double *)(it->data.data());
				break;
			default:
				break;
			}
		}
		inData = inDataBuf;
	}

	void sendData() {
		for (std::vector<nodeCOMFORT>::iterator it = nodeList.begin(); it != nodeList.end(); ++it) {
			if (it->type != nodeTypeDef::Output) {
				return;
			}
			const string outputData = "Output Data:";
			CryptoPP::byte sendBuff[3];

		}
	}

	void clearNodes() {
		nodeList.clear();
	}

	void authProcess(uint8_t * receiveBuf, size_t receiveBufSize) {
		if (receiveBufSize != 128) {
			cout << "Wrong return packet." << endl;
			return;
		}
		const int SIZE = 128;

		SecByteBlock incomingPubKey(dh.PublicKeyLength());
		SecByteBlock sharedKey(dh.AgreedValueLength());
		memcpy(&incomingPubKey.BytePtr()[1], receiveBuf, incomingPubKey.SizeInBytes() - 1);
		incomingPubKey.BytePtr()[0] = 0x04;

		cout << "Server PubKey Key: ";
		for (int i = 0; i < pubKey.SizeInBytes(); i++) cout << std::hex << "0x" << (int)(pubKey.BytePtr()[i]) << ", ";
		cout << std::dec << endl;

		cout << "PubKey In Key: ";
		for (int i = 0; i < incomingPubKey.SizeInBytes(); i++) cout << std::hex << "0x" << (int)(incomingPubKey.BytePtr()[i]) << ", ";
		cout << std::dec << endl;

		uint8_t cipherText[SIZE - (dh.PublicKeyLength() - 1)];
		if (!genSharedKey(incomingPubKey, sharedKey)) cout << "ITS NOT WORKING" << endl;
		sharedKey = SHA256Hash(sharedKey);

		cout << "Shared Key: ";
		for (int i = 0; i < sharedKey.SizeInBytes(); i++) cout << std::hex << "0x" << (int)(sharedKey.BytePtr()[i]) << ", ";
		cout << std::dec << endl;

		cout << "IV: ";
		for (int i = 0; i < sizeof(iv); i++) cout << std::hex << "0x" << (int)(iv[i]) << ", ";
		cout << std::dec << endl;
		memcpy(cipherText, &receiveBuf[dh.PublicKeyLength() - 1], sizeof(cipherText));

		cout << "Cipher: ";
		for (int i = 0; i < sizeof(cipherText); i++) cout << std::hex << "0x" << (int)(cipherText[i]) << ", ";
		cout << std::dec << endl;

		vector<uint8_t> plainText;

		int lengthPlain = Decrypt(cipherText, plainText, sharedKey, iv, sizeof(cipherText));
		if (lengthPlain <= 0 || plainText.size() != 50) {
			cout << "Incoming plainText not of right size..." << endl;
			return;
		}
		cout << "Plaintext: ";
		for (int i = 0; i < lengthPlain; i++) cout << std::hex << "0x" << (int)(plainText[i]) << ", ";
		cout << std::dec << endl;

		string deviceID;
		deviceID.assign((char *)plainText.data(), 8);
		cout << "Device ID: " << deviceID << endl;

		uint32_t deviceIP = addr.sin_addr.s_addr;
		cout << "IP ADDR: ";
		for (int i = 0; i < sizeof(int); i++) cout << (int)((uint8_t *)&deviceIP)[i] << ", ";
		cout << endl;

		string deviceType;
		deviceType.assign((char *)&plainText.data()[deviceID.length()], 16);
		int endOfString = deviceType.find('\0');
		if (endOfString != -1) {
			deviceType.resize(endOfString);
		}
		cout << "Device Type: " << deviceType << endl;

		string uniqueIdentAttempt;
		uniqueIdentAttempt.assign((char *)&plainText.data()[8 + 16], UniqueIdentifier.length());
		cout << "Unique Identifier Input: " << uniqueIdentAttempt << endl;
		if (uniqueIdentAttempt != UniqueIdentifier) {
			cout << "Incorrect Unique Identifier... cancelling..." << endl;
			return;
		}
		uint32_t deviceCounter = *(uint32_t *)&plainText.data()[8 + 16 + UniqueIdentifier.length()];
		cout << "Counter : " << deviceCounter << endl;

		bool alreadyExists = false;
		// Scan to ensure these nodes don't already exist:
		for (std::vector<nodeCOMFORT>::iterator it = nodeList.begin(); it != nodeList.end(); ++it) {
			if (it->IP == deviceIP) {
				cout << "IP already exists in table, disabling already existing node." << endl;
				if (it->connectionFD != -1) {
					close(it->connectionFD);
					it->connectionFD = -1;
				}
				it->connected = false;
			}

			if (it->ID == deviceID) {
				cout << "ID already exists... updating IP of node." << endl;
				it->connected = false;
				it->IP = deviceIP;
				it->shared = sharedKey;
				it->counter = deviceCounter;
				alreadyExists = true;
				return;
			}
		}

		// Add a new node to the list
		if (!alreadyExists) {
			nodeTypeDef type = nodeCOMFORT::stringToNodeType(deviceType);
			if (type == nodeTypeDef::Error) {
				cout << "Type does not exist... cancelling." << endl;
				return;
			}
			else {
				nodeCOMFORT client(deviceID, deviceIP, type, deviceCounter, sharedKey);
				nodeList.push_back(client);
			}
		}

		// Send to udpReceive
		const char reply[] = "Send";
		int sentSize = sendto(udpFDReceive, reply, sizeof(reply)-1, MSG_CONFIRM, (sockaddr *)&addr, sizeof(addr));
		if (sentSize != sizeof(reply)-1) {
			cout << "Warning: Broadcast message not fully sent. Size " << sentSize << endl;
			cerr << strerror(errno) << endl;
		}

		cout << "Node has been notified to start TCP communication." << endl;
	}

	void printNodeCOMFORT() {
		int num = 0;
		cout << "---------------------------------" << endl;
		for (std::vector<nodeCOMFORT>::iterator it = nodeList.begin(); it != nodeList.end(); ++it) {
			num++;
			cout << "Node " << num << ":" << endl;
			cout << "ID: " << it->ID << endl;
			cout << "Type: " << it->NodeTypetoString() << endl;
			cout << "IP ADDR: ";
			for (int i = 0; i < sizeof(int); i++) cout << (int)((uint8_t *)&it->IP)[i] << ", ";
			cout << endl;
			cout << "Port: " << it->TCPConnection.sin_port << endl;
			cout << "Counter: " << it->counter << endl;
			cout << "Connected: " << (it->connected ? "On" : "Off") << endl;
			cout << "TCP File Descriptor: " << it->connectionFD << endl;
			cout << "Shared Key: 0x";
			for (int i = 0; i < it->shared.SizeInBytes(); i++) cout << std::hex << (int)(it->shared.BytePtr()[i]);
			cout << std::dec << endl;
			cout << "Data: ";
			if (it->type == nodeTypeDef::Occupancy) {
				cout << ((bool)it->data.data()[0] ? "true" : "false") << endl;
			}
			else {
				cout << *(double *)&it->data.data()[0] << endl;
			}
		}
	}

	// Sensor data public members:
	dataIn inData;
	dataIn inDataBuf;
	volatile dataOut outData;
	volatile dataOut outDataBuf;

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
	volatile int gatherFrequency = 60000;
	volatile bool inData_received = false;
	volatile bool outData_toSend = false;
	volatile bool active = true;
	volatile bool authenticate = false;
	volatile bool clearNode = false;

private:

	// AES Functions
	int Encrypt(uint8_t* plainText, vector<uint8_t> &cipherText, SecByteBlock key, CryptoPP::byte* IV, int size) {
		CBC_Mode<AES>::Encryption e;
		e.SetKeyWithIV(key.BytePtr(), key.SizeInBytes(), IV);
		StreamTransformationFilter encryptor(e, NULL);

		encryptor.Put(plainText, size);
		encryptor.MessageEnd();
		size_t ready = 0;
		while (ready == 0)
			ready = encryptor.MaxRetrievable();

		if (cipherText.size() < ready) {
			cipherText.resize(ready);
		}
		encryptor.Get(&cipherText[0], ready);

		return ready;
	}

	int Decrypt(uint8_t* cipherText, vector<uint8_t>& plainText, SecByteBlock key, CryptoPP::byte* IV, int size) {
		CBC_Mode<AES>::Decryption d(key.BytePtr(), key.SizeInBytes(), IV);
		StreamTransformationFilter decryptor(d, NULL);
		decryptor.Put(cipherText, size);
		decryptor.MessageEnd();
		size_t ready = 0;

		while (ready == 0) {
			ready = decryptor.MaxRetrievable();
		}
		if (plainText.size() < ready) {
			plainText.resize(ready);
		}
		decryptor.Get(&plainText[0], ready);
		return ready;
	}

	SecByteBlock SHA256Hash(SecByteBlock secretKey) {
		SHA256 hash;
		SecByteBlock key(SHA256::DIGESTSIZE);
		hash.CalculateDigest(key, secretKey, secretKey.size());
		return key;
	}

	bool setNetworkInterface() {
		struct ifaddrs *ifap, *ifa;
		getifaddrs(&ifap);
		for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
			if (!(ifa->ifa_flags & IFF_LOOPBACK) && (ifa->ifa_addr && ifa->ifa_broadaddr) && (ifa->ifa_addr->sa_family == AF_INET && ifa->ifa_broadaddr->sa_family == AF_INET)) {
				addr = *((sockaddr_in *)(ifa->ifa_addr));
				broadcast = *((sockaddr_in *)(ifa->ifa_broadaddr));
				freeifaddrs(ifap);
				return true;
			}
		}
		freeifaddrs(ifap);
		return false;
	}

	void cryptoInit() {
		nodeList.reserve(8);
		CURVE = secp256r1();
		dh = ECDH<ECP>::Domain(CURVE);
		privKey = SecByteBlock(dh.PrivateKeyLength());
		pubKey = SecByteBlock(dh.PublicKeyLength());
		rng.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE);
	}

	void genLocalKeys() {
		dh.GenerateKeyPair(rng, privKey, pubKey);
	}

	bool genSharedKey(SecByteBlock &pKey, SecByteBlock &sharedKey) {
		//SecByteBlock inPubKey = SecByteBlock(pKey,dh.PublicKeyLength());
		return dh.Agree(sharedKey, privKey, pKey);
	}

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

int main() {
	serverCOMFORT server;
	server.serverOperation();
	return 0;
};