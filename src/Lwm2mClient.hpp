#ifndef LWM2M_CLIENT_HPP
#define LWM2M_CLIENT_HPP

#include "liblwm2m.h"
#include "connection.h"

#include "UDPSocket.h"
#include "UDPClient.hpp"

#include "socket.h"

namespace mbed
{
    class LWM2MClient
    {
    public:
        struct client_data_t
        {
            lwm2m_object_t *securityObjP;
            LWM2MClient* sessionP;
        };
    
    public:
        explicit LWM2MClient(
            UDPClient *cellularClient, 
            int port = MBED_CONF_LWM2M_CLIENT_PORT,
            const char* ip = MBED_CONF_LWM2M_CLIENT_IPV4);

        ~LWM2MClient() = default; // TODO: implement destructor to free resources

        void Run();

        SocketAddress* GetAddress() { return &_socketAddress; }
        UDPClient* SocketClient() const { return _udpClient; }

    private:
        static constexpr uint8_t OBJ_COUNT {4};
        static constexpr int MAX_PACKET_SIZE {1024};

        UDPClient *_udpClient;
        SocketAddress _socketAddress;
        EventFlags _flags;
        lwm2m_context_t* _lwm2mH;
        std::string _name;
        client_data_t _data;

        lwm2m_object_t *_objects[OBJ_COUNT] = {0};
    };

    using connection_t = LWM2MClient;

} // namespace mbed

#endif // LWM2M_CLIENT_HPP