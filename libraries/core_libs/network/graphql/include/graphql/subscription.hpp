#pragma once

#include "SubscriptionObject.h"

namespace graphql::daily {

class Subscription {
 public:
  explicit Subscription() noexcept = default;

  response::Value getTestSubscription() const noexcept;
};

}  // namespace graphql::daily