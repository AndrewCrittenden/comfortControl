#include "servercomfort.h"

serverCOMFORT::serverCOMFORT(QObject *parent) : QObject(parent)
{
}

serverCOMFORT::~serverCOMFORT()
{

}

// Call this function after instancing the object in a thread (should be thread safe)
void serverCOMFORT::serverOperation() {
    if (setNetworkInterface() == false) return;
    cryptoInit();
    genLocalKeys();

    cout << "Private Key, size of " << (int)privKey.size() << ": " << endl;
    for (int i = 0; i < (int)privKey.size(); i++)
        cout << std::hex << "0x" << (int)privKey[i] << ", ";
    cout << std::dec << endl;

    cout << "Public Key, size of " << pubKey.size() << ": " << endl;
    for (int i = 0; i < (int)pubKey.size(); i++)
        cout << std::hex << "0x" << (int)pubKey[i] << ", ";
    cout << std::dec << endl;

    // Disabling this will end the server operation
	active = true;
    while (active) {

        // Timer before new data refresh
        // Check if authentication bool is pressed:
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
        int ms = 0;
        //int last_print_time = 0;
        while (ms < gatherFrequency) {

            //Authentication flag:
            if (authenticate) {
                authentication();
				ms = 0;
                authenticate = false;
            }

            // Clear Devices flag:
            if (clearNode) {
                clearNodes();
				clearNode = false;
            }

			// TODO: Set error flags with Andrew?
			if (outData_toSend) {
				sendData();
				//outData_toSend = false;
			}

            ms = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count();
            //if (last_print_time < (ms/1000)) {
            //    cout << ms << endl;
            //    last_print_time = (ms/1000);
            //}
        }

        cout << "Timer done." << endl;

        checkSensorReady();
        dataRequest();
        //outData.output = 10.34;
        //cout << "__-______-_____--_-r-23-34gt-43g-hg54h54-h54h-54h-45h-45h4-" << endl;

        printNodeCOMFORT();
    }
    return;
}

const int authenticationTime = 10000;

void serverCOMFORT::authentication() {
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
    cout << "BroadcastAddr: ";
    for (int i = 0; i < (int)sizeof(int); i++) cout << (int)((uint8_t *)&broadcast.sin_addr.s_addr)[i] << ", ";
    cout << endl;
    cout << "IP ADDR: ";
    for (int i = 0; i < (int)sizeof(int); i++) cout << (int)((uint8_t *)&addr.sin_addr.s_addr)[i] << ", ";
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

            // Checksum
            uint8_t checkSum[16];
            memset(checkSum, 0, sizeof(checkSum));
            for (int i = 0; i < (int)UniqueIdentifier.length(); i++)
            {
                checkSum[i & 15] ^= UniqueIdentifier[i];
            }
            for (int i = 1; i < (int)pubKey.SizeInBytes(); i++)
            {
                checkSum[(i-1) & 15] ^= pubKey[i];
            }


			// Populate broadcast message
            CryptoPP::byte broadcastMessage[UniqueIdentifier.length() + pubKey.SizeInBytes()-1 + sizeof(checkSum) + sizeof(iv)];
            memcpy(&broadcastMessage, UniqueIdentifier.c_str(), UniqueIdentifier.length());
            memcpy(&broadcastMessage[UniqueIdentifier.length()], &(pubKey.BytePtr()[1]), pubKey.SizeInBytes()-1);
            memcpy(&broadcastMessage[UniqueIdentifier.length() + pubKey.SizeInBytes() - 1], checkSum, sizeof(checkSum));
            memcpy(&broadcastMessage[sizeof(broadcastMessage) - sizeof(iv)], &iv[0], sizeof(iv));
            cout << "Broadcast size: " << sizeof(broadcastMessage) << endl;

            //cout << "Broadcast Message: ";
            //for (int i = 0; i < sizeof(broadcastMessage); i++) cout << std::hex << "0x" << (int)broadcastMessage[i] << ", ";
            //cout << std::dec << endl;

			// UDP send
            int sentSize = sendto(udpFDSend, broadcastMessage, sizeof(broadcastMessage), MSG_CONFIRM, (sockaddr *)&broadcast, sizeof(broadcast));
            if (sentSize != (int)sizeof(broadcastMessage)) {
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

					// Timestamping
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
                    Encrypt(RTCPlain, RTCEncrypt, it->shared, iv, sizeof(RTCPlain));
                    int size = write(newtcpFD, RTCEncrypt.data(), RTCEncrypt.size());
					if (size != (int)RTCEncrypt.size()) {
						cout << "Problem in sending back RTC..." << endl;
						return;
					}
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

const int RECV_WAIT = 2000;

// Requests data from the sensors, blocks for 2 seconds to allow sensor data to come in.
void serverCOMFORT::dataRequest() {
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

        Encrypt(sendBuff, cipherText, it->shared, iv, sizeof(sendBuff));
        int retSize = write(it->connectionFD, &cipherText[0],cipherText.size());
		if (retSize != (int)cipherText.size()) {
			cout << "Error in data Request" << endl;
		}
    }


    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
    while (chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < RECV_WAIT);


    // Get stuff back
    for (std::vector<nodeCOMFORT>::iterator it = nodeList.begin(); it != nodeList.end(); ++it) {
        if (it->connected == false || it->type == nodeTypeDef::Output || it->type == nodeTypeDef::Error) {
            continue;
        }
        const string dataResponse = "Response:";
        CryptoPP::byte recvBuff[it->data.size() + dataResponse.length() + sizeof(uint32_t) + sizeof(uint32_t) + 16];
		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(it->connectionFD, &read_fds);
		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		int selectStatus = select(it->connectionFD+1, &read_fds, NULL, NULL, &tv);
		int recvSize = 0;
		switch (selectStatus) {
		case -1:
			cout << "Oh no..." << endl;
			it->connected = false;
            switch(it->type) {
                case nodeTypeDef::Occupancy:
                    setStatusOccupancy(false);
                break;
                case nodeTypeDef::Indoor:
                    setStatusIndoor(false);
                break;
                case nodeTypeDef::Outdoor:
                    setStatusOutdoor(false);
                break;
                case nodeTypeDef::Globe:
                    setStatusGlobe(false);
               break;
                case nodeTypeDef::Rel_Humidity:
                    setStatusRelHum(false);
                break;
                default:

                break;
            }
			continue;

		case 0:
            cout << "Nothing to read..." << it->ID << "..." << endl;
            switch(it->type) {
                case nodeTypeDef::Occupancy:
                    setStatusOccupancy(false);
                break;
                case nodeTypeDef::Indoor:
                    setStatusIndoor(false);
                break;
                case nodeTypeDef::Outdoor:
                    setStatusOutdoor(false);
                break;
                case nodeTypeDef::Globe:
                    setStatusGlobe(false);
               break;
                case nodeTypeDef::Rel_Humidity:
                    setStatusRelHum(false);
                break;
                default:

                break;
            }
			it->connected = false;
			continue;

		default:
			recvSize = read(it->connectionFD, recvBuff, sizeof(recvBuff));
			break;
		}
        
        cout << "Received from ID " << it->ID << ", Size " << recvSize << endl;
        if (recvSize == 0) {
            switch(it->type) {
                case nodeTypeDef::Occupancy:
                    setStatusOccupancy(false);
                break;
                case nodeTypeDef::Indoor:
                    setStatusIndoor(false);
                break;
                case nodeTypeDef::Outdoor:
                    setStatusOutdoor(false);
                break;
                case nodeTypeDef::Globe:
                    setStatusGlobe(false);
               break;
                case nodeTypeDef::Rel_Humidity:
                    setStatusRelHum(false);
                break;
                default:

                break;
            }
            it->connected = false;
            continue;
        }

        vector<uint8_t> plainText;
        if (recvSize % 16) {
            cout << "Not a multiple of 16. Disabling Node " << it->ID << endl;
            switch(it->type) {
                case nodeTypeDef::Occupancy:
                    setStatusOccupancy(false);
                break;
                case nodeTypeDef::Indoor:
                    setStatusIndoor(false);
                break;
                case nodeTypeDef::Outdoor:
                    setStatusOutdoor(false);
                break;
                case nodeTypeDef::Globe:
                    setStatusGlobe(false);
               break;
                case nodeTypeDef::Rel_Humidity:
                    setStatusRelHum(false);
                break;
                default:

                break;
            }

            it->connected = false;
            continue;
        }
        int plainSize = Decrypt(recvBuff, plainText, it->shared, iv, recvSize);
        if (plainSize != (int)sizeof(recvBuff) - 16) {
            cout << "Node " << it->ID << " sent incorrect packet size." << endl;
            switch(it->type) {
                case nodeTypeDef::Occupancy:
                    setStatusOccupancy(false);
                break;
                case nodeTypeDef::Indoor:
                    setStatusIndoor(false);
                break;
                case nodeTypeDef::Outdoor:
                    setStatusOutdoor(false);
                break;
                case nodeTypeDef::Globe:
                    setStatusGlobe(false);
               break;
                case nodeTypeDef::Rel_Humidity:
                    setStatusRelHum(false);
                break;
                default:

                break;
            }
            it->connected = false;
            continue;
        }
        if (memcmp(&plainText[0], dataResponse.c_str(), dataResponse.length())) {
            cout << "Node " << it->ID << " sent incorrect DataResponse Identifier." << endl;
            switch(it->type) {
                case nodeTypeDef::Occupancy:
                    setStatusOccupancy(false);
                break;
                case nodeTypeDef::Indoor:
                    setStatusIndoor(false);
                break;
                case nodeTypeDef::Outdoor:
                    setStatusOutdoor(false);
                break;
                case nodeTypeDef::Globe:
                    setStatusGlobe(false);
               break;
                case nodeTypeDef::Rel_Humidity:
                    setStatusRelHum(false);
                break;
                default:

                break;
            }
            it->connected = false;
            continue;
        }

        if (*(uint32_t *)&plainText[dataResponse.length()+it->data.size()] != it->counter) {
            cout << "Counter " << *(uint32_t *)&plainText[dataResponse.length()] << "does not match " << it->counter << endl;
            cout << "Disabling node " << it->ID << endl;
			continue;
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
        if (difference > 5) {
            cout << "Node " << it->ID << " sent invalid timestamp." << endl;
            switch(it->type) {
                case nodeTypeDef::Occupancy:
                    setStatusOccupancy(false);
                break;
                case nodeTypeDef::Indoor:
                    setStatusIndoor(false);
                break;
                case nodeTypeDef::Outdoor:
                    setStatusOutdoor(false);
                break;
                case nodeTypeDef::Globe:
                    setStatusGlobe(false);
               break;
                case nodeTypeDef::Rel_Humidity:
                    setStatusRelHum(false);
                break;
                default:

                break;
            }
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
    //inData = inDataBuf;
	buffIn.SetPrivateBuffer(inDataBuf);
	buffIn.RotatePrivateBuffer();
	setInData_received(true);
}

const int SEND_WAIT = 2000;

// Sends data that's in the triple buffer
void serverCOMFORT::sendData() {
    for (std::vector<nodeCOMFORT>::iterator it = nodeList.begin(); it != nodeList.end(); ++it) {
        if (it->type != nodeTypeDef::Output || it->connected == false) {
            //cout << "Skipping node " << it->ID << " for output." << endl;
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

        const string outputData = "Output Data:";

        CryptoPP::byte sendBuff[UniqueIdentifier.length() + outputData.length() + sizeof(uint32_t) + sizeof(RTCPlain) + sizeof(outData)];
		memcpy(sendBuff, UniqueIdentifier.c_str(), UniqueIdentifier.length());
		memcpy(&sendBuff[UniqueIdentifier.length()], outputData.c_str(), outputData.length());
        dataOut temp = buffOut.GetPublicBuffer();
        memcpy(&sendBuff[UniqueIdentifier.length() + outputData.length()], &temp, sizeof(outData));
		memcpy(&sendBuff[UniqueIdentifier.length() + outputData.length() + sizeof(outData)], &(it->counter), sizeof(uint32_t));
		memcpy(&sendBuff[sizeof(sendBuff) - sizeof(RTCPlain)], RTCPlain, sizeof(RTCPlain));

		vector<uint8_t> cipherText;
		Encrypt(sendBuff, cipherText, it->shared, iv, sizeof(sendBuff));
		int retSize = write(it->connectionFD, &cipherText[0], cipherText.size());
		if (retSize != (int)cipherText.size()) {
			cout << "Error in data output" << endl;
		}
		memcpy(&it->data[0], &outData, it->data.size());
    }

	chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
	while (chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() < SEND_WAIT);

	// Receive a confirmation back
	for (std::vector<nodeCOMFORT>::iterator it = nodeList.begin(); it != nodeList.end(); ++it) {
		if (it->connected == false || it->type != nodeTypeDef::Output) {
			continue;
		}
		const string dataReceived = "DataReceived";
		CryptoPP::byte recvBuff[UniqueIdentifier.length() + dataReceived.length() + sizeof(uint32_t) + sizeof(uint32_t) + 16];
		
		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(it->connectionFD, &read_fds);
		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 1;

		int selectStatus = select(it->connectionFD+1, &read_fds, NULL, NULL, &tv);
		int recvSize = 0;
		switch (selectStatus) {
		case -1:
			cout << "Oh no..." << endl;
			it->connected = false;
			continue;

		case 0:
			cout << "Nothing to read..." << endl;
			it->connected = false;
			continue;

		default:
			recvSize = read(it->connectionFD, recvBuff, sizeof(recvBuff));
			break;
		}

		if (recvSize == 0) {
			it->connected = false;
			return;
		}

		vector<uint8_t> plainText;
        if (recvSize % 16) {
            cout << "Not a multiple of 16. Disabling Node " << it->ID << endl;
            it->connected = false;
            continue;
        }

		int plainSize = Decrypt(recvBuff, plainText, it->shared, iv, recvSize);
		if (plainSize != (int)sizeof(recvBuff) - 16) {
			cout << "Node " << it->ID << " sent incorrect packet size." << endl;
			it->connected = false;
			continue;
		}

		// Check Unique Identifier:
		if (memcmp(&plainText[0], UniqueIdentifier.c_str(), UniqueIdentifier.length())) {
			cout << "Node " << it->ID << " sent incorrect Unique Identifier." << endl;
			it->connected = false;
			continue;
		}

		// Check DataReceived:
		if (memcmp(&plainText[UniqueIdentifier.length()], dataReceived.c_str(), dataReceived.length())) {
			cout << "Node " << it->ID << " sent incorrect DataReceived Identifier." << endl;
			it->connected = false;
			continue;
		}

		// Check Counter Value:
		if (*(uint32_t *)&plainText[UniqueIdentifier.length() + dataReceived.length()] != it->counter) {
			cout << "Counter " << *(uint32_t *)&plainText[UniqueIdentifier.length() + dataReceived.length()] 
				 << "does not match " << it->counter << endl;
			cout << "Disabling node " << it->ID << endl;
			continue;
		}

		// Timestamp time:
		uint32_t clockTime = *(uint32_t *)&plainText[UniqueIdentifier.length() + dataReceived.length() + sizeof(uint32_t)];
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
		if (difference > 5) {
			cout << "Node " << it->ID << " sent invalid timestamp." << endl;
			it->connected = false;
			continue;
		}
		it->counter++;
		outData_toSend = false;
	}
}

void serverCOMFORT::clearNodes() {
    nodeList.clear();
	cout << "Node List Cleared..." << endl;
}

void serverCOMFORT::checkSensorReady() {
    sensorsReady = sensorStatus.indoor && sensorStatus.outdoor && sensorStatus.globe && sensorStatus.relHumidity && sensorStatus.occupancy;
    cout << "sensorsReady = " << (sensorsReady ? "true" : "false") << endl;
    if (sensorsReady == false) {
        cout << "Not all sensors are connected." << endl;
    }
}

// Function for authentication process
void serverCOMFORT::authProcess(uint8_t * receiveBuf, size_t receiveBufSize) {
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
    for (int i = 0; i < (int)pubKey.SizeInBytes(); i++) cout << std::hex << "0x" << (int)(pubKey.BytePtr()[i]) << ", ";
    cout << std::dec << endl;

    cout << "PubKey In Key: ";
    for (int i = 0; i < (int)incomingPubKey.SizeInBytes(); i++) cout << std::hex << "0x" << (int)(incomingPubKey.BytePtr()[i]) << ", ";
    cout << std::dec << endl;

    uint8_t cipherText[SIZE - (dh.PublicKeyLength() - 1)];
    if (!genSharedKey(incomingPubKey, sharedKey)) cout << "ITS NOT WORKING" << endl;
    sharedKey = SHA256Hash(sharedKey);

    cout << "Shared Key: ";
    for (int i = 0; i < (int)sharedKey.SizeInBytes(); i++) cout << std::hex << "0x" << (int)(sharedKey.BytePtr()[i]) << ", ";
    cout << std::dec << endl;

    cout << "IV: ";
    for (int i = 0; i < (int)sizeof(iv); i++) cout << std::hex << "0x" << (int)(iv[i]) << ", ";
    cout << std::dec << endl;
    memcpy(cipherText, &receiveBuf[dh.PublicKeyLength() - 1], sizeof(cipherText));

    cout << "Cipher: ";
    for (int i = 0; i < (int)sizeof(cipherText); i++) cout << std::hex << "0x" << (int)(cipherText[i]) << ", ";
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
    for (int i = 0; i < (int)sizeof(int); i++) cout << (int)((uint8_t *)&deviceIP)[i] << ", ";
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
    nodeTypeDef type = nodeCOMFORT::stringToNodeType(deviceType);
    switch(type) {
        case nodeTypeDef::Occupancy:
            setStatusOccupancy(true);
        break;
        case nodeTypeDef::Indoor:
            setStatusIndoor(true);
        break;
        case nodeTypeDef::Outdoor:
            setStatusOutdoor(true);
        break;
        case nodeTypeDef::Globe:
            setStatusGlobe(true);
       break;
        case nodeTypeDef::Rel_Humidity:
            setStatusRelHum(true);
        break;
        case nodeTypeDef::Error:
            cout << "Type does not exist... cancelling." << endl;
            return;
        default:

        break;
    }

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
            it->type = type;
            it->shared = sharedKey;
            it->counter = deviceCounter;
            alreadyExists = true;
            return;
        }
    }


    // Add a new node to the list
    if (!alreadyExists) {
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

// Simple print all nodes in nodeList
void serverCOMFORT::printNodeCOMFORT() {
    int num = 0;
    cout << "---------------------------------" << endl;
    for (std::vector<nodeCOMFORT>::iterator it = nodeList.begin(); it != nodeList.end(); ++it) {
        num++;
        cout << "Node " << num << ":" << endl;
        cout << "ID: " << it->ID << endl;
        cout << "Type: " << it->NodeTypetoString() << endl;
        cout << "IP ADDR: ";
        for (int i = 0; i < (int)sizeof(int); i++) cout << (int)((uint8_t *)&it->IP)[i] << ", ";
        cout << endl;
        cout << "Port: " << it->TCPConnection.sin_port << endl;
        cout << "Counter: " << it->counter << endl;
        cout << "Connected: " << (it->connected ? "On" : "Off") << endl;
        cout << "TCP File Descriptor: " << it->connectionFD << endl;
        cout << "Shared Key: 0x";
        for (int i = 0; i < (int)it->shared.SizeInBytes(); i++) cout << std::hex << (int)(it->shared.BytePtr()[i]);
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

//Signals to send to GUI
void serverCOMFORT::setInData_received(bool value)
{
    inData_received = value;
    if (inData_received) {
        emit inData_receivedChanged(inData_received);
    }
}

void serverCOMFORT::setStatusIndoor(bool value)
{
    sensorStatus.indoor = value;
    emit statusIndoorChanged(sensorStatus.indoor);
}
void serverCOMFORT::setStatusOutdoor(bool value)
{
    sensorStatus.outdoor = value;
    emit statusOutdoorChanged(sensorStatus.outdoor);
}
void serverCOMFORT::setStatusRelHum(bool value)
{
    sensorStatus.relHumidity = value;
    emit statusRelHumChanged(sensorStatus.relHumidity);
}
void serverCOMFORT::setStatusGlobe(bool value)
{
    sensorStatus.globe = value;
    emit statusGlobeChanged(sensorStatus.globe);
}
void serverCOMFORT::setStatusOccupancy(bool value)
{
    sensorStatus.occupancy = value;
    emit statusOccupancyChanged(sensorStatus.occupancy);
}

// AES Functions
int serverCOMFORT::Encrypt(uint8_t* plainText, vector<uint8_t> &cipherText, SecByteBlock key, CryptoPP::byte* IV, int size) {
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

int serverCOMFORT::Decrypt(uint8_t* cipherText, vector<uint8_t>& plainText, SecByteBlock key, CryptoPP::byte* IV, int size) {
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

// Hash function
SecByteBlock serverCOMFORT::SHA256Hash(SecByteBlock secretKey) {
    SHA256 hash;
    SecByteBlock key(SHA256::DIGESTSIZE);
    hash.CalculateDigest(key, secretKey, secretKey.size());
    return key;
}

// Sets network interface settings to the IP and broadcast address of the connection to LAN
bool serverCOMFORT::setNetworkInterface() {
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

// Initializes objects and cryptography engines
void serverCOMFORT::cryptoInit() {
    nodeList.reserve(8);
    CURVE = secp256r1();
    dh = ECDH<ECP>::Domain(CURVE);
    privKey = SecByteBlock(dh.PrivateKeyLength());
    pubKey = SecByteBlock(dh.PublicKeyLength());
    rng.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE);

	memset(&inData, 0, sizeof(dataIn));
	inDataBuf = inData;
	memset(&outData, 0, sizeof(dataOut));
	outDataBuf = outData;
    memset(&sensorStatus,0,sizeof(sensorReady));
    sensorsReady = false;

	buffIn = TripleBuffer<dataIn>(inData, inData, inData);
	buffOut = TripleBuffer<dataOut>(outData, outData, outData);

}

// Generates local keys
void serverCOMFORT::genLocalKeys() {
    dh.GenerateKeyPair(rng, privKey, pubKey);
}

// Generates shared key
bool serverCOMFORT::genSharedKey(SecByteBlock &pKey, SecByteBlock &sharedKey) {
    //SecByteBlock inPubKey = SecByteBlock(pKey,dh.PublicKeyLength());
    return dh.Agree(sharedKey, privKey, pKey);
}
