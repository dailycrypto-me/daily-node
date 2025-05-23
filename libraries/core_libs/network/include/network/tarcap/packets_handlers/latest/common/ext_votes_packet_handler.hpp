#pragma once

#include "network/tarcap/packets/latest/get_pbft_sync_packet.hpp"
#include "network/tarcap/packets/latest/votes_bundle_packet.hpp"
#include "network/tarcap/packets_handlers/latest/common/exceptions.hpp"
#include "packet_handler.hpp"
#include "pbft/pbft_manager.hpp"
#include "vote/pbft_vote.hpp"
#include "vote/votes_bundle_rlp.hpp"
#include "vote_manager/vote_manager.hpp"

namespace daily::network::tarcap {

/**
 * @brief ExtVotesPacketHandler is extended abstract PacketHandler with added functions that are used in packet
 *        handlers that process pbft votes
 */
template <class PacketType>
class ExtVotesPacketHandler : public PacketHandler<PacketType> {
 public:
  ExtVotesPacketHandler(const FullNodeConfig& conf, std::shared_ptr<PeersState> peers_state,
                        std::shared_ptr<TimePeriodPacketsStats> packets_stats, std::shared_ptr<PbftManager> pbft_mgr,
                        std::shared_ptr<PbftChain> pbft_chain, std::shared_ptr<VoteManager> vote_mgr,
                        std::shared_ptr<SlashingManager> slashing_manager, const addr_t& node_addr,
                        const std::string& log_channel_name)
      : PacketHandler<PacketType>(conf, std::move(peers_state), std::move(packets_stats), node_addr, log_channel_name),
        last_votes_sync_request_time_(std::chrono::system_clock::now()),
        last_pbft_block_sync_request_time_(std::chrono::system_clock::now()),
        pbft_mgr_(std::move(pbft_mgr)),
        pbft_chain_(std::move(pbft_chain)),
        vote_mgr_(std::move(vote_mgr)),
        slashing_manager_(std::move(slashing_manager)) {}

  virtual ~ExtVotesPacketHandler() = default;
  ExtVotesPacketHandler(const ExtVotesPacketHandler&) = delete;
  ExtVotesPacketHandler(ExtVotesPacketHandler&&) = delete;
  ExtVotesPacketHandler& operator=(const ExtVotesPacketHandler&) = delete;
  ExtVotesPacketHandler& operator=(ExtVotesPacketHandler&&) = delete;

  /**
   * @brief Process vote
   *
   * @param vote
   * @param pbft_block
   * @param peer
   * @param validate_max_round_step
   * @return if vote was successfully processed, otherwise false
   */
  bool processVote(const std::shared_ptr<PbftVote>& vote, const std::shared_ptr<PbftBlock>& pbft_block,
                   const std::shared_ptr<DailyPeer>& peer, bool validate_max_round_step) {
    if (pbft_block && !validateVoteAndBlock(vote, pbft_block)) {
      throw MaliciousPeerException("Received vote's voted value != received pbft block");
    }

    if (vote_mgr_->voteInVerifiedMap(vote)) {
      LOG(this->log_dg_) << "Vote " << vote->getHash() << " already inserted in verified queue";
      return false;
    }

    // Validate vote's period, round and step min/max values
    if (const auto vote_valid = validateVotePeriodRoundStep(vote, peer, validate_max_round_step); !vote_valid.first) {
      LOG(this->log_wr_) << "Vote period/round/step " << vote->getHash()
                         << " validation failed. Err: " << vote_valid.second;
      return false;
    }

    // Check is vote is unique per period, round & step & voter -> each address can generate just 1 vote
    // (for a value that isn't NBH) per period, round & step
    if (auto vote_valid = vote_mgr_->isUniqueVote(vote); !vote_valid.first) {
      // Create double voting proof
      slashing_manager_->submitDoubleVotingProof(vote, vote_valid.second);
      throw MaliciousPeerException("Received double vote", vote->getVoter());
    }

    // Validate vote's signature, vrf, etc...
    if (const auto vote_valid = vote_mgr_->validateVote(vote); !vote_valid.first) {
      LOG(this->log_wr_) << "Vote " << vote->getHash() << " validation failed. Err: " << vote_valid.second;
      return false;
    }

    if (!vote_mgr_->addVerifiedVote(vote)) {
      LOG(this->log_dg_) << "Vote " << vote->getHash() << " already inserted in verified queue(race condition)";
      return false;
    }

    if (pbft_block) {
      pbft_mgr_->processProposedBlock(pbft_block, vote);
    }

    return true;
  }

  /**
   * @brief Checks is vote is relevant for current pbft state in terms of period, round and type
   * @param vote
   * @return true if vote is relevant for current pbft state, otherwise false
   */
  bool isPbftRelevantVote(const std::shared_ptr<PbftVote>& vote) const {
    const auto [current_pbft_round, current_pbft_period] = pbft_mgr_->getPbftRoundAndPeriod();

    if (vote->getPeriod() >= current_pbft_period && vote->getRound() >= current_pbft_round) {
      // Standard current or future vote
      return true;
    } else if (vote->getPeriod() == current_pbft_period && vote->getRound() == (current_pbft_round - 1) &&
               vote->getType() == PbftVoteTypes::next_vote) {
      // Previous round next vote
      return true;
    } else if (vote->getPeriod() == current_pbft_period - 1 && vote->getType() == PbftVoteTypes::cert_vote) {
      // Previous period cert vote - potential reward vote
      return true;
    }

    return false;
  }

  virtual void sendPbftVotesBundle(const std::shared_ptr<DailyPeer>& peer,
                                   std::vector<std::shared_ptr<PbftVote>>&& votes) {
    if (votes.empty()) {
      return;
    }

    auto sendVotes = [this, &peer](std::vector<std::shared_ptr<PbftVote>>&& votes) {
      auto packet = VotesBundlePacket{OptimizedPbftVotesBundle{.votes = std::move(votes)}};
      if (this->sealAndSend(peer->getId(), SubprotocolPacketType::kVotesBundlePacket, encodePacketRlp(packet))) {
        LOG(this->log_dg_) << " Votes bundle with " << packet.votes_bundle.votes.size() << " votes sent to "
                           << peer->getId();
        for (const auto& vote : packet.votes_bundle.votes) {
          peer->markPbftVoteAsKnown(vote->getHash());
        }
      }
    };

    if (votes.size() <= kMaxVotesInBundleRlp) {
      sendVotes(std::move(votes));
      return;
    } else {
      // Need to split votes into multiple packets
      size_t index = 0;
      while (index < votes.size()) {
        const size_t votes_count = std::min(kMaxVotesInBundleRlp, votes.size() - index);

        const auto begin_it = std::next(votes.begin(), index);
        const auto end_it = std::next(begin_it, votes_count);

        std::vector<std::shared_ptr<PbftVote>> votes_sub_vector;
        std::move(begin_it, end_it, std::back_inserter(votes_sub_vector));

        sendVotes(std::move(votes_sub_vector));

        index += votes_count;
      }
    }
  }

 private:
  /**
   * @brief Validates vote period, round and step against max values from config
   *
   * @param vote to be validated
   * @param peer
   * @param validate_max_round_step validate also max round and step
   * @return <true, ""> vote validation passed, otherwise <false, "err msg">
   */
  std::pair<bool, std::string> validateVotePeriodRoundStep(const std::shared_ptr<PbftVote>& vote,
                                                           const std::shared_ptr<DailyPeer>& peer,
                                                           bool validate_max_round_step) {
    const auto [current_pbft_round, current_pbft_period] = pbft_mgr_->getPbftRoundAndPeriod();

    auto genErrMsg = [period = current_pbft_period, round = current_pbft_round,
                      step = pbft_mgr_->getPbftStep()](const std::shared_ptr<PbftVote>& vote) -> std::string {
      std::stringstream err;
      err << "Vote " << vote->getHash() << " (period, round, step) = (" << vote->getPeriod() << ", " << vote->getRound()
          << ", " << vote->getStep() << "). Current PBFT (period, round, step) = (" << period << ", " << round << ", "
          << step << ")";
      return err.str();
    };

    // Period validation
    // vote->getPeriod() == current_pbft_period - 1 && cert_vote -> potential reward vote
    if (vote->getPeriod() < current_pbft_period - 1 ||
        (vote->getPeriod() == current_pbft_period - 1 && vote->getType() != PbftVoteTypes::cert_vote)) {
      return {false, "Invalid period(too small): " + genErrMsg(vote)};
    } else if (this->kConf.network.ddos_protection.vote_accepting_periods &&
               vote->getPeriod() - 1 >
                   current_pbft_period + this->kConf.network.ddos_protection.vote_accepting_periods) {
      // skip this check if kConf.network.ddos_protection.vote_accepting_periods == 0
      // vote->getPeriod() - 1 is here because votes are validated against vote_period - 1 in dpos contract
      // Do not request round sync too often here
      if (vote->getVoter() == peer->getId() &&
          std::chrono::system_clock::now() - last_pbft_block_sync_request_time_ > kSyncRequestInterval) {
        // request PBFT chain sync from this node
        this->sealAndSend(
            peer->getId(), SubprotocolPacketType::kGetPbftSyncPacket,
            encodePacketRlp(GetPbftSyncPacket{std::max(vote->getPeriod() - 1, peer->pbft_chain_size_.load())}));
        last_pbft_block_sync_request_time_ = std::chrono::system_clock::now();
      }

      return {false, "Invalid period(too big): " + genErrMsg(vote)};
    }

    // Round validation
    auto checking_round = current_pbft_round;
    // If period is not the same we assume current round is equal to 1
    // So we won't accept votes for future period with round bigger than kConf.network.vote_accepting_steps
    if (current_pbft_period != vote->getPeriod()) {
      checking_round = 1;
    }

    // vote->getRound() == checking_round - 1 && next_vote -> previous round next vote
    if (vote->getRound() < checking_round - 1 ||
        (vote->getRound() == checking_round - 1 && vote->getType() != PbftVoteTypes::next_vote)) {
      return {false, "Invalid round(too small): " + genErrMsg(vote)};
    } else if (validate_max_round_step && this->kConf.network.ddos_protection.vote_accepting_rounds &&
               vote->getRound() >= checking_round + this->kConf.network.ddos_protection.vote_accepting_rounds) {
      // skip this check if kConf.network.vote_accepting_rounds == 0
      // Trigger votes(round) syncing only if we are in sync in terms of period
      if (current_pbft_period == vote->getPeriod()) {
        // Do not request round sync too often here
        if (vote->getVoter() == peer->getId() &&
            std::chrono::system_clock::now() - last_votes_sync_request_time_ > kSyncRequestInterval) {
          // request round votes sync from this node
          this->requestPbftNextVotesAtPeriodRound(peer->getId(), current_pbft_period, current_pbft_round);
          last_votes_sync_request_time_ = std::chrono::system_clock::now();
        }
      }

      return {false, "Invalid round(too big): " + genErrMsg(vote)};
    }

    // Step validation
    auto checking_step = pbft_mgr_->getPbftStep();
    // If period or round is not the same we assume current step is equal to 1
    // So we won't accept votes for future rounds with step bigger than kConf.network.vote_accepting_steps
    if (current_pbft_period != vote->getPeriod() || current_pbft_round != vote->getRound()) {
      checking_step = 1;
    }

    // skip check if kConf.network.vote_accepting_steps == 0
    if (validate_max_round_step && this->kConf.network.ddos_protection.vote_accepting_steps &&
        vote->getStep() >= checking_step + this->kConf.network.ddos_protection.vote_accepting_steps) {
      return {false, "Invalid step(too big): " + genErrMsg(vote)};
    }

    return {true, ""};
  }

  /**
   * @brief Validates provided vote if voted value == provided block
   *
   * @param vote
   * @param pbft_block
   * @return true if validation successful, otherwise false
   */
  bool validateVoteAndBlock(const std::shared_ptr<PbftVote>& vote, const std::shared_ptr<PbftBlock>& pbft_block) const {
    if (pbft_block->getBlockHash() != vote->getBlockHash()) {
      LOG(this->log_er_) << "Vote " << vote->getHash() << " voted block " << vote->getBlockHash() << " != actual block "
                         << pbft_block->getBlockHash();
      return false;
    }
    return true;
  }

 protected:
  constexpr static size_t kMaxVotesInBundleRlp{1000};
  constexpr static std::chrono::seconds kSyncRequestInterval = std::chrono::seconds(10);

  mutable std::chrono::system_clock::time_point last_votes_sync_request_time_;
  mutable std::chrono::system_clock::time_point last_pbft_block_sync_request_time_;

  std::shared_ptr<PbftManager> pbft_mgr_;
  std::shared_ptr<PbftChain> pbft_chain_;
  std::shared_ptr<VoteManager> vote_mgr_;
  std::shared_ptr<SlashingManager> slashing_manager_;
};

}  // namespace daily::network::tarcap
