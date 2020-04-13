#ifndef ANKICLIENT_H
#define ANKICLIENT_H

#include "curl_request.h"
#include <libs/json.hpp>


class AnkiClient
{
public:
    AnkiClient();

    nlohmann::json request(const std::string &action, const nlohmann::json &params = nullptr);

private:
    CurlSession session;
};

#endif // ANKICLIENT_H
