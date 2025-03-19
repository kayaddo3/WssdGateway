#include "NaloUssdController.h"
#include <drogon/HttpClient.h>
#include <drogon/HttpAppFramework.h>
#include <json/json.h>

void NaloUssdController::handleUssdRequest(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback)
{
    // write your application logic here
    auto jsonBody = req->getJsonObject();
    if (!jsonBody)
    {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        resp->setBody("{\"error\":\"Invalid JSON body\"}");
        callback(resp);
        return;
    }

    Json::Value requestPayload;
    if ((*jsonBody)["USERID"].isString())
        requestPayload["USERID"] = (*jsonBody)["USERID"].asString();
    if ((*jsonBody)["MSISDN"].isString())
        requestPayload["MSISDN"] = (*jsonBody)["MSISDN"].asString();
    if ((*jsonBody)["USERDATA"].isString())
        requestPayload["USERDATA"] = (*jsonBody)["USERDATA"].asString();
    if ((*jsonBody)["MSGTYPE"].isBool())
        requestPayload["MSGTYPE"] = (*jsonBody)["MSGTYPE"].asBool();
    if ((*jsonBody)["NETWORK"].isString())
        requestPayload["NETWORK"] = (*jsonBody)["NETWORK"].asString();

    // Validate required fields
    if (requestPayload["USERID"].isNull() || requestPayload["MSISDN"].isNull() ||
        requestPayload["USERDATA"].isNull() || requestPayload["MSGTYPE"].isNull() ||
        requestPayload["NETWORK"].isNull())
    {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::HttpStatusCode::k400BadRequest);
        resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        resp->setBody("{\"error\":\"Missing required parameters\"}");
        callback(resp);
        return;
    }

    // Convert JSON to string for sending
    Json::FastWriter writer;
    std::string requestBody = writer.write(requestPayload);

    // Create HTTP client and request
    auto client = drogon::HttpClient::newHttpClient("https://wssd-engine-api.rhyoliteprime.com");
    auto apiRequest = drogon::HttpRequest::newHttpRequest();
    apiRequest->setMethod(drogon::HttpMethod::Post);
    apiRequest->setPath("/api/services/app/NaloUssd/HandleInteraction");
    apiRequest->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    apiRequest->setBody(requestBody);

    // Send the request to the upstream service
    client->sendRequest(apiRequest, [callback](drogon::ReqResult result, const drogon::HttpResponsePtr &response)
                        {
        if (result == drogon::ReqResult::Ok)
        {
            // Parse the response body
            std::string responseBody = std::string(response->getBody());
            Json::Value responseJson;
            Json::Reader reader;
            
            if (reader.parse(responseBody, responseJson) && responseJson.isMember("result"))
            {
                // Extract only the "result" field
                Json::Value resultField = responseJson["result"];
                
                // Create a new JSON object with uppercase keys
                Json::Value uppercaseResponse;
                
                // Convert each property name to uppercase
                for (auto const& key : resultField.getMemberNames()) {
                    // Convert key to uppercase
                    std::string uppercaseKey = key;
                    std::transform(uppercaseKey.begin(), uppercaseKey.end(), uppercaseKey.begin(), ::toupper);
                    
                    // Copy the value to the new JSON object with uppercase key
                    uppercaseResponse[uppercaseKey] = resultField[key];
                }
                
                // Convert the result object back to JSON string
                Json::FastWriter writer;
                std::string resultJson = writer.write(uppercaseResponse);
                
                // Create response with the uppercase field names
                auto clientResponse = drogon::HttpResponse::newHttpResponse();
                clientResponse->setStatusCode(drogon::HttpStatusCode::k200OK);
                clientResponse->setContentTypeCode(drogon::CT_APPLICATION_JSON);
                clientResponse->setBody(resultJson);
                callback(clientResponse);
            }
            else
            {
                // Failed to parse JSON or no "result" field
                auto errorResp = drogon::HttpResponse::newHttpResponse();
                errorResp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
                errorResp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
                errorResp->setBody("{\"ERROR\":\"Invalid response format from upstream service\"}");
                callback(errorResp);
            }
        }
        else
        {
            // Handle error cases
            auto errorResp = drogon::HttpResponse::newHttpResponse();
            errorResp->setStatusCode(drogon::HttpStatusCode::k500InternalServerError);
            errorResp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
            errorResp->setBody("{\"ERROR\":\"Failed to connect to the upstream service\"}");
            callback(errorResp);
        } });
}
