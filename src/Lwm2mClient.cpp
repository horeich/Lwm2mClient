/**
 * @file LWM2MClient.cpp
 * @author Niklas W., Andreas R.
 * @brief 
 * @version 0.1
 * @date 2021-08-27
 * 
 * @copyright Copyright (c) 2021, all rights reserved
 * 
 */

#include "Lwm2mClient.hpp"

#include "object_device.c"
#include "object_security.c"
#include "object_test.c"
#include "object_server.c"

#define TRACE_GROUP "LM2M"
#define LESHAN_DEMO_SERVER_IP "23.97.187.154"
#define LESHAN_DEMO_SERVER_PORT 5683

using namespace mbed;

LWM2MClient::LWM2MClient(UDPClient *udpClient, int port, const char *ip) : _udpClient(udpClient),
                                                                           _flags(),
                                                                           _lwm2mH(nullptr),
                                                                           _name("HRDevice")
{
    _socketAddress.set_port(LESHAN_DEMO_SERVER_PORT);          //MBED_CONF_LWM2M_CLIENT_PORT);
    if (!_socketAddress.set_ip_address(LESHAN_DEMO_SERVER_IP)) //MBED_CONF_LWM2M_CLIENT_IPV4))
    {
        tr_error("ip address format is invalid");
        // MBED_WARNING(MBED_MAKE_ERROR(MBED_MODULE_NETWORK_STACK, MBED_ERROR_CODE_INVALID_FORMAT), "ip");
    }

    // TODO: maybe check zero
    _objects[0] = get_security_object();
    _objects[1] = get_server_object();
    _objects[2] = get_object_device(); // WTF: get_device_object!!!
    _objects[3] = get_test_object();

    _data.securityObjP = _objects[0];
    _data.sessionP = this;

    // _data.connP.address = &_socketAddress;
    // _data.connP.udp = _udpClient; // TODO: use pointer to LWM2MClient object itself _> Done

    _lwm2mH = lwm2m_init(&_data); // sets userdata in context
    if (_lwm2mH == nullptr)
    {
        tr_error("lwm2m_init failed");
        // TODO: MBED_ERROR(); fatal error -> constructur must fail
    }

    int rc = lwm2m_configure(_lwm2mH, _name.c_str(), NULL, NULL, (uint16_t)OBJ_COUNT, _objects);
    if (rc != MBED_SUCCESS)
    {
        tr_error("lwm2m_configure failed %d", rc);
        // TODO: MBED_ERROR() -> constructur must fail
    }

    tr_warn("created lwm2m client");
}

void LWM2MClient::Run()
{
    tr_debug("LWM2MClient::%s", __func__);

    int rc = _udpClient->OpenSocket();
    if (rc < MBED_SUCCESS)
    {
        tr_error("Failed to open socket");
        return;
    }

    while (1)
    {
        struct timeval tv;
        tv.tv_sec = 60;
        tv.tv_usec = 0;

        rc = lwm2m_step(_lwm2mH, &(tv.tv_sec));
        if (rc != MBED_SUCCESS)
        {
            tr_error("lwm2m_step() failed: %d", rc);
        }

        // TODO: use flags/ callbacks
        rc = _udpClient->Poll(tv.tv_sec); // substitutes select(...) function
        if (rc > 0)
        {
            tr_info("Received message from server");
            // TODO: Get max buffer size from HF-module (1024kB)
            uint8_t buffer[MAX_PACKET_SIZE];

            SocketAddress recvAddress;
            rc = _udpClient->Receive(recvAddress, buffer, MAX_PACKET_SIZE);
            if (rc > MAX_PACKET_SIZE)
            {
                tr_error("Received packet is greater than maximum packet size!");
            }
            else if (rc > 0)
            {
                // TODO: compare send and recv address (the lwm2m lib does that for us in the lwm2m_seeion_is_equal, too)
                connection_t* connP = this;
                lwm2m_handle_packet(_lwm2mH, buffer, rc, connP);
            }
        }
    }
}

/**
 * @brief   This function is declared in liblwm2m.c and called back from registration.c
 *          to fetch a socket object
 * 
 * @param   secObjInstID 
 * @param   userData 
 * @return  void* 
 */
void *lwm2m_connect_server(uint16_t secObjInstID, void *userData)
{
    // TODO: clean up

    LWM2MClient::client_data_t *dataP;
    char *uri;
    connection_t *newConnP = NULL;

    dataP = (connection_t::client_data_t *)userData;

    uri = get_server_uri(dataP->securityObjP, secObjInstID);

    tr_warn("uri = %s", uri);

    if (uri == NULL)
        return NULL;

    fprintf(stdout, "Connecting to %s\r\n", uri);

    // parse uri in the form "coaps://[host]:[port]"
    // if (0 == strncmp(uri, "coaps://", strlen("coaps://")))
    // {
    //     host = uri+strlen("coaps://");
    // }
    // else if (0 == strncmp(uri, "coap://", strlen("coap://")))
    // {
    //     host = uri+strlen("coap://");
    // }
    // else
    // {
    //     goto exit;
    // }
    // port = strrchr(host, ':');
    // if (port == NULL) goto exit;
    // // remove brackets
    // if (host[0] == '[')
    // {
    //     host++;
    //     if (*(port - 1) == ']')
    //     {
    //         *(port - 1) = 0;
    //     }
    //     else goto exit;
    // }
    // // split strings
    // *port = 0;
    // port++;

    // newConnP = connection_create(dataP->connList, dataP->sock, host, port, dataP->addressFamily);
    // if (newConnP == NULL) {
    //     fprintf(stderr, "Connection creation failed.\r\n");
    // }
    // else {
    //     dataP->connList = newConnP;
    // }
    //dataP->connP = _data.connP;

    // return (void*)_data.connP;
    //exit:
    lwm2m_free(uri);
    //return (void *)newConnP;
    // tr_warn("%d", _data.connP->udp);
    // tr_warn("%d", _data.connP);


    //_data.connP = 

    return (void *)(dataP->sessionP); // returns client object
}

void lwm2m_close_connection(void *sessionH,
                            void *userData)
{
    // client_data_t *app_data;
    // connection_t *targetP;

    // TODO: close connection
    // app_data = (client_data_t *)userData;
    // targetP = (connection_t *)sessionH;

    // if (targetP == app_data->connList)
    // {
    //     app_data->connList = targetP->next;
    //     lwm2m_free(targetP);
    // }
    // else
    // {
    //     connection_t * parentP;

    //     parentP = app_data->connList;
    //     while (parentP != NULL && parentP->next != targetP)
    //     {
    //         parentP = parentP->next;
    //     }
    //     if (parentP != NULL)
    //     {
    //         parentP->next = targetP->next;
    //         lwm2m_free(targetP);
    //     }
    // }
}

// TODO: remove, implment in UDPClient class
// TODO: rename file
int connection_send(connection_t *connP, uint8_t *buffer, size_t length)
{
    size_t offset{0};

    tr_warn("Sending %lu bytes to [%s]:%hu", length, connP->GetAddress()->get_ip_address(), connP->GetAddress()->get_port());

    offset = 0;
    while (offset != length)
    {
        int nbSent = connP->SocketClient()->Send(*(connP->GetAddress()), buffer + offset, length - offset);
        if (nbSent == -1)
        {
            return -1;
        }
        offset += nbSent;
    }
    return 0;
}

uint8_t lwm2m_buffer_send(void *sessionH, // unused
                          uint8_t *buffer,
                          size_t length,
                          void *userdata) // unused
{

    connection_t* connP = static_cast<connection_t*>(sessionH);

    if (connP == NULL)
    {
        // fprintf(stderr, "#> failed sending %lu bytes, missing connection\r\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    if (-1 == connection_send(connP, buffer, length))
    {
        // fprintf(stderr, "#> failed sending %lu bytes\r\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    return COAP_NO_ERROR;
}

bool lwm2m_session_is_equal(void *session1,
                            void *session2,
                            void *userData)
{
    (void)userData; /* unused */

    // connection_t* s1 = static_cast<connection_t*>(session1);
    // connection_t* s2 = static_cast<connection_t*>(session2);


    // TODO: compare ip and port etc.
    //if (s1->address->get_addr().bytes.)

    return true; //(session1 == session2);
}