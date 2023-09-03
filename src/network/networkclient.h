#ifndef CLIENT_H
#define CLIENT_H

#include "curl/curl.h"
#include "CSoup.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <memory>
#include <QDebug>
#define StringMap std::map<std::string, std::string>

class NetworkClient
{
private:
    static constexpr int maxCurls = 10;

    static std::vector<CURL*> curls;

    static void createHandles(){
        for( int i=0; i<maxCurls; ){
            CURL *curl = curl_easy_init ();
            if (curl){
                initHandle(curl);
                curls.push_back(curl);
                i++;
            }
        }
    };

    static void initHandle(CURL* curl){
        if (curl) {
            // Set the timeouts
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);

            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

            // Set the user agent
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3");

            // Set SSL options
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, defaultUA.c_str());
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        }
    }

    static CURL* getCurl(){
        static std::mutex mutex;
        mutex.lock();
        CURL* curl = curls.back ();
        curls.pop_back ();
        mutex.unlock();
        return curl;
    }

    static void returnCurl(CURL* curl){
        static std::mutex mutex;
        mutex.lock();
        curls.push_back(curl);
        mutex.unlock();
    }

    static bool initialised;

    enum RequestType{
        GET,
        POST,
        HEAD,
        PUT,
        DELETE_,
        CONNECT,
        OPTIONS
    };
public:
    static void init(){
        if(initialised) return;
        curl_global_init(CURL_GLOBAL_ALL);
        createHandles();
        initialised = true;
    }

    static void shutdown(){
        if(!initialised) return;
        for(auto& curl: curls){
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup ();
        curls.clear ();
        initialised = false;
    }

public:
    struct Response{
        long code;
        std::string url;
        std::string redirectUrl;
        //        std::map<std::string, std::string> headers;
        std::string headers;
        std::string body;
        std::map<std::string, std::string> cookies;
        CSoup document(){
            return CSoup(body);
        }
        nlohmann::json json(){
            return nlohmann::json::parse (body);
        }
        ~Response(){
        }
        friend QDebug operator<<(QDebug debug, const Response& response)
        {
            debug << QString::fromStdString (response.body);
            return debug;
        }
    };

    static Response get(const std::string &url, const StringMap& headers={}, const StringMap& params = {})
    {
        std::stringstream ss;
        std::string queryParams;

        // Build the query parameters
        if (!params.empty()) {
            ss << "?";
            for (auto const& [key, value] : params) {
                ss << key << "=" << value << "&";
            }
            queryParams = ss.str();
            // remove the last '&'
            queryParams.pop_back();
        }

        // Set the URL
        std::string fullUrl = url + queryParams;
        return request(GET,fullUrl.c_str ());
    }

    static Response post(const std::string& url, const StringMap& headers={}, const StringMap& data={}) {
        // Set the post data
        std::string postData;
        for (auto const& x : data) {
            postData += std::string(x.first) + "=" + std::string(x.second) + "&";
        }
        return request(POST,url.c_str (),headers,postData.c_str());
    }

    static Response request(int type,const char* url,const StringMap& headers={},const char* data = nullptr){
        CURL* curl = getCurl ();
        Response response;

        curl_easy_setopt(curl, CURLOPT_URL, url);

        // Set the headers
        struct curl_slist* headerLista = NULL;
        StringMap _headers = headers;
        if(headers.empty ()){
            _headers = defaultHeaders;
        }
        for (auto const& [key, value] : _headers) {
            std::string header = std::string(key) + ":" + std::string(value);
            headerLista = curl_slist_append(headerLista, header.c_str());
        }

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerLista);
        if(type == POST){
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        }
        // Set the response callback function
        std::string readBuffer;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response.headers);

        // Perform the GET request
        CURLcode res = curl_easy_perform(curl);

        // Get the response code
        long code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

        // Check for errors
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        // Clean up
        curl_slist_free_all(headerLista);

        // Parse the response
        response.url = url;
        response.body = readBuffer;
        response.code = code;
        returnCurl (curl);
        return response;
    }

private:
    NetworkClient() = default;
    ~NetworkClient() = default;
    // WriteCallback function to handle the response body
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*) userp)->append((char*) contents, size * nmemb);
        return size * nmemb;
    }

    // HeaderCallback function to handle the response headers
    static size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
        size_t numbytes = size * nitems;
        std::string* header = static_cast<std::string*>(userdata);
        header->append(buffer, numbytes);
        return numbytes;
    }

private:
    static StringMap defaultHeaders;
    static std::string defaultUA;
};


#endif // CLIENT_H
