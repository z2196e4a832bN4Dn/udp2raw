#include "udp2raw_api.h"
#include "common.h"
#include "misc.h"
#include "log.h"

extern int client_event_loop();
extern int server_event_loop();

#include <thread>
#include <atomic>

struct Udp2Raw::Impl {
    RecvCB recv_cb;
    ErrorCB error_cb;
    std::thread th;
    std::atomic<bool> running{false};
};

static raw_mode_t to_raw_mode(RawProto p) {
    switch (p) {
        case RawProto::FAKETCP:
            return mode_faketcp;
        case RawProto::ICMP:
            return mode_icmp;
        case RawProto::UDP:
            return mode_udp;
        default:
            return mode_udp;
    }
}

static void run_client() {
    client_event_loop();
}

static void run_server() {
    server_event_loop();
}

std::unique_ptr<Udp2Raw> Udp2Raw::start_client(const U2RConfig &cfg, const SockAddr &remote) {
    auto ptr = std::unique_ptr<Udp2Raw>(new Udp2Raw);
    ptr->p_.reset(new Impl);

    raw_mode = to_raw_mode(cfg.proto);
    log_level = cfg.log_level;
    strncpy(key_string, cfg.key.c_str(), sizeof(key_string) - 1);
    key_string[sizeof(key_string) - 1] = 0;

    remote_addr.clear();
    remote_addr.from_ip_port(remote.ip, remote.port);

    ptr->p_->running = true;
    ptr->p_->th = std::thread(run_client);
    return ptr;
}

std::unique_ptr<Udp2Raw> Udp2Raw::start_server(const U2RConfig &cfg, const SockAddr &bind) {
#ifdef UDP2RAW_LINUX
    auto ptr = std::unique_ptr<Udp2Raw>(new Udp2Raw);
    ptr->p_.reset(new Impl);

    raw_mode = to_raw_mode(cfg.proto);
    log_level = cfg.log_level;
    strncpy(key_string, cfg.key.c_str(), sizeof(key_string) - 1);
    key_string[sizeof(key_string) - 1] = 0;

    local_addr.clear();
    local_addr.from_ip_port(bind.ip, bind.port);
    auto_add_iptables_rule = cfg.auto_iptables;

    ptr->p_->running = true;
    ptr->p_->th = std::thread(run_server);
    return ptr;
#else
    (void)cfg;
    (void)bind;
    return nullptr;
#endif
}

int Udp2Raw::send_raw(const void *buf, size_t len, const SockAddr &peer, uint32_t sid) {
    (void)buf;
    (void)len;
    (void)peer;
    (void)sid;
    // TODO send through existing connection
    return -1;
}

void Udp2Raw::set_on_recv(RecvCB cb) {
    if (p_) p_->recv_cb = std::move(cb);
}

void Udp2Raw::set_on_error(ErrorCB cb) {
    if (p_) p_->error_cb = std::move(cb);
}

void Udp2Raw::stop() {
    if (!p_ || !p_->running) return;
    about_to_exit = 1;
    if (p_->th.joinable()) p_->th.join();
    p_->running = false;
}

Udp2Raw::~Udp2Raw() { stop(); }
