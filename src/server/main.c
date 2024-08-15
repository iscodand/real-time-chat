#include <libwebsockets.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_CLIENTS 100
#define MAX_MESSAGE_SIZE 128
#define MAX_USERNAME_SIZE 128

struct per_session_data
{
    struct lws *wsi;                // sessao
    char username[32];              // username
    char roomId[32];                // sala
    char message[MAX_MESSAGE_SIZE]; // mensagem
    size_t len;                     // tamanho em bytes
};

// create max clients per room
struct per_session_data *clients[MAX_CLIENTS];

// create client count per room
size_t client_count = 0;

void on_user_connected(struct per_session_data *pss)
{
    char notification_message[MAX_MESSAGE_SIZE];

    snprintf(notification_message, sizeof(notification_message), "um cabra se conectou!%zu\n", client_count);

    for (size_t i = 0; i < client_count; ++i)
    {
        clients[i]->len = pss->len;
        strcpy(clients[i]->message, pss->message);
        lws_callback_on_writable(clients[i]->wsi);
    }
}

void send_message_to_users(struct per_session_data *pss, char *message, int message_len, bool send_just_to_connected_room)
{
    if (message_len > MAX_MESSAGE_SIZE)
    {
        printf("Limite máximo de caracteres: 256");
    }
    else
    {
        if (send_just_to_connected_room)
        {
            for (size_t i = 0; i < client_count; ++i)
            {
                // valida a mensagem sera enviada apenas para os usuarios da sala
                if (clients[i] != pss && strcmp(clients[i]->roomId, pss->roomId) == 0)
                {
                    clients[i]->len = strlen(message);
                    strcpy(clients[i]->message, message);
                    lws_callback_on_writable(clients[i]->wsi);
                }
            }
        }
        else
        {
            for (size_t i = 0; i < client_count; ++i)
            {
                clients[i]->len = strlen(message);
                strcpy(clients[i]->message, message);
                lws_callback_on_writable(clients[i]->wsi);
            }
        }
    }
}

void callback_receive(struct per_session_data *pss, void *in, size_t len)
{
    // isco: first message is user username
    if (pss->username[0] == '\0')
    {
        strncpy(pss->username, (const char *)in, len);
        pss->username[len] = '\0';

        char message[MAX_MESSAGE_SIZE];
        snprintf(message, sizeof(message), "%s acabou de se conectar! \n", pss->username);
        send_message_to_users(pss, message, strlen(message), true);
    }

    // isco: second message is room
    else if (pss->roomId[0] == '\0')
    {
        strncpy(pss->roomId, (const char *)in, len);
        pss->roomId[len] = '\0';

        char message[MAX_MESSAGE_SIZE];
        snprintf(message, sizeof(message), "%s entrou na sala (%s)! \n", pss->username, pss->roomId);
        send_message_to_users(pss, message, strlen(message), true);
    }

    // isco: after this, only messages to others users in the room
    else
    {
        pss->len = len < MAX_MESSAGE_SIZE ? len : MAX_MESSAGE_SIZE - 1;
        memcpy(pss->message, in, pss->len);
        pss->message[pss->len] = '\0';
        send_message_to_users(pss, pss->message, strlen(pss->message), true);
    }
}

void callback_closed(struct per_session_data *pss)
{
    for (size_t i = 0; i < client_count; ++i)
    {
        if (clients[i] == pss)
        {
            clients[i] = clients[--client_count];
            break;
        }
    }

    char message[MAX_MESSAGE_SIZE];
    snprintf(message, sizeof(message), "%s saiu do chat! \n", pss->username);
    send_message_to_users(pss, message, strlen(message), false);
}

void callback_established(struct per_session_data *pss, struct lws *wsi)
{
    if (client_count < MAX_CLIENTS)
    {
        pss->wsi = wsi;
        clients[client_count++] = pss;
        on_user_connected(pss);
    }
    else
    {
        char message[MAX_MESSAGE_SIZE];
        snprintf(message, sizeof(message), "o servidor está lotado (%zu usuários online)! espere um pouco e tente novamente", client_count);
        send_message_to_users(pss, message, strlen(message), false);
    }
}

void callback_writeable(struct per_session_data *pss, struct lws *wsi)
{
    if (pss->len > 0)
    {
        unsigned char buf[LWS_PRE + 128];
        unsigned char *p = &buf[LWS_PRE];
        size_t n = sprintf((char *)p, "%s", pss->message);
        lws_write(wsi, p, n, LWS_WRITE_TEXT);
        lws_write(wsi, p, n, LWS_WRITE_PING);
        pss->len = 0; // limpa o buffer
    }
}

int callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    struct per_session_data *pss = (struct per_session_data *)user;

    switch (reason)
    {
    case LWS_CALLBACK_ESTABLISHED:
        callback_established(pss, wsi);
        break;

    case LWS_CALLBACK_RECEIVE:
        callback_receive(pss, in, len);
        break;

    case LWS_CALLBACK_SERVER_WRITEABLE:
        callback_writeable(pss, wsi);
        break;

    case LWS_CALLBACK_CLOSED:
        callback_closed(pss);
        break;

    default:
        break;
    }

    return 0;
}

int main(int argc, char **argv)
{
    static struct lws_protocols protocols[] = {
        {"demo_protocol",
         callback,
         sizeof(struct per_session_data),
         0,
         0, NULL, 0},
        {NULL, NULL, 0, 0}};

    struct lws_context_creation_info info =
        {
            .port = 3001,
            .protocols = protocols,
            .options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT};

    struct lws_context *context = lws_create_context(&info);

    if (!context)
    {
        printf("Failed to create websocket context. \n");
    }

    while (1)
    {
        lws_service(context, 50);
    }

    lws_context_destroy(context);

    return 0;
}