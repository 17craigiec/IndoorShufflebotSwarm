#pragma once
#include "MessageType.hh"

#include <cstddef>
#include <cstring>
#include <cstdio>


enum SerializerMode
{
    MODE_READ = 0,
    MODE_WRITE = 1
};

template<typename T>
class MessageSerializer
{
private:
    T *msg_;

    MessageType msg_type_;
    SerializerMode serializer_mode_;
    float msg_mono_sec_;
    bool verbose_ = true;

public:
    // Write mode constructor - takes a pointer to the message struct to be serialized, and the message type
    MessageSerializer(T *msg_, MessageType type);
    // Read mode constructor - takes only the message type, since the message struct will be populated by parsing raw data
    MessageSerializer(MessageType type);

    ~MessageSerializer();

    // Parses the first byte to determine message type, then reads the rest of the data into the appropriate struct.
    // Returns 0 on success, -1 on failure (e.g. invalid message)
    int ParseMessageType(const unsigned char *_data, unsigned int _len, MessageType &_type);

    // Reports the size of the message based on the message type (including the 1 byte for message type)
    unsigned int GetMessageSize() const;

    SerializerMode GetMode() const;

    // Reads raw data and populates the appropriate message struct based on the first byte (message type).
    // Inputs: _data is the raw message data, _len is the length of the data.
    // Returns 0 on success, -1 on failure (e.g. invalid message)
    int Read(const unsigned char *_data, unsigned int _len, T *_msg);

    // Writes the appropriate message struct into the provided buffer, prefixed with the message type byte.
    // Inputs: _data is the destination buffer, _max_len is the size of the destination buffer.
    // Returns 0 on success, -1 on failure (e.g. buffer too small)
    int Write(unsigned char *_data, unsigned int _max_len);
};


template<typename T>
MessageSerializer<T>::MessageSerializer(T *msg, MessageType type)
{
    msg_ = msg;
    serializer_mode_ = MODE_WRITE;

    // Check the message type is within valid bounds
    int type_value = static_cast<int>(type);
    if (!isValidMessageType(type_value)) {
        printf("[Error] MessageSerializer: Invalid message type value: %d\n", type_value);
        msg_type_ = MTYPE_UNKNOWN; // Default to unknown type
        return;
    }

    msg_type_ = type;
};

template<typename T>
MessageSerializer<T>::MessageSerializer(MessageType type)
{
    msg_ = nullptr;
    serializer_mode_ = MODE_READ;

    // Check the message type is within valid bounds
    int type_value = static_cast<int>(type);
    if (!isValidMessageType(type_value)) {
        printf("[Error] MessageSerializer: Invalid message type value: %d\n", type_value);
        msg_type_ = MTYPE_UNKNOWN; // Default to unknown type
        return;
    }

    msg_type_ = type;
};

template<typename T>
MessageSerializer<T>::~MessageSerializer()
{
};

template<typename T>
int MessageSerializer<T>::ParseMessageType(const unsigned char *_data, unsigned int _len, MessageType &_type)
{
    if (_len < 1)
    {
        printf("[Error] ParseMessageType: Message too short to contain type byte. Length: %u\n", _len);
        return -1; // Invalid message
    }
    
    // Check the message type is within valid bounds
    int type_value = static_cast<int>(_data[0]);
    if (!isValidMessageType(type_value))
    {
        printf("[Error] ParseMessageType: Invalid message type value: %d\n", type_value);
        return -1; // Invalid message type
    }

    // Cast to enum after validation
    _type = static_cast<MessageType>(_data[0]);

    if (_len != GetMessageSize())
    {
        printf("[Error] ParseMessageType: Invalid message length: %u  Expected: %u\n", _len, GetMessageSize());
        return -1; // Invalid message
    }

    return 0; // Success
};

template<typename T>
unsigned int MessageSerializer<T>::GetMessageSize() const
{
    return 1 + sizeof(T);
};

template<typename T>
SerializerMode MessageSerializer<T>::GetMode() const
{
    return serializer_mode_;
};

template<typename T>
int MessageSerializer<T>::Read(const unsigned char *_data, unsigned int _len, T *_msg)
{
    if (GetMode() == MODE_WRITE)
    {
        printf("[Error] Read: Serializer constructed in WRITE mode.\n");
        return -1;
    }
    

    MessageType type;
    if (ParseMessageType(_data, _len, type) != 0)
        return -1;

    if (type != msg_type_ && verbose_)
    {
        printf("[Note] Read discovered incompatible type %u, got %u\n", msg_type_, type);
        return 1;
    }

    // Verify we have enough data
    if (_len < 1 + sizeof(T))
    {
        printf("[Error] Read: Not enough data. Expected %zu, got %u\n", 1 + sizeof(T), _len);
        return -1;
    }

    memcpy(_msg, _data + 1, sizeof(T));
    return 0;
}

template<typename T>
int MessageSerializer<T>::Write(unsigned char *_data, unsigned int _max_len)
{
    if (GetMode() == MODE_READ)
    {
        printf("[Error] Write: Serializer constructed in READ mode.\n");
        return -1;
    }

    size_t needed = 1 + sizeof(T);
    if (needed > _max_len)
    {
        printf("[Error] Write: Buffer too small. Needed: %zu, Max: %u\n", needed, _max_len);
        return -1;
    }

    _data[0] = static_cast<unsigned char>(msg_type_);
    memcpy(_data + 1, msg_, sizeof(T));
    return 0; // Success
}