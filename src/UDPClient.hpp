#ifndef UDP_CLIENT_HPP
#define UDP_CLIENT_HPP

#include "UDPSocket.h"

#include "CellularClient.hpp"
#include "EventSet.hpp"
#include "LowPowerTimer.h"


namespace mbed
{

class UDPClient
{
private:
    /* data */
public:
    explicit UDPClient(CellularClient* networkClient);
    ~UDPClient();

    bool OpenSocket();

    nsapi_error_t Send(const SocketAddress& address, uint8_t* buffer, size_t length);

    nsapi_error_t Receive(SocketAddress& address, uint8_t* buffer, size_t length);

    nsapi_error_t Poll(time_t timeout);

private:

    void DataReceivedCallback();

    void NetworkStatusCallback(NET_DEFS_STATE status, intptr_t param);
private:

    CellularClient* _networkClient;
    EventSet _networkState;
    UDPSocket _udpSocket;
    bool _dataReceived;
};




}


#endif // UDP_CLIENT_HPP