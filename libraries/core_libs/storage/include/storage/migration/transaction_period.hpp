#pragma once
#include <libdevcore/Common.h>

#include "storage/migration/migration_base.hpp"

namespace daily::storage::migration {
class TransactionPeriod : public migration::Base {
 public:
  TransactionPeriod(std::shared_ptr<DbStorage> db);
  std::string id() override;
  uint32_t dbVersion() override;

 protected:
  void migrate(logger::Logger& log) override;
};
}  // namespace daily::storage::migration