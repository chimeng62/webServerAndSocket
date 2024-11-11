# WebServerAndSocket Library

A simple and flexible library designed for ESP32 devices to integrate an HTTP server and WebSocket functionality in a single class. This library provides a streamlined way to handle WebSocket connections, manage mDNS for easy hostname access, and serve dynamic HTML content over HTTP.

## Features
- **HTTP Server**: Serves static HTML content on the root URL (`/`).
- **WebSocket Support**: Allows bi-directional communication with WebSocket clients connected at `/ws`.
- **mDNS Support**: Sets up mDNS with a custom hostname, simplifying network access.
- **Event Callbacks**: Customizable callbacks for handling WebSocket messages and events.
- **Client Tracking**: Monitors and provides the count of active WebSocket clients.

## Installation
Include the following dependencies in your project:
- [`AsyncTCP`](https://github.com/me-no-dev/AsyncTCP)
- [`ESPAsyncWebServer`](https://github.com/me-no-dev/ESPAsyncWebServer)
- [`ESPmDNS`](https://www.arduino.cc/reference/en/libraries/esp-mdns/)

## Usage

### 1. Basic Setup

Create an instance of `WebServerAndSocket`, specifying the HTTP port:
```cpp
#include <WebServerAndSocket.h>

WebServerAndSocket server(80);  // Set up on HTTP port 80

void setup() {
    Serial.begin(115200);

    server.setMdnsName("my-device");   // Optional: Set custom mDNS name
    server.setHtmlContent("<h1>Hello, ESP32!</h1>");  // Set HTML content for HTTP server
    server.begin();                    // Start HTTP server and WebSocket

    server.setMessageHandler(onMessageReceived);   // Set message handler
    server.setWebSocketEventCallback(onEvent);     // Set WebSocket event callback
}

void loop() {
    server.cleanupWebsocketClients();  // Call this regularly to handle WebSocket clients
}