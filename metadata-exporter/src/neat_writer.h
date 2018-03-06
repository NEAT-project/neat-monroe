#include <string>
#include <jsoncpp/json/json.h>

#include "mqloop.h"

struct neat_event {
  std::string iccid;
  std::string ifname;
  uint64_t tstamp;
  std::string ip_addr;
  uint8_t device_mode;
  uint8_t device_submode;
  int8_t rssi;
  int16_t rscp;
  int8_t ecio;
  int8_t lte_rssi;
  int16_t lte_rsrp;
  int8_t lte_rsrq;
  uint16_t lac;
  int32_t cid;
  uint32_t nw_mccmnc;
  uint32_t imsi_mccmnc;
  uint8_t device_state;
};

struct dlb_iface_info
{
  int conn;
  int quality;
};

class neat_writer
{
  private:
    std::string cib_socket;
    std::string cib_prefix;
    std::string cib_extension;
    std::string zmq_topic;
    std::string zmq_addr;
    std::string ifname_key;
    mqloop& loop;
    zmq::socket_t subscriber;
    mqtimer dlb_timer;
    std::map<std::string, dlb_iface_info> dlb_info;

    void add_property(Json::Value *root, const char *key,
                      const Json::Value& value, uint8_t precedence) const;

    neat_event parse_zmq_message(const std::string& message) const;
    std::string form_neat_message(const neat_event& event) const;
    void notify_policy_manager(const std::string& message) const;
    void dump_cib_file(const std::string& uid, const std::string& message) const;

    bool handle_message();
    bool hande_dlb_timer();
    static size_t handle_curl_data(char *ptr, size_t size, size_t nmemb, void *userdata);

  public:
    neat_writer(mqloop& loop, const std::string& zmq_topic, const std::string& zmq_addr);
    ~neat_writer();

    void set_cib_socket(const std::string& name);
    void set_cib_file(const std::string& prefix, const std::string& extension);
    void set_ifname_key(const std::string& key);
};
