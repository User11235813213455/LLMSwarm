#include "protocol.hpp"
#ifdef __linux
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif
#include <stdexcept>

std::vector<uint8_t> Message::serialize()
{
    std::vector<uint8_t> content = this->getContent();
    std::vector<uint8_t> result(content.size() + 1, '\0');
    result[0] = this->getID();
    std::copy(content.begin(), content.end(), result.begin() + 1);
    return result;
}
uint8_t networkByteOrderDataToUint8(std::vector<uint8_t>& pVector)
{
    return pVector.at(0);
}
uint16_t networkByteOrderDataToUint16(std::vector<uint8_t>& pVector)
{
    uint16_t n = (static_cast<uint16_t>(pVector[0])) << 8
             |   (static_cast<uint16_t>(pVector[1]));
    return ntohs(n);
}
uint32_t networkByteOrderDataToUint32(std::vector<uint8_t>& pVector)
{
    uint32_t n = (static_cast<uint32_t>(pVector[0])) << 24
             |   (static_cast<uint32_t>(pVector[1])) << 16
             |   (static_cast<uint32_t>(pVector[2])) << 8
             |   (static_cast<uint32_t>(pVector[3]));
    return ntohl(n);
}
uint8_t  networkByteOrderDataToUint8(std::vector<uint8_t>::const_iterator pVectorBegin)
{
    return *pVectorBegin;
}
uint16_t networkByteOrderDataToUint16(std::vector<uint8_t>::const_iterator pVectorBegin)
{
    uint16_t n = (static_cast<uint16_t>(*pVectorBegin)) << 8
             |   (static_cast<uint16_t>(*(pVectorBegin + 1)));

    return ntohs(n);
}
uint32_t networkByteOrderDataToUint32(std::vector<uint8_t>::const_iterator pVectorBegin)
{
    uint32_t n = (static_cast<uint32_t>(*pVectorBegin)) << 24
             |   (static_cast<uint32_t>(*(pVectorBegin + 1))) << 16
             |   (static_cast<uint32_t>(*(pVectorBegin + 2))) << 8
             |   (static_cast<uint32_t>(*(pVectorBegin + 3)));
    return ntohl(n);
}

void appendNetworkByteOrderInteger(std::vector<uint8_t>& pVector, uint8_t* pData, size_t pSize)
{
    switch(pSize)
    {
        case 1:
        {
            pVector.push_back(*pData);
        }
        break;
        case 2:
        {
            uint16_t value = htons(*reinterpret_cast<uint16_t*>(pData));
            pVector.push_back((value & 0xFF00) >> 8);
            pVector.push_back((value & 0x00FF));
        }
        break;
        case 4:
        {
            uint32_t value = htonl(*reinterpret_cast<uint32_t*>(pData));
            pVector.push_back((value & 0xFF000000) >> 24);
            pVector.push_back((value & 0x00FF0000) >> 16);
            pVector.push_back((value & 0x0000FF00) >> 8);
            pVector.push_back((value & 0x000000FF));
        }
        break;
        default:
        {
            /*Unsupported*/
            throw(std::invalid_argument("Invalid data size for network order function!"));
        }
    }
}
int sendVector(int pSocket, const std::vector<uint8_t>& pData)
{
    char* buffer = new char[pData.size()];
    unsigned int cntr = 0;
    for(cntr=0; cntr<pData.size(); cntr++)
    {
        buffer[cntr] = static_cast<char>(pData[cntr]);
    }
    int ret = send(pSocket, buffer, pData.size(), 0);
    delete[] buffer;
    return ret;
}