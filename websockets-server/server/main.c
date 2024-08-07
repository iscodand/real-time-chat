#include <libwebsockets.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_CLIENTS 100
#define MAX_MESSAGE_SIZE 128

struct per_session_data
{
    struct lws *wsi;                // ws da sessao
    char room[32];                  // sala do cliente
    char message[MAX_MESSAGE_SIZE]; // mensagem recebida
    size_t len;                     // tamanho da mensagem recebida
};

struct per_session_data *clients[MAX_CLIENTS];
size_t client_count = 0;

int callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    struct per_session_data *pss = (struct per_session_data *)user;

    switch (reason)
    {
    case LWS_CALLBACK_ESTABLISHED:
        if (client_count < MAX_CLIENTS)
        {
            pss->wsi = wsi;
            clients[client_count++] = pss;
            strcpy(pss->room, "defaultRoom");
            printf("um viado foi conectado! viados totais na sala: %zu\n", client_count);
        }
        else
        {
            printf("ja tem gente demais na sala! espera um pouco ae! \n");
            return -1;
        }

        break;

    case LWS_CALLBACK_RECEIVE:
        pss->len = len < MAX_MESSAGE_SIZE ? len : MAX_MESSAGE_SIZE - 1;
        memcpy(pss->message, in, pss->len);
        pss->message[pss->len] = '\0';
        printf("mensagem recebida sua bichinha: %s\n", pss->message);

        for (size_t i = 0; i < client_count; ++i)
        {
            if (clients[i] != pss && strcmp(clients[i]->room, pss->room) == 0)
            {
                clients[i]->len = pss->len;
                strcpy(clients[i]->message, pss->message);
                lws_callback_on_writable(clients[i]->wsi);
            }
        }

        break;

    case LWS_CALLBACK_SERVER_WRITEABLE:
        if (pss->len > 0)
        {
            unsigned char buf[LWS_PRE + 128];
            unsigned char *p = &buf[LWS_PRE];
            size_t n = sprintf((char *)p, "%s", pss->message);
            lws_write(wsi, p, n, LWS_WRITE_TEXT);
            pss->len = 0; // limpa o buffer
        }

        break;

    case LWS_CALLBACK_CLOSED:
        for (size_t i = 0; i < client_count; ++i)
        {
            if (clients[i] == pss)
            {
                clients[i] = clients[--client_count];
                break;
            }
        }

        printf("um qualhira saiu, agora temos apenas essa quantidade na sala: %zu\n", client_count);

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