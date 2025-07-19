#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <libwebsockets.h>

#define BUFFER_SIZE 4096

static int interrupted = 0;
static struct lws *websocket_instance = NULL;
static char channel_name[256];

// Signal handler for graceful shutdown
static void sigint_handler(int sig) {
    interrupted = 1;
}

// Parse IRC message to extract username and message
void parse_chat_message(const char* line) {
    // Look for PRIVMSG which indicates a chat message
    if (strstr(line, "PRIVMSG") == NULL) {
        return;
    }
    
    // Extract username from :username!username@username.tmi.twitch.tv
    const char* start = strchr(line, ':');
    if (!start) return;
    start++; // Skip the ':'
    
    const char* end = strchr(start, '!');
    if (!end) return;
    
    int username_len = end - start;
    char username[256];
    strncpy(username, start, username_len);
    username[username_len] = '\0';
    
    // Find the actual message after the second ':'
    const char* msg_start = strchr(end, ':');
    if (!msg_start) return;
    msg_start++; // Skip the ':'
    
    // Remove trailing \r\n
    char message[1024];
    strcpy(message, msg_start);
    char* newline = strchr(message, '\r');
    if (newline) *newline = '\0';
    newline = strchr(message, '\n');
    if (newline) *newline = '\0';
    
    printf("[%s]: %s\n", username, message);
    fflush(stdout);
}

// WebSocket callback function
static int callback_twitch_chat(struct lws *wsi, enum lws_callback_reasons reason,
                               void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            printf("WebSocket connection established\n");
            printf("Joining channel: %s\n", channel_name);
            
            // Request capability to see chat messages
            lws_callback_on_writable(wsi);
            break;
            
        case LWS_CALLBACK_CLIENT_RECEIVE:
            {
                char *message = (char *)in;
                char buffer[BUFFER_SIZE];
                
                // Copy message and null-terminate
                size_t copy_len = len < BUFFER_SIZE - 1 ? len : BUFFER_SIZE - 1;
                memcpy(buffer, message, copy_len);
                buffer[copy_len] = '\0';
                
                printf("Received: %s\n", buffer); // Debug output
                
                // Handle PING/PONG
                if (strstr(buffer, "PING")) {
                    printf("Received PING, sending PONG\n");
                    lws_callback_on_writable(wsi);
                } else if (strstr(buffer, "Welcome") || strstr(buffer, "001")) {
                    printf("Successfully authenticated\n");
                    // Continue with next step
                    lws_callback_on_writable(wsi);
                } else if (strstr(buffer, "CAP * ACK")) {
                    printf("Capabilities acknowledged\n");
                    // Continue with authentication
                    lws_callback_on_writable(wsi);
                } else {
                    // Parse and print chat messages
                    char *line = strtok(buffer, "\r\n");
                    while (line != NULL) {
                        parse_chat_message(line);
                        line = strtok(NULL, "\r\n");
                    }
                }
            }
            break;
            
        case LWS_CALLBACK_CLIENT_WRITEABLE:
            {
                static int step = 0;
                
                if (step == 0) {
                    // Send CAP REQ to request capabilities
                    char cap_msg[] = "CAP REQ :twitch.tv/tags twitch.tv/commands\r\n";
                    
                    unsigned char buf[LWS_PRE + 512];
                    unsigned char *p = &buf[LWS_PRE];
                    int len = snprintf((char *)p, 512, "%s", cap_msg);
                    
                    lws_write(wsi, p, len, LWS_WRITE_TEXT);
                    step = 1;
                    printf("Sent CAP REQ\n");
                    
                } else if (step == 1) {
                    // Send authentication (anonymous)
                    char auth_msg[] = "PASS SCHMOOPIIE\r\nNICK justinfan12345\r\n";
                    
                    unsigned char buf[LWS_PRE + 512];
                    unsigned char *p = &buf[LWS_PRE];
                    int len = snprintf((char *)p, 512, "%s", auth_msg);
                    
                    lws_write(wsi, p, len, LWS_WRITE_TEXT);
                    step = 2;
                    printf("Sent authentication\n");
                    
                } else if (step == 2) {
                    // Join channel
                    char join_msg[256];
                    snprintf(join_msg, sizeof(join_msg), "JOIN #%s\r\n", channel_name);
                    
                    unsigned char buf[LWS_PRE + 512];
                    unsigned char *p = &buf[LWS_PRE];
                    int len = snprintf((char *)p, 512, "%s", join_msg);
                    
                    lws_write(wsi, p, len, LWS_WRITE_TEXT);
                    step = 3;
                    printf("Sent JOIN command\n");
                    printf("Listening for chat messages...\n\n");
                    
                } else if (step == 3) {
                    // Handle PONG response to PING
                    char pong_msg[] = "PONG :tmi.twitch.tv\r\n";
                    
                    unsigned char buf[LWS_PRE + 64];
                    unsigned char *p = &buf[LWS_PRE];
                    int len = snprintf((char *)p, 64, "%s", pong_msg);
                    
                    lws_write(wsi, p, len, LWS_WRITE_TEXT);
                    printf("Sent PONG\n");
                }
            }
            break;
            
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            printf("Connection error: %s\n", in ? (char *)in : "unknown");
            interrupted = 1;
            break;
            
        case LWS_CALLBACK_CLOSED:
            printf("Connection closed\n");
            interrupted = 1;
            break;
            
        case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
            return 0;
            
        default:
            break;
    }
    
    return 0;
}

// Protocol definition
static struct lws_protocols protocols[] = {
    {
        "irc",                          // Protocol name
        callback_twitch_chat,           // Callback function
        0,                              // Per-session data size
        BUFFER_SIZE,                    // RX buffer size
    },
    { NULL, NULL, 0, 0 }               // Terminator
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <channel_name>\n", argv[0]);
        printf("Example: %s shroud\n", argv[0]);
        return 1;
    }
    
    strncpy(channel_name, argv[1], sizeof(channel_name) - 1);
    channel_name[sizeof(channel_name) - 1] = '\0';
    
    // Set up signal handler
    signal(SIGINT, sigint_handler);
    
    // Create libwebsockets context
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    
    // Reduce log level to minimize debug output
    lws_set_log_level(LLL_USER | LLL_ERR | LLL_WARN, NULL);
    
    struct lws_context *context = lws_create_context(&info);
    if (!context) {
        printf("Failed to create libwebsockets context\n");
        return 1;
    }
    
    // Set up connection info
    struct lws_client_connect_info ccinfo;
    memset(&ccinfo, 0, sizeof(ccinfo));
    
    ccinfo.context = context;
    ccinfo.address = "irc-ws.chat.twitch.tv";
    ccinfo.port = 443;
    ccinfo.path = "/";
    ccinfo.host = ccinfo.address;
    ccinfo.origin = ccinfo.address;
    ccinfo.protocol = "irc";
    ccinfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;
    
    printf("Connecting to Twitch WebSocket server...\n");
    
    // Connect to WebSocket
    websocket_instance = lws_client_connect_via_info(&ccinfo);
    if (!websocket_instance) {
        printf("Failed to connect to WebSocket\n");
        lws_context_destroy(context);
        return 1;
    }
    
    // Main event loop
    while (!interrupted) {
        lws_service(context, 100);
    }
    
    printf("\nShutting down...\n");
    
    // Cleanup
    lws_context_destroy(context);
    
    return 0;
}
