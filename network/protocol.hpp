#ifndef PROTOCOL_HPP_INCLUDED
#define PROTOCOL_HPP_INCLUDED

#include <vector>
#include <cstdint>

/**
 * @brief Abstract class representing a message. A message has to be serializable into a byte vector, its content has to be retrievable, it has to have a parse method which constructs a message from its serialized form and an ID, which identifies the message
 */
class Message
{
public:
    /**
     * @brief Serializes the message into a byte vector
     * @return Serialized message
     */
    virtual std::vector<uint8_t> serialize();

    /**
     * @brief Returns the payload/data of the message
     * @return Vector containing the content/payload
     */
    virtual std::vector<uint8_t> getContent() const=0;

    /**
     * @brief Parses a serialized form of a message into a Message object; If the argument contains more than one message (or other bytes), the rest is ignored
     * @param pSerializedData The serialized representation which shall be used to reconstruct the message from
     * @return The number of bytes out of the pSerializedData which contained the message
     */
    virtual uint32_t parse(const std::vector<uint8_t>& pSerializedData)=0;

    /**
     * @brief Returns the ID of the message
     * @return The ID of the message
     */
    virtual uint8_t getID() const=0;
};

/**
 * @brief Helper function which allows to append data with a specific number of bytes to a vector in network byte order
 * @param pVector The vector to append the bytes to
 * @param pData The data to append to the vector pVector
 * @param pSize The size of pData in Bytes
 */
void appendNetworkByteOrderInteger(std::vector<uint8_t>& pVector, uint8_t* pData, size_t pSize);

/**
 * @brief Returns a single byte red from the iterator pVectorBegin
 * @return First byte red from the iterator pVectorBegin
 */
uint8_t  networkByteOrderDataToUint8 (std::vector<uint8_t>::const_iterator pVectorBegin);


/**
 * @brief Reads a uint16 from the iterator which is given as argument, assuming that the data is in network byte order
 * @return Red uint16 value
 */
uint16_t networkByteOrderDataToUint16(std::vector<uint8_t>::const_iterator pVectorBegin);

/**
 * @brief Reads a uint32 from the iterator which is given as argument, assuming that the data is in network byte order
 * @return Red uint32 value
 */
uint32_t networkByteOrderDataToUint32(std::vector<uint8_t>::const_iterator pVectorBegin);

/**
 * @brief Sends a vector of bytes over a connected socket; This is a convenience function
 * @return Return value of the send() call
 */
int sendVector(int pSocket, const std::vector<uint8_t>& pData);

#endif /*PROTOCOL_HPP_INCLUDED*/