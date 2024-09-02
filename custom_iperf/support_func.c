#include "support.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>


char *get_ip(char *if_name) {
    char *ip_address;
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;

            char ipaddr[INET_ADDRSTRLEN];

            inet_ntop(AF_INET, &sa->sin_addr, ipaddr, INET_ADDRSTRLEN);

            if (strcmp(ifa->ifa_name, if_name) == 0) {
                ip_address = strdup(&ipaddr[0]);
                break;
            }
        }
    }
    return ip_address;
}


char *get_mac(char *if_name, int socketfd) {
    char *mac_address;
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr; 

            char ipaddr[INET_ADDRSTRLEN];

            inet_ntop(AF_INET, &sa->sin_addr, ipaddr, INET_ADDRSTRLEN);

            if (strcmp(ifa->ifa_name, if_name) == 0) {
                struct ifreq ifr;
                memset(&ifr, 0, sizeof(ifr));
                strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ-1);

                if (ioctl(socketfd, SIOCGIFHWADDR, &ifr) == 0) {
                    char macaddr[MAC_ADDR_LEN];
                    sprintf(macaddr, "%02x:%02x:%02x:%02x:%02x:%02x",
                            (unsigned char)ifr.ifr_hwaddr.sa_data[0],
                            (unsigned char)ifr.ifr_hwaddr.sa_data[1],
                            (unsigned char)ifr.ifr_hwaddr.sa_data[2],
                            (unsigned char)ifr.ifr_hwaddr.sa_data[3],
                            (unsigned char)ifr.ifr_hwaddr.sa_data[4],
                            (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
                    mac_address = strdup(macaddr);
                    break;
                } else {
                    perror("ioctl");
                }
            }
        }
    }
    return mac_address;
}

int GetConnection(struct sockaddr_in *dst_addr, int *sockfd) {
    int status;
    status = connect(*sockfd, (struct sockaddr *)dst_addr, sizeof(*dst_addr));
    return status;
}

void print_suggestions(void) {
    printf("Options:\nArg[0]  executable\nArg[1]  -s -> server (OR) -c -> client\nArg[2]  interface_name\nArg[3]  -tcp -> TCP (OR) -udp -> UDP\nArg[4]  Dest IP incase of CLIENT application else NULL\n");
}

int start_tcp_server(int *fd) {
    struct sockaddr_in saddr, daddr;
    int new_fd;
    int opt = 1;
    int addr_len = sizeof(struct sockaddr);
    if (setsockopt(*fd, SOL_SOCKET,
                SO_REUSEADDR | SO_REUSEPORT, &opt,
                sizeof(opt))) {
        perror("setsockopt");
        close(*fd);
        return FAILURE;
    }
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_family = AF_INET;
    saddr.sin_port = 5201;
    bind(*fd, (struct sockaddr *)&saddr, sizeof(saddr));
    listen(*fd, 3);
    printf("Server running .....\n");
    if ((new_fd = accept(*fd, (struct sockaddr *)&daddr, (socklen_t*)&addr_len)) < 0) {
        perror("accept");
        close(*fd);
        return FAILURE;
    }
    char *buf = malloc(sizeof(char) *255);
    while (1) {
        memset(buf, 0, 255);
        read(new_fd, buf, sizeof(buf));
        printf("%s\n", buf);
    }
    free(buf);

    return SUCCESS;
}

int start_tcp_client(int *fd, char *dst_addr) {
    struct sockaddr_in saddr, daddr;
    if (dst_addr != NULL) {
        daddr.sin_family = AF_INET;
        daddr.sin_port = 5201;
        if (inet_pton(AF_INET, dst_addr, &daddr.sin_addr) <= 0) {
            perror("Invalid address or address not supported\n");
            close(*fd);
            return FAILURE;
        }
    }
    if (GetConnection(&daddr,fd) < 0) {
        perror("Connection Failed");
        close(*fd);
        return FAILURE;
    }
    printf("Client running ......\n");
    char buf[255] = "hello";
    MULTIPLE_WRITE(*fd, buf, sizeof(buf))
    printf("%s\n", buf);
    return SUCCESS;
}

int start_udp_server(int *fd) {
    struct sockaddr_in saddr, daddr;
    int new_fd;
    int opt = 1;
    int addr_len = sizeof(struct sockaddr);
    if (setsockopt(*fd, SOL_SOCKET,
                SO_REUSEADDR | SO_REUSEPORT, &opt,
                sizeof(opt))) {
        perror("setsockopt");
        close(*fd);
        return FAILURE;
    }
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_family = AF_INET;
    saddr.sin_port = 5201;
    bind(*fd, (struct sockaddr *)&saddr, sizeof(saddr));
    listen(*fd, 3);
    printf("Server running .....\n");
    if ((new_fd = accept(*fd, (struct sockaddr *)&daddr, (socklen_t*)&addr_len)) < 0) {
        perror("accept");
        close(*fd);
        return FAILURE;
    }
    char *buf = malloc(sizeof(char) *255);
    while (1) {
        memset(buf, 0, 255);
        read(new_fd, buf, sizeof(buf));
        printf("%s\n", buf);
    }
    free(buf);

    return SUCCESS;
}

int start_udp_client(int *fd, char *dst_addr) {
    struct sockaddr_in saddr, daddr;
    if (dst_addr != NULL) {
        daddr.sin_family = AF_INET;
        daddr.sin_port = 5201;
        if (inet_pton(AF_INET, dst_addr, &daddr.sin_addr) <= 0) {
            perror("Invalid address or address not supported\n");
            close(*fd);
            return FAILURE;
        }
    }
    if (GetConnection(&daddr,fd) < 0) {
        perror("Connection Failed");
        close(*fd);
        return FAILURE;
    }
    printf("Client running ......\n");
    char buf[255] = "hello";
    MULTIPLE_WRITE(*fd, buf, sizeof(buf))
    printf("%s\n", buf);
    return SUCCESS;
}

int Configure(char *APPtype, char *if_name, char *proto, char *dst_addr) {
    int fd, new_fd, opt = 1;
    char *ip_a, *mac_a;
    struct sockaddr_in saddr, daddr;
    if (strcmp(proto, "-udp") == 0)
        fd = socket(AF_INET, SOCK_DGRAM, 0);
    else if (strcmp(proto, "-tcp") == 0)
        fd = socket(AF_INET, SOCK_STREAM, 0);
    else {
        perror("ERROR in opening socket!!!!");
        return FAILURE;
    }
    ip_a = get_ip(if_name);
    mac_a = get_mac(if_name, fd);

    printf("IP address of %s: %s\n", if_name, ip_a);
    printf("MAC address of %s: %s\n", if_name, mac_a);

    if (strcmp(APPtype, "-s") == 0) {
        if (start_tcp_server(&fd) < 0) {
            perror("Error starting server!!!");
            return FAILURE;
        }
    } else if (strcmp(APPtype, "-c") == 0) {
        if (start_tcp_client(&fd, dst_addr) < 0) {
            perror("Error starting client");
            return FAILURE;
        }
    } else {
        print_suggestions();
        return FAILURE;
    }

    return SUCCESS;
}
