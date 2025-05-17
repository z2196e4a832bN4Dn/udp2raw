#ifndef UDP2RAW_API_H
#define UDP2RAW_API_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

struct SockAddr {
    uint32_t ip;
    uint16_t port;
};

using RecvCB = std::function<void(const uint8_t *, size_t, const SockAddr &, uint32_t)>;
using ErrorCB = std::function<void(int, const std::string &)>;

enum class RawProto { FAKETCP,
                      ICMP,
                      UDP };

struct U2RConfig {
    RawProto proto;
    std::string key;
    int log_level = 2;
    bool auto_iptables = false;  // linux server only
};

class Udp2Raw {
   public:
    static std::unique_ptr<Udp2Raw> start_client(const U2RConfig &cfg, const SockAddr &remote);
    static std::unique_ptr<Udp2Raw> start_server(const U2RConfig &cfg, const SockAddr &bind);

    int send_raw(const void *buf, size_t len, const SockAddr &peer, uint32_t sid);
    void set_on_recv(RecvCB cb);
    void set_on_error(ErrorCB cb);
    void stop();
    ~Udp2Raw();

   private:
    struct Impl;
    std::unique_ptr<Impl> p_;
};

#endif  // UDP2RAW_API_H
