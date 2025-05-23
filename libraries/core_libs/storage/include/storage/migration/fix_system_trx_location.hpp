#pragma once

#include "storage/migration/migration_base.hpp"

namespace daily::storage::migration {
class FixSystemTrxLocation : public migration::Base {
 public:
  FixSystemTrxLocation(std::shared_ptr<DbStorage> db);
  std::string id() override;
  uint32_t dbVersion() override;

 protected:
  void migrate(logger::Logger& log) override;
};

}  // namespace daily::storage::migration