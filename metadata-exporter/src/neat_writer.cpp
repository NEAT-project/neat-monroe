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
  ev.ifname = root.get("InterfaceName", "").asString();
  ev.tstamp = root.get("Timestamp", 0).asUInt64();
  ev.ip_addr = root.get("IPAddress", "").asString();
  ev.device_mode = root.get("DeviceMode", 0).asUInt();
  ev.device_submode =  root.get("DeviceSubmode", 0).asUInt();
  ev.rssi = root.get("RSSI", 0).asInt();
  ev.rscp = root.get("RSCP", 0).asInt();
  ev.ecio = root.get("ECIO", 0).asInt();
  ev.lte_rssi = root.get("LTERSSI", 0).asInt();
  ev.lte_rsrp = root.get("LTERSRP", 0).asInt();
  ev.lte_rsrq = root.get("LTERSRQ", 0).asInt();
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

  message["uid"] =  ev.ifname;
  message["ts"] =  static_cast<Json::Value::UInt64>(ev.tstamp);
  message["root"] =  true;
  message["priority"] =  4;

  add_property(&message, "interface", ev.ifname, 2);
  add_property(&message, "local_ip", ev.ip_addr, 2);
  add_property(&message, "is_wired", false, 2);
  add_property(&message, "device_mode", ev.device_mode, 2);
  add_property(&message, "device_submode", ev.device_submode, 2);

  if (ev.device_mode == 5) { // LTE
    add_property(&message, "rssi", ev.rssi, 2);
    add_property(&message, "rscp", ev.rscp, 2);
    add_property(&message, "ecio", ev.ecio, 2);
  } else {
    add_property(&message, "lte_rssi", ev.lte_rssi, 2);
    add_property(&message, "lte_rsrp", ev.lte_rsrp, 2);
    add_property(&message, "lte_rsrq", ev.lte_rsrq, 2);
  }

  add_property(&message, "lac", ev.lac, 2);
  add_property(&message, "cid", ev.cid, 2);
  add_property(&message, "oper", ev.nw_mccmnc, 2);
  add_property(&message, "device_state", ev.device_state, 2);
    
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

  std::istringstream iss(std::string(static_cast<char*>(msg.data()), msg.size()));
  std::string header, body;
  iss >> header >> body;

  std::cerr << "--------" << std::endl;
  std::cerr << header << std::endl << body << std::endl << std::endl;

  neat_event ev = parse_zmq_message(body);
  std::string message = form_neat_message(ev);
  //std::cerr << message;
  notify_policy_manager(message);
  dump_cib_file(ev.ifname, message);
  
  return true;
}

neat_writer::neat_writer(mqloop& loop, const std::string& zmq_topic, const std::string& zmq_addr)
  : zmq_topic(zmq_topic), zmq_addr(zmq_addr), loop(loop),
    subscriber(loop.get_zmq_context(), ZMQ_SUB)
{
  subscriber.connect(zmq_addr);
  subscriber.setsockopt(ZMQ_SUBSCRIBE, zmq_topic.data(), zmq_topic.length());
  loop.register_socket(&subscriber, std::bind(&neat_writer::handle_message, this));
}

neat_writer::~neat_writer()
{
  loop.unregister_socket(&subscriber);
  subscriber.close();
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
