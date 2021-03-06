#include "Video.h"
#include "Input.h"
#include "Config.h"
#include "Packets.h"
#include "Restore.h"
#include "Ping.h"
#include "Sockets.h"
#include "Overview.h"
#include "Units.h"
#include "Args.h"
#include "Util.h"

#include <time.h>

static Overview WaitInLobby(const Video video, const Sock sock)
{
    int32_t loops = 0;
    Overview overview = Overview_Init(video.xres, video.yres);
    for(Input input = Input_Ready(); !input.done; input = Input_Pump(input))
    {
        UTIL_TCP_SEND(sock.server, &overview);
        const Packet packet = Packet_Get(sock);
        if(Packet_IsAlive(packet))
        {
            overview.color = (Color) packet.client_id;
            Video_PrintLobby(video, packet.users_connected, packet.users, overview.color, loops++);
            if(packet.game_running)
            {
                overview.users = packet.users;
                overview.map_power = packet.map_power;
                overview.seed = packet.seed;
                break;
            }
        }
        SDL_Delay(CONFIG_MAIN_LOOP_SPEED_MS);
    }
    return overview;
}

static void Play(const Video video, const Data data, const Args args)
{
    Ping_Init(args);
    const Sock sock = Sock_Connect(args.host, args.port);
    const Sock reset = Sock_Connect(args.host, args.port_reset);
    Overview overview = WaitInLobby(video, sock);
    Util_Srand(overview.seed);
    const Map map = Map_Make(overview.map_power, data.terrain);
    const Grid grid = Grid_Make(map.size, map.tile_width, map.tile_height);
    Units units  = Units_New(grid.size, video.cpu_count, CONFIG_UNITS_MAX, overview.color, args.civ);
    Units floats = Units_New(grid.size, video.cpu_count, CONFIG_UNITS_FLOAT_BUFFER, overview.color, args.civ);
    units = Units_Generate(units, map, grid, data.graphics, overview.users);
    overview.pan = Units_GetFirstTownCenterPan(units, grid);
    Packets packets = Packets_Init();
    int32_t cycles = 0;
    for(Input input = Input_Ready(); !input.done; input = Input_Pump(input))
    {
        const int32_t t0 = SDL_GetTicks();
        const Field field = Units_Field(units, map);
        const int32_t size = Packets_Size(packets);
        const uint64_t parity = Units_Xor(units);
        const int32_t ping = Ping_Get();
        overview = Overview_Update(overview, input, parity, cycles, size, units.share, ping);
        if (UTIL_TCP_SEND(sock.server, &overview) != sizeof(overview)) {
            fprintf(stderr, "Failed to send overview update to server\n");
            return;
        }
        const Packet packet = Packet_Get(sock);
        if(packet.is_out_of_sync)
        {
            if(packet.client_id == COLOR_BLU) // XXX. MUST ASK SERVER FOR FIRST AVAIL PLAYER. BLUE IS OK FOR NOW.
            {
                Units_FreeAllPaths(units); // PATHS ARE TOO RISKY TO RESTORE.
                const Restore restore = Units_PackRestore(units, cycles);
                Restore_Send(restore, reset.server);
            }
            const Restore restore = Restore_Recv(reset.server);
            units = Units_UnpackRestore(units, restore, grid);
            cycles = restore.cycles;
            Util_Srand(overview.seed);
            packets = Packets_Clear(packets); // DISPOSES USER SPACE BUFFERING.
            Packet_Flush(sock); // DISPOSES KERNEL SPACE BUFFERING. THIS NUKES ALL PACKETS.
            Restore_Free(restore);
        }
        else
        {
            if(Packet_IsReady(packet))
                packets = Packets_Queue(packets, packet);
            packets = Packets_ClearStale(packets, cycles);
            while(Packets_MustExecute(packets, cycles))
            {
                Packet dequeued;
                packets = Packets_Dequeue(packets, &dequeued);
                units = Units_PacketService(units, data.graphics, dequeued, grid, map, field);
            }
            units = Units_Caretake(units, data.graphics, grid, map, field);
            cycles++;
            if(packet.control != PACKET_CONTROL_SPEED_UP)
            {
                floats = Units_Float(floats, units, data.graphics, overview, grid, map, units.stamp[units.color].motive);
                Video_Draw(video, data, map, units, floats, overview, grid);
                const int32_t t1 = SDL_GetTicks();
                Video_Render(video, units, overview, map, t1 - t0, cycles, ping);
                const int32_t t2 = SDL_GetTicks();
                const int32_t ms = CONFIG_MAIN_LOOP_SPEED_MS - (t2 - t0);
                if(ms > 0)
                    SDL_Delay(ms);
            }
        }
        Field_Free(field);
    }
    Units_Free(floats);
    Units_Free(units);
    Packets_Free(packets);
    Sock_Disconnect(sock);
    Sock_Disconnect(reset);
    Ping_Shutdown();
    Map_Free(map);
}

static void RunClient(const Args args)
{
    SDL_Init(SDL_INIT_VIDEO);
    const Video video = Video_Setup(args.xres, args.yres, CONFIG_MAIN_GAME_NAME);
    Video_PrintLobby(video, 0, 0, COLOR_GAIA, 0);
    const Data data = Data_Load(args.path);
    if (data.loaded) {
        args.demo
            ? Video_RenderDataDemo(video, data, args.color)
            : Play(video, data, args);
    } else {
        fprintf(stderr, "Failed to load data\n");
    }
    Video_Free(video);
    Data_Free(data);
    SDL_Quit();
}

static void RunServer(const Args args)
{
    srand(time(NULL));
    Sockets sockets = Sockets_Init(args.port);
    Sockets pings = Sockets_Init(args.port_ping);
    Sockets resets = Sockets_Init(args.port_reset);
    Cache cache = Cache_Init(args.users, args.map_power);
    for(int32_t cycles = 0; true; cycles++)
    {
        const int32_t t0 = SDL_GetTicks();
        sockets = Sockets_Accept(sockets);
        pings = Sockets_Accept(pings);
        resets = Sockets_Accept(resets);
        sockets = Sockets_Recv(sockets, &cache);
        Sockets_Send(sockets, &cache, cycles, args.quiet);
        Sockets_Ping(pings);
        Sockets_Reset(resets, &cache);
        const int32_t t1 = SDL_GetTicks();
        const int32_t ms = 10 - (t1 - t0);
        if(ms > 0)
            SDL_Delay(ms);
    }
    Sockets_Free(resets);
    Sockets_Free(pings);
    Sockets_Free(sockets);
}

int main(const int argc, const char* argv[])
{
    SDLNet_Init();
    const Args args = Args_Parse(argc, argv);
    args.is_server
        ? RunServer(args)
        : RunClient(args);
    SDLNet_Quit();
}
