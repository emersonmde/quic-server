#include <msquic.h>

#define ALPN "echo"
#define PORT 4433

HQUIC Registration;
HQUIC Configuration;
HQUIC Listener;

const QUIC_API_TABLE* MsQuic;
const QUIC_BUFFER Alpn = { sizeof("sample") - 1, (uint8_t*)"sample" };

QUIC_STATUS
StreamCallback(
    HQUIC Stream,
    void* Context,
    QUIC_STREAM_EVENT* Event
    )
{

    switch (Event->Type) {
    case QUIC_STREAM_EVENT_RECEIVE:
        MsQuic->StreamSend(
            Stream,
            Event->RECEIVE.Buffers,
            Event->RECEIVE.BufferCount,
            QUIC_SEND_FLAG_NONE,
            NULL
        );
        break;
    // case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
    case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
        MsQuic->StreamShutdown(
            Stream,
            QUIC_STREAM_SHUTDOWN_FLAG_GRACEFUL,
            0
        );
        break;
    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS
ConnectionCallback(
    HQUIC Connection,
    void* Context,
    QUIC_CONNECTION_EVENT* Event
    )
{
    switch (Event->Type) {
    case QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED:
        MsQuic->SetCallbackHandler(
            Event->PEER_STREAM_STARTED.Stream,
            (void*)StreamCallback,
            NULL
        );
        break;
    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS
ListenerCallback(
    HQUIC Listener,
    void* Context,
    QUIC_LISTENER_EVENT* Event
    )
{
    switch (Event->Type) {
    case QUIC_LISTENER_EVENT_NEW_CONNECTION:
        MsQuic->SetCallbackHandler(
            Event->NEW_CONNECTION.Connection,
            (void*)ConnectionCallback,
            NULL
        );
        MsQuic->ConnectionSetConfiguration(
            Event->NEW_CONNECTION.Connection,
            Configuration
        );
        break;
    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}

int main()
{
    QUIC_CREDENTIAL_CONFIG CredConfig = {
        .Type = QUIC_CREDENTIAL_TYPE_NONE,
        .Flags = QUIC_CREDENTIAL_FLAG_NONE
    };

    MsQuicOpen2(&MsQuic);
    MsQuic->RegistrationOpen(NULL, &Registration);

    QUIC_SETTINGS Settings = { .IsSet.IdleTimeoutMs = TRUE };
    MsQuic->ConfigurationOpen(Registration, &Alpn, 1, &Settings, sizeof(Settings), NULL, &Configuration);
    MsQuic->ConfigurationLoadCredential(Configuration, &CredConfig);
    MsQuic->ListenerOpen(Registration, ListenerCallback, NULL, &Listener);


    // QUIC_ADDR addr = { .Ipv4 = QUIC_ADDR_V4_ANY, .Ipv4.sin_port = htons(PORT) };
    QUIC_ADDR addr = {0};
    // TODO Alpn?
    MsQuic->ListenerStart(Listener, &Alpn, 1, &addr);

    printf("Server is running. Press Enter to exit...\n");
    getchar();

    MsQuic->ListenerClose(Listener);
    MsQuic->ConfigurationClose(Configuration);
    MsQuic->RegistrationClose(Registration);
    MsQuicClose(MsQuic);

    return 0;
}
