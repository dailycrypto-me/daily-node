#pragma once

#include "common/packet_handler.hpp"
#include "network/tarcap/packets/latest/get_pillar_votes_bundle_packet.hpp"
#include "pillar_chain/pillar_chain_manager.hpp"

namespace daily::network::tarcap {

class GetPillarVotesBundlePacketHandler : public PacketHandler<GetPillarVotesBundlePacket> {
 public:
  GetPillarVotesBundlePacketHandler(const FullNodeConfig& conf, std::shared_ptr<PeersState> peers_state,
                                    std::shared_ptr<TimePeriodPacketsStats> packets_stats,
                                    std::shared_ptr<pillar_chain::PillarChainManager> pillar_chain_manager,
                                    const addr_t& node_addr, const std::string& logs_prefix);

  void requestPillarVotesBundle(PbftPeriod period, const blk_hash_t& pillar_block_hash,
                                const std::shared_ptr<DailyPeer>& peer);

  // Packet type that is processed by this handler
  static constexpr SubprotocolPacketType kPacketType_ = SubprotocolPacketType::kGetPillarVotesBundlePacket;

 private:
  virtual void process(GetPillarVotesBundlePacket&& packet, const std::shared_ptr<DailyPeer>& peer) override;

 protected:
  std::shared_ptr<pillar_chain::PillarChainManager> pillar_chain_manager_;
};

}  // namespace daily::network::tarcap
