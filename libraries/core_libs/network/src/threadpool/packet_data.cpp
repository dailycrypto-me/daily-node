#include "network/threadpool/packet_data.hpp"

namespace daily::network::threadpool {

PacketData::PacketData(SubprotocolPacketType type, const dev::p2p::NodeID& from_node_id,
                       std::vector<unsigned char>&& bytes)
    : rlp_bytes_(std::move(bytes)),
      receive_time_(std::chrono::steady_clock::now()),
      type_(type),
      type_str_(convertPacketTypeToString(static_cast<SubprotocolPacketType>(type_))),
      priority_(getPacketPriority(type)),
      from_node_id_(from_node_id),
      rlp_(dev::RLP(rlp_bytes_)) {}

/**
 * @param packet_type
 * @return PacketPriority <high/mid/low> based om packet_type
 */
PacketData::PacketPriority PacketData::getPacketPriority(SubprotocolPacketType packet_type) {
  if (packet_type > SubprotocolPacketType::kHighPriorityPackets &&
      packet_type < SubprotocolPacketType::kMidPriorityPackets) {
    return PacketPriority::High;
  } else if (packet_type > SubprotocolPacketType::kMidPriorityPackets &&
             packet_type < SubprotocolPacketType::kLowPriorityPackets) {
    return PacketPriority::Mid;
  } else if (packet_type > SubprotocolPacketType::kLowPriorityPackets) {
    return PacketPriority::Low;
  }

  assert(false);
}

Json::Value PacketData::getPacketDataJson() const {
  Json::Value ret;

  // Transforms receive_time into the human-readable form
  const auto receive_time = std::chrono::system_clock::now() + (receive_time_ - std::chrono::steady_clock::now());
  const std::time_t t_c = std::chrono::system_clock::to_time_t(
      std::chrono::time_point_cast<std::chrono::system_clock::duration>(receive_time));
  std::ostringstream receive_time_os;
  receive_time_os << std::put_time(std::localtime(&t_c), "%F %T node local time");

  ret["id"] = Json::UInt64(id_);
  ret["type"] = type_str_;
  ret["priority"] = Json::UInt64(priority_);
  ret["receive_time"] = receive_time_os.str();
  ret["from_node_id"] = from_node_id_.toString();
  ret["rlp_items_count"] = Json::UInt64(rlp_.itemCount());
  ret["rlp_size"] = Json::UInt64(rlp_.size());

  return ret;
}

}  // namespace daily::network::threadpool
