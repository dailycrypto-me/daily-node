#pragma once

#include "common/packet_handler.hpp"
#include "network/tarcap/packets/latest/get_dag_sync_packet.hpp"
#include "transaction/transaction.hpp"

namespace daily {
class DagManager;
class DbStorage;
class TransactionManager;
}  // namespace daily

namespace daily::network::tarcap {

class GetDagSyncPacketHandler : public PacketHandler<GetDagSyncPacket> {
 public:
  GetDagSyncPacketHandler(const FullNodeConfig& conf, std::shared_ptr<PeersState> peers_state,
                          std::shared_ptr<TimePeriodPacketsStats> packets_stats,
                          std::shared_ptr<TransactionManager> trx_mgr, std::shared_ptr<DagManager> dag_mgr,
                          std::shared_ptr<DbStorage> db, const addr_t& node_addr,
                          const std::string& logs_prefix = "GET_DAG_SYNC_PH");

  void sendBlocks(const dev::p2p::NodeID& peer_id, std::vector<std::shared_ptr<DagBlock>>&& blocks,
                  SharedTransactions&& transactions, PbftPeriod request_period, PbftPeriod period);

  // Packet type that is processed by this handler
  static constexpr SubprotocolPacketType kPacketType_ = SubprotocolPacketType::kGetDagSyncPacket;

 private:
  virtual void process(GetDagSyncPacket&& packet, const std::shared_ptr<DailyPeer>& peer) override;

 protected:
  std::shared_ptr<TransactionManager> trx_mgr_;
  std::shared_ptr<DagManager> dag_mgr_;
  std::shared_ptr<DbStorage> db_;
};

}  // namespace daily::network::tarcap
