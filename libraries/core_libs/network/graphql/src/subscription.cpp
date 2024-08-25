#include "graphql/subscription.hpp"

#include <iostream>

namespace graphql::daily {

response::Value Subscription::getTestSubscription() const noexcept {
  std::cout << "Subscription::getTestSubscription" << std::endl;
  return response::Value(123456789);
}

}  // namespace graphql::daily