#include "support.h"

int main(int argc, char *argv[]) {
    if (argc < 4) {
        print_suggestions();
        return -1;
    }
    char *if_name = argv[2];
    char *Type = argv[1];
    char *Proto = argv[3];
    if (argc > 4 && strcmp(Type, "-c") == 0) {
        char *dst_ip;
        if (strcmp(Type, "-c") == 0) {
            dst_ip = argv[4];
            Configure(Type, if_name, Proto, dst_ip);
        }
    } else {
        Configure(Type, if_name, Proto, NULL);
    }
    return 0;
}
