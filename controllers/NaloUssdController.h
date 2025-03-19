#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

namespace
{
  const std::string PREFIX = "/api/v1/nalo-ussd";
}

class NaloUssdController : public drogon::HttpController<NaloUssdController>
{
public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(NaloUssdController::handleUssdRequest, PREFIX + "/interaction-handler", Post);
  METHOD_LIST_END

  // The handler method for the search route
  void handleUssdRequest(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback);
};
