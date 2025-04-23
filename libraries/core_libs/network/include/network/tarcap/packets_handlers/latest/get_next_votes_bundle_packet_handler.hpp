#pragma once

#include "common/ext_votes_packet_handler.hpp"
#include "network/tarcap/packets/latest/get_next_votes_bundle_packet.hpp"

namespace daily {
class PbftManager;
class VoteManager;
}  // namespace daily

namespace daily::network::tarcap {

class GetNextVotesBundlePacketHandler : public ExtVotesPacketHandler<GetNextVotesBundlePacket> {
 public:
  GetNextVotesBundlePacketHandler(const FullNodeConfig& conf, std::shared_ptr<PeersState> peers_state,
                                  std::shared_ptr<TimePeriodPacketsStats> packets_stats,
                                  std::shared_ptr<PbftManager> pbft_mgr, std::shared_ptr<PbftChain> pbft_chain,
                                  std::shared_ptr<VoteManager> vote_mgr,
                                  std::shared_ptr<SlashingManager> slashing_manager, const addr_t& node_addr,
                                  const std::string& logs_prefix = "GET_NEXT_VOTES_BUNDLE_PH");

  // Packet type that is processed by this handler
  static constexpr SubprotocolPacketType kPacketType_ = SubprotocolPacketType::kGetNextVotesSyncPacket;

 private:
  virtual void process(GetNextVotesBundlePacket&& packet, const std::shared_ptr<DailyPeer>& peer) override;
};

}  // namespace daily::network::tarcap
