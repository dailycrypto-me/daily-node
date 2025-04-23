#pragma once
#include "storage/migration/migration_base.hpp"

namespace daily::storage::migration {
class PeriodDagBlocks : public migration::Base {
 public:
  PeriodDagBlocks(std::shared_ptr<DbStorage> db);
  std::string id() override;
  uint32_t dbVersion() override;
  void migrate(logger::Logger& log) override;
};
}  // namespace daily::storage::migration