/*
Copyright (C) 2024 Ahmad Ismail
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

enum service_status
{
    SUCCESS,
    VALUE_TOO_SMALL,
    VALUE_TOO_LARGE,
    SYSTEM_FAILURE
};

int main(int argc, char **argv)
{
    if (argc == 2)
    {
        if (strlen(argv[1]) > 3)
        {
            fputs("Please input up to 3 digits\n", stderr);
            return 1;
        }
        int threshold = atoi(argv[1]);
        if (threshold < 1 || threshold > 100)
        {
            fputs("Not a valid battery threshold\n", stderr);
            return 1;
        }
        struct sockaddr_un srv_socket;
        int client_fd;

        /* Create local socket. */

        client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (client_fd == -1)
        {
            fputs("Failed to create socket!\n", stderr);
            return 1;
        }

        /*
         * For portability clear the whole structure, since some
         * implementations have additional (nonstandard) fields in
         * the structure. (this is according to man unix(7))
         */
        memset(&srv_socket, 0, sizeof(srv_socket));

        srv_socket.sun_family = AF_UNIX;
        strcpy(srv_socket.sun_path, "/run/batteryd");

        int status = connect(client_fd, (struct sockaddr *)&srv_socket, sizeof(srv_socket));
        if (status == -1)
        {
            perror("Failed to connect to batteryd service");
            return 1;
        }
        if (write(client_fd, &threshold, 1) < 1)
        {
            fputs("Failed to write to the server socket!\n", stderr);
            return 1;
        }
        int8_t server_response;
        read(client_fd, &server_response, 1);
        switch (server_response)
        {
        case SUCCESS:
            printf("Battery charge threshold set to %d\n", threshold);
            break;
        case VALUE_TOO_SMALL:
            fputs("Failed to set threshold: value too small, try value > 49\n", stderr);
            break;
        // This should never happen in theory as we validate the threshold earlier
        case VALUE_TOO_LARGE:
            fputs("Failed to set threshold: value too large, try value <= 100\n", stderr);
            break;
        case SYSTEM_FAILURE:
            fputs("Something went wrong with the service, check batteryd's logn\n", stderr);
            break;
        default:
            fputs("Unexpected response, please check batteryd service for malfunction.\n", stderr);
            break;
        }
        return server_response;
    }
}