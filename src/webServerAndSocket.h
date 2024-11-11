#ifndef WEBSERVERANDSOCKET_H
#define WEBSERVERANDSOCKET_H

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>  // Include mDNS library

class WebServerAndSocket {
public:
    // Define a function pointer type for the message handler callback
    typedef void (*MessageHandlerCallback)(const String& message);
    typedef void (*WebSocketEventCallback)();  // Define a function pointer type for WebSocket event callback

    // Constructor with HTTP port and optional mDNS setup
    WebServerAndSocket(int httpPort)
        : server(httpPort), messageHandler(nullptr), eventCallback(nullptr), webSocketConnectedClients(0), ws("/ws"), mdnsName("") {  // Default empty name, no mDNS started
        
        // WebSocket event handler
        ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
            this->onEvent(server, client, type, arg, data, len);
        });

        // Attach WebSocket to the HTTP server
        server.addHandler(&ws);
    }

    // Initialize WebSocket and HTTP server
    void begin() {
        
        // HTTP server setup
        server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
            // Serve the HTML content passed by the user
            request->send(200, "text/html", htmlContent);
        });

        // Start HTTP server
        server.begin();
        Serial.println("HTTP server started...");
    }

    // Set the mDNS hostname dynamically from the client
    void setMdnsName(const String& newMdnsName) {
        mdnsName = newMdnsName;
        if (mdnsName.length() > 0) {
            if (MDNS.begin(mdnsName.c_str())) {  // Initialize mDNS with the custom hostname
                Serial.println("mDNS responder started for hostname: " + mdnsName);
                MDNS.addService("http", "tcp", 80);  // Announce web server on port 80
            } else {
                Serial.println("Error starting mDNS");
            }
        } else {
            Serial.println("mDNS not started: hostname is empty.");
        }
    }

    // Call this in the main loop
    void cleanupWebsocketClients() {
        // WebSocket loop to handle events
        ws.cleanupClients();  // Clean up disconnected clients
    }

    // Set the message handler callback function
    void setMessageHandler(MessageHandlerCallback handler) {
        messageHandler = handler;  // Set the custom message handler callback
    }

    void setWebSocketEventCallback(WebSocketEventCallback callback) {
        eventCallback = callback;  // Set the custom WebSocket event callback
    }
    // Set the HTML content to be served by HTTP server
    void setHtmlContent(const char*content) {
        htmlContent = content;  // Set the HTML content to be served by the HTTP server
    }

    // Notify all WebSocket clients with a new message
    void notifyClients(const String& config) {
        ws.textAll(config);  // Send a message to all connected clients
    }

    // Public method to get the count of WebSocket connections
    int getWebSocketClientCount() const {
        return webSocketConnectedClients;  // Return the number of connected WebSocket clients
    }

private:
    AsyncWebSocket ws;         // WebSocket server instance with path "/ws"
    AsyncWebServer server;     // HTTP server instance
    MessageHandlerCallback messageHandler;  // Store the message handler callback
    WebSocketEventCallback eventCallback;  // Store the WebSocket event callback
    String htmlContent;        // HTML content for the HTTP server
    int webSocketConnectedClients;  // Track the number of connected WebSocket clients
    String mdnsName;           // Store the mDNS name (hostname)

    // Handle WebSocket events (connection, disconnection, data, etc.)
    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        switch (type) {
            case WS_EVT_CONNECT:
                webSocketConnectedClients++;  // Increment the client counter
                Serial.printf("WebSocket Client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
                break;
            case WS_EVT_DISCONNECT:
                webSocketConnectedClients--;  // Decrement the client counter
                Serial.printf("WebSocket Client #%u disconnected\n", client->id());
                break;
            case WS_EVT_DATA:
                // Handle incoming WebSocket data
                handleWebSocketMessage(arg, data, len);
                break;
            case WS_EVT_PONG:
            case WS_EVT_ERROR:
                break;
        }
        if (eventCallback != nullptr) {
            eventCallback();  // Call the WebSocket event callback if set
        }        
    }

    // Process the incoming WebSocket message
    void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            // Handle WebSocket text message
            String message = (char *)data;
            message.trim();  // Remove leading/trailing whitespace

            Serial.println("Received message from client: " + message);
            if (messageHandler != nullptr) {
                messageHandler(message);  // Call the user-defined message handler
            }
        }
    }
};

#endif // WEBSERVERANDSOCKET_H
