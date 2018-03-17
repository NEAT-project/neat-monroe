/* Copyright (c) 2015, Celerway, Tomasz Rozensztrauch <t.rozensztrauch@radytek.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <zmq.hpp>
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <fstream>

#include "neat_writer.h"


void neat_writer::add_property(Json::Value *root, const char *key,
                               const Json::Value& value, uint8_t precedence) const
{
    (*root)["properties"][key]["value"] = value;
    (*root)["properties"][key]["precedence"] = precedence;
}

neat_event neat_writer::parse_zmq_message(const std::string& message) const
{
  Json::Value root;
  Json::Reader reader;
  bool ok = reader.parse(message, root);
  if (!ok) {
    throw std::runtime_error("Failed to parsing JSON message");
  }
  
  struct neat_event ev;
  ev.iccid = root.get("ICCID", "").asString();
  ev.ifname = root.get(ifname_key, "").asString();
  ev.dlb_ifname = root.get("InterfaceName", "").asString();
  ev.tstamp = root.get("Timestamp", 0).asUInt64();
  ev.ip_addr = root.get("IPAddress", "").asString();
  ev.device_mode = root.get("DeviceMode", 0).asUInt();
  ev.device_submode =  root.get("DeviceSubmode", 0).asUInt();
  ev.rssi = root.get("RSSI", 0).asInt();
  ev.rscp = root.get("RSCP", 0).asInt();
  ev.ecio = root.get("ECIO", 0).asInt();
  ev.lte_rssi = root.get("RSSI", 0).asInt();
  ev.lte_rsrp = root.get("RSRP", 0).asInt();
  ev.lte_rsrq = root.get("RSRQ", 0).asInt();
  ev.lte_freq = root.get("Frequency", 0).asInt();
  ev.lte_band = root.get("Band", 0).asInt();
  ev.lac = root.get("LAC", 0).asUInt();
  ev.cid = root.get("CID", 0).asInt();
  ev.nw_mccmnc = root.get("NWMCCMNC", 0).asUInt();
  ev.imsi_mccmnc = root.get("IMSIMCCMNC", 0).asUInt();
  ev.device_state = root.get("DeviceState", 0).asUInt();

  return ev;
}

std::string neat_writer::form_neat_message(const neat_event& ev) const
{
  Json::Value message;

  message["uid"] =  ev.ifname + "_metadata_exporter";
  //message["ts"] =  static_cast<Json::Value::UInt64>(ev.tstamp);
  //message["root"] =  true;
  message["description"] = "Interface's metadata";
  message["priority"] =  4;
  message["link"] = true;

  //message["match"] = Json::Value(Json::arrayValue);
  //message["match"].append() ) ["uid"]["value"] = ev.ifname;
  //Json::Value dict;
  //dict[]

  message["match"][0]["uid"]["value"] = ev.ifname;
  

  //add_property(&message, "interface", ev.ifname, 2);
  //add_property(&message, "local_ip", ev.ip_addr, 2);
  add_property(&message, "is_wired", false, 2);
  add_property(&message, "device_mode", ev.device_mode, 2);
  add_property(&message, "device_submode", ev.device_submode, 2);

  if (ev.device_mode == 5) { // LTE
    add_property(&message, "lte_rssi", ev.lte_rssi, 2);
    add_property(&message, "lte_rsrp", ev.lte_rsrp, 2);
    add_property(&message, "lte_rsrq", ev.lte_rsrq, 2);
    add_property(&message, "lte_freq", ev.lte_freq, 2);
    add_property(&message, "lte_band", ev.lte_band, 2);
  } else {
    add_property(&message, "rssi", ev.rssi, 2);
    add_property(&message, "rscp", ev.rscp, 2);
    add_property(&message, "ecio", ev.ecio, 2);
  }

  add_property(&message, "lac", ev.lac, 2);
  add_property(&message, "cid", ev.cid, 2);
  add_property(&message, "oper", ev.nw_mccmnc, 2);
  add_property(&message, "device_state", ev.device_state, 2);

  auto dlb = dlb_info.find(ev.dlb_ifname);
  if(dlb != dlb_info.end()) {
    add_property(&message, "dlb_conn", dlb->second.conn, 2);
    add_property(&message, "dlb_quality", dlb->second.quality, 2);
  }
    
  Json::StyledWriter writer;
  return writer.write(message);
}

void neat_writer::notify_policy_manager(const std::string& message) const
{
    if (cib_socket.empty())
        return;

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        std::cerr << "NEAT: notify_policy_manager: Failed to create unix socket" << std::endl;
        return;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, cib_socket.c_str(), sizeof(addr.sun_path) - 1);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        std::cerr << "NEAT: md_motify_pm connect error" << std::endl;
        return;
    }

    ssize_t res = write(fd, message.c_str(), message.length());
    std::cerr << "NEAT: notify_policy_manager: writen "<< res
              << " bytes out of " << message.length() << std::endl;

    close(fd);
}

void neat_writer::dump_cib_file(const std::string& uid, const std::string& message) const
{
    if (cib_prefix.empty())
        return;

    std::stringstream ss;
    ss << cib_prefix << uid << cib_extension;
    std::ofstream ofs(ss.str());
    if (!ofs) {
      std::cerr << "NEAT: dump_cib_file: failed to open file "<< ss.str() << std::endl;
      return;
    }

    ofs << message;
}

bool neat_writer::handle_message()
{
  zmq::message_t msg;
  subscriber.recv(&msg);

  std::string raw = std::string(static_cast<char*>(msg.data()), msg.size());
  std::size_t pos = raw.find(' ');
  if (pos != std::string::npos) {
    std::string header = raw.substr(0, pos);
    std::string body = raw.substr(pos + 1);

    std::cerr << "--------" << std::endl;
    std::cerr << header << std::endl << body << std::endl << std::endl;

    neat_event ev = parse_zmq_message(body);
    std::string message = form_neat_message(ev);
    //std::cerr << message;
    notify_policy_manager(message);
    dump_cib_file(ev.ifname, message);
  } else {
    std::cerr << "NEAT: Invalid ZMQ message: " << raw << std::endl;
  }
  
  return true;
}

size_t neat_writer::handle_curl_data(char *ptr, size_t size, size_t nmemb, void *userdata)
{
  std::string *str = (std::string *)userdata;
  str->append((const char *)ptr, size * nmemb);
  return size * nmemb;
}

bool neat_writer::hande_dlb_timer()
{
  /*std::cerr << "dlb_timer" << std::endl;
  CURL *curl = curl_easy_init();
  if (curl) {
    std::string json_str;
    CURLcode res;
    //curl_easy_setopt(curl, CURLOPT_URL, "http://localhost/dlb.json");
    curl_easy_setopt(curl, CURLOPT_URL, "http://172.17.0.1:88/dlb");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_curl_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&json_str);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
      std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    } else {
      Json::Reader reader;
      Json::Value message;
      reader.parse(json_str, message);

      std::cerr << "------" << std::endl;
      for (Json::Value::ArrayIndex i = 0; i != message["interfaces"].size(); i++) {
        std::string name = message["interfaces"][i]["name"].asString();
        int conn = message["interfaces"][i]["conn"].asInt();
        int quality = message["interfaces"][i]["quality"].asInt();

        std::cerr << "index: " << i << std::endl;
        std::cerr << "name: " << name << std::endl;
        std::cerr << "conn: " << conn << std::endl;
        std::cerr << "quality: " << quality << std::endl;
        std::cerr << "------" << std::endl;
        dlb_info[name] = {conn, quality};
      }

      //Json::StyledWriter writer;
      //std::cerr << writer.write(message) << std::endl;
    }

    curl_easy_cleanup(curl);
  }*/
  return true;
}

neat_writer::neat_writer(mqloop& loop, const std::string& zmq_topic, const std::string& zmq_addr)
  : zmq_topic(zmq_topic), zmq_addr(zmq_addr), ifname_key("InterfaceName"),
    loop(loop), subscriber(loop.get_zmq_context(), ZMQ_SUB),
    dlb_timer(loop, {{5,0},{0,500000000}}, std::bind(&neat_writer::hande_dlb_timer, this))
{
  curl_global_init(CURL_GLOBAL_DEFAULT);
  subscriber.connect(zmq_addr.c_str());
  subscriber.setsockopt(ZMQ_SUBSCRIBE, zmq_topic.data(), zmq_topic.length());
  loop.register_socket(&subscriber, std::bind(&neat_writer::handle_message, this));
}

neat_writer::~neat_writer()
{
  loop.unregister_socket(&subscriber);
  subscriber.close();
  curl_global_cleanup();
}

void neat_writer::set_cib_socket(const std::string& name)
{
  cib_socket = name;
}

void neat_writer::set_cib_file(const std::string& prefix, const std::string& extension)
{
  cib_prefix = prefix;
  cib_extension = extension;
}

void neat_writer::set_ifname_key(const std::string& key)
{
  ifname_key = key;
}
