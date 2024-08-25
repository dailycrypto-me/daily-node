#pragma once

#include "network/ws_server.hpp"

namespace daily::net {

class GraphQlWsSession final : public WsSession {
 public:
  using WsSession::WsSession;
  std::string processRequest(const std::string_view& request) override;
  void triggerTestSubscribtion(unsigned int number);
};

class GraphQlWsServer final : public WsServer {
 public:
  using WsServer::WsServer;
  std::shared_ptr<WsSession> createSession(tcp::socket&& socket) override;
};

}  // namespace daily::net