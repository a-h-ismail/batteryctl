#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>
#include <grp.h>

int set_battery_charge_threshold(int8_t threshold)
{
    char *ctrl = "/sys/class/power_supply/BAT0/charge_control_end_threshold";
    if (threshold < 50)
        return 1;
    else if (threshold > 100)
        return 2;
    FILE *control = fopen(ctrl, "w");
    fprintf(control, "%" PRId8, threshold);
    fclose(control);
    return 0;
}

int main(void)
{
    struct sockaddr_un srv_socket = {.sun_family = AF_UNIX, .sun_path = "/run/batteryd"};
    int srv_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct group *grp;
    grp = getgrnam("bctrl");

    if (bind(srv_fd, (struct sockaddr *)(&srv_socket), sizeof(srv_socket)) == -1)
    {
        fputs("Failed to bind the unix socket...", stderr);
        return 1;
    }
    fchmod(srv_fd, 660);
    if (listen(srv_fd, 1) == -1)
    {
        fputs("Failed to start listener...", stderr);
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
        }
        // Rate limiting to avoid a faulty user space process from thrashing the battery controller
        sleep(2);
    }
    return 0;
}