#include "curl_request.h"
#include <curl/curl.h>


CurlSession::CurlSession()
{
    if (auto res = curl_global_init(CURL_GLOBAL_ALL); res != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(res));
    }
    if (!(curl = curl_easy_init())) {
        throw std::runtime_error("Can't init curl library");
    }
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1);
}

CurlSession::~CurlSession()
{
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

void CurlSession::set_json_headers()
{
    set_header("Accept: application/json");
    set_header("Content-Type: application/json");
    set_header("charsets: utf-8");
}

void CurlSession::set_header(const char *value)
{
    headers = curl_slist_append(headers, value);
}

void CurlSession::set_useragent(const char *value)
{
    curl_easy_setopt(curl, CURLOPT_USERAGENT, value);
}

void CurlSession::set_verbose(bool value)
{
    curl_easy_setopt(curl, CURLOPT_VERBOSE, value);
}

void CurlSession::clear_headers()
{
    curl_slist_free_all(headers);
    headers = nullptr;
}

std::string CurlSession::get(const char *url)
{
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    return perform_request(url);
}

std::string CurlSession::post(const char *url, const std::string &data)
{
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
    return perform_request(url);
}

std::string CurlSession::put(const char *url, const std::string &data)
{
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
    return perform_request(url);
}

std::string CurlSession::perform_request(const char *url)
{
    std::string result;
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
    if (CURLcode res;
            (res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers)) != CURLE_OK ||
            (res = curl_easy_perform(curl)) != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(res));
    }
    return result;
}

size_t CurlSession::read_callback(void *data, size_t size, size_t nmemb, void *ptr)
{
    static_cast<std::string *>(ptr)->append(static_cast<char *>(data), (size *= nmemb));
    return size;
}
