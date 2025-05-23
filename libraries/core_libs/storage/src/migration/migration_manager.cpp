#include "storage/migration/migration_manager.hpp"

#include "storage/migration/final_chain_header.hpp"
#include "storage/migration/fix_system_trx_location.hpp"
#include "storage/migration/period_dag_blocks.hpp"
#include "storage/migration/transaction_period.hpp"
#include "storage/migration/transaction_receipts_by_period.hpp"
namespace daily::storage::migration {

Manager::Manager(std::shared_ptr<DbStorage> db, const addr_t& node_addr) : db_(db) {
  registerMigration<PeriodDagBlocks>();
  registerMigration<FinalChainHeader>();
  registerMigration<FixSystemTrxLocation>();
  LOG_OBJECTS_CREATE("MIGRATIONS");
}
void Manager::applyMigration(std::shared_ptr<migration::Base> m) {
  if (m->isApplied()) {
    LOG(log_si_) << "Skip \"" << m->id() << "\" migration. It was already applied";
    return;
  }

  if (db_->getMajorVersion() != m->dbVersion()) {
    LOG(log_si_) << "Skip \"" << m->id() << "\" migration as it was made for different major db version "
                 << m->dbVersion() << ", current db major version " << db_->getMajorVersion()
                 << ". Migration can be removed from the code";
    return;
  }

  LOG(log_si_) << "Applying migration " << m->id();
  m->apply(log_si_);
  LOG(log_si_) << "Migration applied " << m->id();
}

void Manager::applyAll() {
  for (const auto& m : migrations_) {
    applyMigration(m);
  }
}
void Manager::applyTransactionPeriod() { applyMigration(std::make_shared<TransactionPeriod>(db_)); }

void Manager::applyReceiptsByPeriod() { applyMigration(std::make_shared<TransactionReceiptsByPeriod>(db_)); }

}  // namespace daily::storage::migration