// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#ifndef CEPH_MOSDFullPayload_H
#define CEPH_MOSDFullPayload_H

#include "messages/PaxosServiceMessage.h"
#include "osd/OSDMap.h"



class MOSDFullPayload final : public PaxosServiceMessage {
public:
  enum {
    FLAG_PAYLOAD_WELL_OFF = 0, 
    FLAG_PAYLOAD_NEAR_FULL = 1,      
    FLAG_PAYLOAD_FULL = 2,    
     
  };
  epoch_t map_epoch = 0;
  uint32_t state = 0;
  __u8 flags = 0;
  double weight = -1.0;
  double pri_aff = -1.0;

private:
  ~MOSDFullPayload() final {}

public:
  MOSDFullPayload(epoch_t e, unsigned s, __u8 f, double w, double p)
    : PaxosServiceMessage{MSG_OSD_FULL_PAYLOAD, e}, map_epoch(e), 
                          state(s), flags(f), weight(w), pri_aff(p){ }
  
  MOSDFullPayload()
    : PaxosServiceMessage{MSG_OSD_FULL_PAYLOAD, 0} {}

public:
  double get_weight() const { return weight; }
  double get_pri_aff() const { return pri_aff; }

  bool if_osd_near_full() const { return flags & FLAG_PAYLOAD_NEAR_FULL; }
  bool if_osd_full() const { return flags & FLAG_PAYLOAD_FULL; }
  
  void encode_payload(uint64_t features) {
    using ceph::encode;
    paxos_encode();
    encode(map_epoch, payload);
    encode(state, payload);
    encode(flags, payload);
    encode(weight, payload);
    encode(pri_aff, payload);

  }
  void decode_payload() {
    using ceph::decode;
    auto p = payload.cbegin();
    paxos_decode(p);
    decode(map_epoch, p);
    decode(state, p);
    decode(flags, p);
    decode(weight, p);
    decode(pri_aff, p);
  }

  std::string_view get_type_name() const { return "osd_full_payload"; }
  void print(std::ostream &out) const {
    std::set<std::string> states;
    OSDMap::calc_state_set(state, states);
    out << "osd_full_payload(e" << map_epoch << " " << states << " v" << version << ")";
  }
private:
  template<class T, typename... Args>
  friend boost::intrusive_ptr<T> ceph::make_message(Args&&... args);
};

#endif
