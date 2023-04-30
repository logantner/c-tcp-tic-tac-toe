#include "../lib/server_controller.h"
#include "../lib/config.h"

int main(int argc, char** argv) {
    char* port =    argc < 3 ? SERVER_PORT : argv[2];
    char* address = argc < 2 ? SERVER_IP   : argv[1];

    return run_server(address, port);
}