#include "qemu/osdep.h"
#include "fork-daemon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "qemu/option.h"
#include "qemu/config-file.h"
#include "sysemu/sysemu.h"




void readlen(int serv_sock, char* buffer, int len) 
{
    int str_len = 0;
    while (str_len < len) {
        int read_len = read(serv_sock, buffer+str_len, len-str_len);
        str_len += read_len;
    }
}

void receive_package(int serv_sock, Package* pack) 
{
    // package* pack = (package*)malloc(sizeof(package));
    int cmd_len = sizeof(pack->command);
    // 读取 CMD
    readlen(serv_sock, (char*)&pack->command, cmd_len);
    // 读取 DATA_LEN
    int args_len = sizeof(pack->len);
    readlen(serv_sock, (char*)&pack->len, args_len);
    // 读取 DATA
    if (pack->len > 0) {
        pack->data = (char*)malloc(pack->len);
        readlen(serv_sock, pack->data, pack->len);
    }
    return ;
}

char* serialize_package(Package *p) 
{
    char* buf = (char*)malloc(sizeof(p->command) + sizeof(p->len) + p->len);
    int offset = 0;
    memcpy(buf, &p->command, sizeof(p->command));
    offset += sizeof(p->command);
    memcpy(buf + offset, &p->len, sizeof(p->len));
    if (p->len > 0) {
        offset += sizeof(p->len);
        memcpy(buf + offset, p->data, p->len);
    }
    return buf;
}

void send_package(Package* p, int serv_sock) 
{
    char* message = serialize_package(p);
    int len = p->len;
    int windex = 0;
    while (windex < len) {
        windex += write(serv_sock, message + windex, len - windex);
    }
    free(message);
    if (p->data != NULL)
        free(p->data);
}

const char* get_forkd_ip(void)
{
    QemuOptsList *list;
    QemuOpts *opts;
    Error *err;

    list = qemu_find_opts_err("forkdaemon", &err);
    opts = qemu_opts_find(list, NULL);
    return qemu_opt_get(opts, "ipaddr");
}

uint64_t get_forkd_port(void)
{
    QemuOptsList *list;
    QemuOpts *opts;
    Error *err;

    list = qemu_find_opts_err("forkdaemon", &err);
    opts = qemu_opts_find(list, NULL);
    return qemu_opt_get_number(opts, "port", 0);
}

uint64_t get_current_pid(void)
{
    QemuOptsList *list;
    QemuOpts *opts;
    Error *err;
    list = qemu_find_opts_err("forkgroup", &err);
    opts = qemu_opts_find(list, NULL);
    return qemu_opt_get_number(opts, "pid", 0);
}

uint64_t get_current_gid(void)
{
    QemuOptsList *list;
    QemuOpts *opts;
    Error *err;

    list = qemu_find_opts_err("forkgroup", &err);
    opts = qemu_opts_find(list, NULL);
    return qemu_opt_get_number(opts, "gid", 0);
}

void save_gid_to_config(uint64_t group_id)
{
    QemuOptsList *list;
    QemuOpts *opts;
    Error *err;

    list = qemu_find_opts_err("forkgroup", &err);
    opts = qemu_opts_find(list, NULL);
    qemu_opt_set_number(opts, "gid", group_id, &err); 
}

void close_forkd_socket(void)
{
    Package* send_pack = (Package*)malloc(sizeof(Package));
    send_pack->command = CMD_CLIENTCOLSE;
    send_pack->len = 0;
    send_pack->data = NULL;
    int serv_sock = qemu_get_forkd_sock();
    send_package(send_pack, serv_sock);
    close(serv_sock);
    free(send_pack);
}