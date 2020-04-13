#ifndef CURL_REQUEST_H
#define CURL_REQUEST_H

#include <string>

typedef void CURL;

class CurlSession
{
public:
    CurlSession();
    ~CurlSession();

    void set_json_headers();
    void set_header(const char *value);
    void set_useragent(const char *value);
    void set_verbose(bool value);

    void clear_headers();

    std::string get(const char *url);
    std::string post(const char *url, const std::string &data);
    std::string put(const char *url, const std::string &data);

private:
    std::string perform_request(const char *url);
    static size_t read_callback(void *data, size_t size, size_t nmemb, void *ptr);

private:
    CURL *curl;
    struct curl_slist *headers = nullptr;
};

#endif // CURL_REQUEST_H
