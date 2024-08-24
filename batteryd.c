/*
Copyright (C) 2024 Ahmad Ismail
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>
#include <grp.h>
#include <signal.h>

enum service_status
{
    SUCCESS,
    VALUE_TOO_SMALL,
    VALUE_TOO_LARGE,
    SYSTEM_FAILURE
};

// The socket file descriptor should be global for easy closing on termination
int srv_fd;

void clean_exit(int signum)
{
    if (signum == SIGINT || signum == SIGTERM)
    {
        close(srv_fd);
        exit(0);
    }
}

int set_battery_charge_threshold(int8_t threshold)
{
    char *ctrl = "/sys/class/power_supply/BAT0/charge_control_end_threshold";
    if (threshold < 50)
        return 1;
    else if (threshold > 100)
        return 2;
    FILE *control = fopen(ctrl, "w");
    int chars_written = fprintf(control, "%" PRId8, threshold);
    if (chars_written <= 0)
    {
        perror("Failed to set threshold");
        return 3;
    }
    fclose(control);
    return 0;
}

int main(void)
{
    signal(SIGINT, clean_exit);
    signal(SIGTERM, clean_exit);
    char *socket_path = "/run/batteryd";
    struct sockaddr_un srv_socket;
    /*
     * For portability clear the whole structure, since some
     * implementations have additional (nonstandard) fields in
     * the structure. (this is according to man unix(7))
     */
    memset(&srv_socket, 0, sizeof(srv_socket));
    srv_socket.sun_family = AF_UNIX;
    strcpy(srv_socket.sun_path, socket_path);

    srv_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    // Get the GID of the batteryd group to allow access of group members to socket
    struct group *grp;
    grp = getgrnam("batteryd");
    if (grp == NULL)
    {
        fputs("Failed to get group batteryd\n", stderr);
        return 2;
    }

    if (bind(srv_fd, (struct sockaddr *)(&srv_socket), sizeof(srv_socket)) == -1)
    {
        perror("Failed to bind the unix socket");
        return 1;
    }
    // Set the socket to be rw for root and the batteryd group
    chmod(socket_path, 660);
    chown(socket_path, 0, grp->gr_gid);
    if (listen(srv_fd, 1) == -1)
    {
        perror("Failed to start listener");
        return 1;
    }
    while (1)
    {
        int client_fd = accept(srv_fd, NULL, NULL);
        int8_t threshold;
        if (read(client_fd, &threshold, 1) < 1)
            close(client_fd);
        else
        {
            int8_t status = set_battery_charge_threshold(threshold);
            write(client_fd, &status, 1);
            close(client_fd);
        }
        // Rate limiting to avoid a faulty user space process from thrashing the battery controller
        sleep(2);
    }
    return 0;
}