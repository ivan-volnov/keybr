#include "anki_client.h"


AnkiClient::AnkiClient()
{
    session.set_json_headers();
}

nlohmann::json AnkiClient::request(const std::string &action, const nlohmann::json &params)
{
    nlohmann::json json;
    json["action"] = action;
    json["version"] = 6;
    if (!params.is_null()) {
        json["params"] = params;
    }
    const auto response = nlohmann::json::parse(session.post("http://127.0.0.1:8765", json.dump()));
    if (!response.at("error").is_null()) {
        throw std::runtime_error("AnkiConnect error: " + response["error"].get<std::string>());
    }
    if (action == "addNotes") {
        for (const auto &card : response) {
            if (card.is_null()) {
                throw std::runtime_error("AnkiConnect error: can't create a card");
            }
        }
    }
    return response.at("result");
}
