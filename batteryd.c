/*
Copyright (C) 2024 Ahmad Ismail
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>
#include <grp.h>

enum service_status
{
    SUCCESS,
    VALUE_TOO_SMALL,
    VALUE_TOO_LARGE,
    SYSTEM_FAILURE
};

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
    struct sockaddr_un srv_socket = {.sun_family = AF_UNIX, .sun_path = "/run/batteryd"};
    int srv_fd = socket(AF_UNIX, SOCK_STREAM, 0);

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
    fchmod(srv_fd, 660);
    fchown(srv_fd, 0, grp->gr_gid);
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