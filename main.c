#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msquic/src/inc/msquic.h"
#include "msquic/src/inc/msquicp.h"
#include <proto/message.pb-c.h>

const QUIC_API_TABLE* MsQuic;

// Callback function for handling incoming QUIC streams
QUIC_STATUS QUIC_API StreamCallback(_In_ HQUIC StreamHandle, _In_opt_ void* Context, _Inout_ QUIC_STREAM_EVENT* Event)
{
    switch (Event->Type) {
        case QUIC_STREAM_EVENT_RECEIVE:
        {
            // Process received data
            QUIC_BUFFER* ReceiveBuffer = &Event->RECEIVE.Buffers[0];
            if (ReceiveBuffer->Length > 0) {
                // Deserialize the protobuf message
                Quicserver__Message* message = quicserver__message__unpack(NULL, ReceiveBuffer->Length, ReceiveBuffer->Buffer);
                if (message != NULL) {
                    // Process the received message
                    printf("Received message:\n");
                    printf("  Key: %s\n", message->key);
                    printf("  Value: %.*s\n", (int)message->value.len, (char*)message->value.data);

                    // Cleanup the unpacked protobuf message
                    quicserver__message__free_unpacked(message, NULL);
                } else {
                    printf("Failed to unpack the protobuf message\n");
                }
            }

            // Close the stream
            MsQuic->StreamShutdown(StreamHandle, QUIC_STREAM_SHUTDOWN_FLAG_NONE, 0);
            break;
        }

        default:
            break;
    }

    return QUIC_STATUS_SUCCESS;
}

int main()
{
    QUIC_STATUS Status;
    HQUIC Registration = NULL;
    HQUIC Configuration = NULL;
    HQUIC Listener = NULL;
    QUIC_SETTINGS Settings;
    QUIC_BUFFER AlpnBuffer;

    // Create the QUIC registration
    Status = MsQuic->RegistrationOpen(&MsQuicRegistrationCallbacks, &Registration);
    if (QUIC_FAILED(Status)) {
        printf("Failed to open QUIC registration: 0x%x\n", Status);
        return -1;
    }

    // Create the QUIC configuration
    Status = MsQuic->ConfigurationOpen(Registration, &AlpnBuffer, 1, NULL, &Configuration);
    if (QUIC_FAILED(Status)) {
        printf("Failed to open QUIC configuration: 0x%x\n", Status);
        return -1;
    }

    // Set QUIC settings
    QuicZeroMemory(&Settings, sizeof(Settings));
    Settings.IdleTimeoutMs = 60000;
    Settings.IsSet.IdleTimeoutMs = TRUE;
    Status = MsQuic->ConfigurationSetSettings(Configuration, &Settings);
    if (QUIC_FAILED(Status)) {
        printf("Failed to set QUIC configuration settings: 0x%x\n", Status);
        return -1;
    }

    // Open a QUIC listener
    Status = MsQuic->ListenerOpen(Registration, StreamCallback, NULL, &Listener);
    if (QUIC_FAILED(Status)) {
        printf("Failed to open QUIC listener: 0x%x\n", Status);
        return -1;
    }

    // Start listening on the specified address and port
    QUIC_ADDR Address;
    QuicAddrSetFamily(&Address, QUIC_ADDRESS_FAMILY_UNSPEC);
    QuicAddrSetPort(&Address, 12345);
    Status = MsQuic->ListenerStart(Listener, Configuration, &Address);
    if (QUIC_FAILED(Status)) {
        printf("Failed to start QUIC listener: 0x%x\n", Status);
        return -1;
    }

    // Wait for incoming connections and events
    while (1) {
        QUIC_EVENT Event;
        QUIC_HANDLE Handles[1];
        QUIC_STREAM* Stream;
        QuicEventInitialize(&Event, TRUE, FALSE);
        Handles[0] = Event.Handle;
        Status = MsQuic->WaitForEvent(Registration, Handles, 1, &Event);
        if (QUIC_FAILED(Status)) {
            printf("Failed to wait for event: 0x%x\n", Status);
            break;
        }
        // Accept new connections
        Status = MsQuic->ListenerAccept(Listener, NULL, NULL, &Stream);
        if (QUIC_FAILED(Status)) {
            printf("Failed to accept connection: 0x%x\n", Status);
            break;
        }
        // Start receiving on the stream
        Status = MsQuic->StreamReceiveSetEnabled(Stream, TRUE);
        if (QUIC_FAILED(Status)) {
            printf("Failed to set receive enabled: 0x%x\n", Status);
            break;
        }
    }

    // Cleanup
    if (Listener != NULL) {
        MsQuic->ListenerClose(Listener);
    }
    if (Configuration != NULL) {
        MsQuic->ConfigurationClose(Configuration);
    }
    if (Registration != NULL) {
        MsQuic->RegistrationClose(Registration);
    }

    return 0;
}
