#include "Ping.h"

#include "Sock.h"
#include "Args.h"
#include "Util.h"

#include <SDL2/SDL_mutex.h>

static SDL_mutex* mutex;

static int32_t ping;

static void Set(const int32_t dt)
{
    if(SDL_TryLockMutex(mutex) == 0)
    {
        ping = dt;
        SDL_UnlockMutex(mutex);
    }
}

int32_t Ping_Get(void)
{
    int32_t out = -1;
    if(SDL_TryLockMutex(mutex) == 0)
    {
        out = ping;
        SDL_UnlockMutex(mutex);
    }
    return out;
}

static int32_t Ping(void* const data)
{
    Args* args = (Args*) data;
    const Sock pinger = Sock_Connect(args->host, args->port_ping);
    if (!pinger.connected) {
        fprintf(stderr, "Failed to connect to server on PING channel\n");
        return;
    }
    while(true)
    {
        const int32_t message = 0xCAFEBABE;
        int32_t temp;
        const int32_t t0 = SDL_GetTicks();
        UTIL_TCP_SEND(pinger.server, &message);
        UTIL_TCP_RECV(pinger.server, &temp);
        const int32_t t1 = SDL_GetTicks();
        if(temp == message)
            Set(t1 - t0);
    }
    Sock_Disconnect(pinger);
    return 0;
}

void Ping_Init(const Args args)
{
    mutex = SDL_CreateMutex();
    SDL_CreateThread(Ping, "N/A", (void*) &args);
}

void Ping_Shutdown(void)
{
    SDL_DestroyMutex(mutex);
}
