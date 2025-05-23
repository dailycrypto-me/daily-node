#include "network/tarcap/packets_handler.hpp"

namespace daily::network::tarcap {

const std::shared_ptr<BasePacketHandler>& PacketsHandler::getSpecificHandler(SubprotocolPacketType packet_type) const {
  auto selected_handler = packets_handlers_.find(packet_type);

  if (selected_handler == packets_handlers_.end()) {
    assert(false);

    throw std::runtime_error("No registered packet handler for packet type: " + std::to_string(packet_type));
  }

  return selected_handler->second;
}

}  // namespace daily::network::tarcap