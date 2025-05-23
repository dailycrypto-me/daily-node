#pragma once

#include "network/tarcap/packets/latest/pillar_vote_packet.hpp"
#include "network/tarcap/packets_handlers/latest/common/ext_pillar_vote_packet_handler.hpp"

namespace daily::network::tarcap {

class PillarVotePacketHandler : public ExtPillarVotePacketHandler<PillarVotePacket> {
 public:
  PillarVotePacketHandler(const FullNodeConfig& conf, std::shared_ptr<PeersState> peers_state,
                          std::shared_ptr<TimePeriodPacketsStats> packets_stats,
                          std::shared_ptr<pillar_chain::PillarChainManager> pillar_chain_manager,
                          const addr_t& node_addr, const std::string& logs_prefix);

  void onNewPillarVote(const std::shared_ptr<PillarVote>& vote, bool rebroadcast = false);
  void sendPillarVote(const std::shared_ptr<DailyPeer>& peer, const std::shared_ptr<PillarVote>& vote);

  // Packet type that is processed by this handler
  static constexpr SubprotocolPacketType kPacketType_ = SubprotocolPacketType::kPillarVotePacket;

 private:
  virtual void process(PillarVotePacket&& packet, const std::shared_ptr<DailyPeer>& peer) override;
};

}  // namespace daily::network::tarcap
