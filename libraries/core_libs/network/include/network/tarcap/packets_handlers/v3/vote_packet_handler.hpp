#pragma once

#include "common/ext_votes_packet_handler.hpp"

namespace daily::network::tarcap::v3 {

class VotePacketHandler : public ExtVotesPacketHandler {
 public:
  VotePacketHandler(const FullNodeConfig& conf, std::shared_ptr<PeersState> peers_state,
                    std::shared_ptr<TimePeriodPacketsStats> packets_stats, std::shared_ptr<PbftManager> pbft_mgr,
                    std::shared_ptr<PbftChain> pbft_chain, std::shared_ptr<VoteManager> vote_mgr,
                    std::shared_ptr<SlashingManager> slashing_manager, const addr_t& node_addr,
                    const std::string& logs_prefix = "");

  /**
   * @brief Sends pbft vote to connected peers
   *
   * @param vote Votes to send
   * @param block block to send - nullptr means no block
   * @param rebroadcast - send even of vote i known for the peer
   */
  void onNewPbftVote(const std::shared_ptr<PbftVote>& vote, const std::shared_ptr<PbftBlock>& block,
                     bool rebroadcast = false);
  void sendPbftVote(const std::shared_ptr<DailyPeer>& peer, const std::shared_ptr<PbftVote>& vote,
                    const std::shared_ptr<PbftBlock>& block);

  // Packet type that is processed by this handler
  static constexpr SubprotocolPacketType kPacketType_ = SubprotocolPacketType::kVotePacket;

 private:
  virtual void validatePacketRlpFormat(const threadpool::PacketData& packet_data) const override;
  virtual void process(const threadpool::PacketData& packet_data, const std::shared_ptr<DailyPeer>& peer) override;

 protected:
  const size_t kVotePacketSize{1};
  const size_t kExtendedVotePacketSize{3};
};

}  // namespace daily::network::tarcap::v3
