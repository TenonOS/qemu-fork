#define BUF_SIZE 256

enum CMD {
    CMD_ERROR = 0,
    CMD_INIT,
    CMD_INITRET,
    CMD_FORKPARA,
    CMD_QEMUFORK,
    CMD_QEMUFORKRET,
    CMD_CLIENTCOLSE,
    CMD_WAITPID,
    CMD_WAITPIDRET,
    CMD_SENDGIDPID,
    CMD_KILL,
};

typedef struct Package{
    enum CMD command;
    int len;
    char *data;
} Package;




void readlen(int serv_sock, char* buffer, int len);
void receive_package(int serv_sock, Package* pack);
char* serialize_package(Package *p, int* str_len);
void send_package(Package* p, int serv_sock);
const char* get_forkd_ip(void);
uint64_t get_forkd_port(void);
uint64_t get_current_pid(void);
uint64_t get_current_gid(void);
void save_gid_to_config(uint64_t group_id);
void close_forkd_socket(void);