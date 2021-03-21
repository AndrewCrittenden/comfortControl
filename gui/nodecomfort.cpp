#include "nodecomfort.h"

nodeCOMFORT::nodeCOMFORT(string ID_new, uint32_t IP_new, nodeTypeDef type_new, uint32_t new_counter, SecByteBlock sharedKey) {
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

string nodeCOMFORT::NodeTypetoString() {
    switch (type) {
    case nodeTypeDef::Indoor:
        return "Indoor";
    case nodeTypeDef::Outdoor:
        return "Outdoor";
    case nodeTypeDef::Test:
        return "Test";
    case nodeTypeDef::Rel_Humidity:
        return "Rel. Humidity";
    case nodeTypeDef::Globe:
        return "Globe";
    case nodeTypeDef::Output:
        return "Output";
    case nodeTypeDef::Occupancy:
        return "Occupancy";
    default:
        break;
    };
    return "Error";
}

nodeTypeDef nodeCOMFORT::stringToNodeType(string input) {
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
