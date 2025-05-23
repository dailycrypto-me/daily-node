#pragma once

#include "common/ext_syncing_packet_handler.hpp"
#include "network/tarcap/packets/latest/status_packet.hpp"

namespace daily::network::tarcap {

class StatusPacketHandler : public ExtSyncingPacketHandler<StatusPacket> {
 public:
  StatusPacketHandler(const FullNodeConfig& conf, std::shared_ptr<PeersState> peers_state,
                      std::shared_ptr<TimePeriodPacketsStats> packets_stats,
                      std::shared_ptr<PbftSyncingState> pbft_syncing_state, std::shared_ptr<PbftChain> pbft_chain,
                      std::shared_ptr<PbftManager> pbft_mgr, std::shared_ptr<DagManager> dag_mgr,
                      std::shared_ptr<DbStorage> db, h256 genesis_hash, const addr_t& node_addr,
                      const std::string& logs_prefix = "STATUS_PH");

  bool sendStatus(const dev::p2p::NodeID& node_id, bool initial);
  void sendStatusToPeers();

  // Packet type that is processed by this handler
  static constexpr SubprotocolPacketType kPacketType_ = SubprotocolPacketType::kStatusPacket;

 private:
  virtual void process(StatusPacket&& packet, const std::shared_ptr<DailyPeer>& peer) override;

 protected:
  const h256 kGenesisHash;
};

}  // namespace daily::network::tarcap
