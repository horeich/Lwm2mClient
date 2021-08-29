#include "UDPClient.hpp"

#define TRACE_GROUP "UDP "

using namespace mbed;

UDPClient::UDPClient(CellularClient* networkClient) :
    _networkClient(networkClient),
    _networkState(NET_DEFS_STATE_NETWORK_DISCONNECTED),
    _udpSocket(),
    _dataReceived(false)
{
    _networkClient->AttachStatusCallback(callback(this, &UDPClient::NetworkStatusCallback));
}

UDPClient::~UDPClient()
{
}

nsapi_error_t UDPClient::Poll(time_t timeout)
{
    LowPowerTimer _timeout;
    _timeout.start();
    while (!_dataReceived)
    {
        if (_timeout.elapsed_time() > std::chrono::seconds(timeout))
        {
            return MBED_SUCCESS;
        }
    }
    return 1;
}

nsapi_error_t UDPClient::Receive(SocketAddress& address, uint8_t* buffer, size_t length)
{
    tr_debug("CoAP::%s", __func__);
    
    // // According to UBLOX: each packet is stored in a separate buffer and only one packet or portion of it is read at a time
    // // e.g. length > packet size -> only packet will be returned
    // // Returns a full datagram if available (maximum R412 datagram size is 1024kB, limited by
    // // current max size [COAP_MESSAGE_BUFFER_SIZE])
    // // Currently we only read responses with 4 bytes
    // // TODO: if rc > 1024 -> read again, do not push paket
    nsapi_error_t rc = _udpSocket.recvfrom(&address, buffer, length);
    if (rc < NSAPI_ERROR_OK) // no data available (NSAPI_ERROR_WOULD_BLOCK) or error
    {
        tr_error("Error while receiving CoAP data");
        return rc; // < 0
    }
    _dataReceived = false;
    tr_warn("Received %d bytes", rc);
    return rc;
}

nsapi_error_t UDPClient::Send(const SocketAddress& address, uint8_t* buffer, size_t length)
{
    tr_debug("UDPClient::%s", __func__);
    
    nsapi_error_t rc = _udpSocket.sendto(address, buffer, length);
    if (rc < NSAPI_ERROR_OK)
    {
        tr_error("Error while sending CoAP data");
        // ReportFailure(protoSending, rc);
        return rc;
    }
    tr_warn("Sent %d bytes", rc);
    return rc;
}

bool UDPClient::OpenSocket()
{
    _networkClient->Connect();
        int rc = _networkState.wait_for(
        NET_DEFS_STATE_NETWORK_CONNECTED | 
        NET_DEFS_STATE_STACK_FAILURE | 
        NET_DEFS_STATE_NETWORK_FAILURE);

    if (rc > 0 && rc == NET_DEFS_STATE_NETWORK_CONNECTED)
    {
        rc = _udpSocket.open(_networkClient->GetCellularInterface()); // calls InternetSocket::open()
        if (rc != NSAPI_ERROR_OK)
        {
            tr_error("Failed to open socket/open connection");
            return false;
        }

        _udpSocket.sigio(mbed::callback(this, &UDPClient::DataReceivedCallback));
        _udpSocket.set_blocking(false);

        return true;
    }

    tr_error("Failed to open network connection");
    return false;
}

void UDPClient::DataReceivedCallback()
{
    // ISR context!
    _dataReceived = true;
}

void UDPClient::NetworkStatusCallback(NET_DEFS_STATE status, intptr_t param)
{
    _networkState = status;
}