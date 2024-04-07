#include "network.h"

#include "myexception.h"



void NetworkClient::init(int maxCurls) {
    if (initialised) return;

    curl_global_init(CURL_GLOBAL_ALL);
    for (int i = 0; i < maxCurls;) {
        CURL *curl = curl_easy_init ();
        if (curl) {
            // Set the timeouts
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);

            curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

            // Set the user agent
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");

            // Set SSL options
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
            //curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
            curls.push_back(curl);
            i++;
        }
    }
    initialised = true;
}

void NetworkClient::cleanUp() {
    if (!initialised) return;
    for (auto& curl: curls){
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup ();
    curls.clear ();
    initialised = false;
}

bool NetworkClient::isUrlValid(const QString &url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw MyException("Failed to initialize CURL.");
    }
    auto urlString = url.toStdString ();
    curl_easy_setopt(curl, CURLOPT_URL, urlString.c_str());
    curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);

    CURLcode res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    return res == CURLE_OK;
}

NetworkClient::Response NetworkClient::get(const QString &url, const QMap<QString, QString> &headers, const QMap<QString, QString> &params)
{
    auto fullUrl = url;
    if (!params.isEmpty ()) {
        for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
            fullUrl += "&" + it.key () + "=" + it.value ();
        }
    }

    return request(GET, fullUrl, headers);
}

NetworkClient::Response NetworkClient::post(const QString &url, const QMap<QString, QString> &headers, const QMap<QString, QString> &data){
    // Set the post data
    QString postData;
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        postData += it.key() + "=" + it.value() + "&";
    }
    return request(POST, url, headers, postData);
}

NetworkClient::Response NetworkClient::request(int type, const QString &url, const QMap<QString, QString> &headersMap, const QString &data){
    CURL* curl = getCurl();
    if (!curl) {
        throw MyException("Failed to get curl");
    }

    Response response;
    std::string urlData = url.toStdString();
    curl_easy_setopt(curl, CURLOPT_URL, urlData.c_str ());

    struct curl_slist* headers = nullptr;
    for (auto it = headersMap.constBegin(); it != headersMap.constEnd(); ++it) {
        auto entry = QString(it.key() + ": " + it.value()).toStdString ();
        headers = curl_slist_append(headers, entry.c_str ());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    switch (type) {
    case POST:
        std::string dataString = data.toStdString ();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dataString.c_str ());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        break;
    }


    // Set the response callback function
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response.headers);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);

    // Perform the GET request
    CURLcode res = curl_easy_perform(curl);

    // Get the response code
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.code);
    // Check for errors
    if (res != CURLE_OK){
        throw MyException(QString("curl_easy_perform() failed: ") + curl_easy_strerror(res));
    }

    // Clean up
    curl_slist_free_all(headers);
    returnCurl (curl);
    return response;
}

size_t NetworkClient::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalBytes(size * nmemb);

    // Cast userp to QString pointer
    QString* str = static_cast<QString*>(userp);

    // Convert the incoming data to QString assuming it's UTF-8 encoded
    // If your data is in another encoding, adjust this part accordingly
    QString newData = QString::fromUtf8(static_cast<char*>(contents), static_cast<int>(totalBytes));

    // Append the new data to the provided QString
    str->append(newData);

    // Return the number of bytes taken care of
    return totalBytes;
}

size_t NetworkClient::HeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata) {
    // Calculate the total size of the incoming header data
    size_t numbytes = size * nitems;
    // Cast userdata to QString pointer
    QString* header = static_cast<QString*>(userdata);
    // Convert the incoming header data to QString assuming it's UTF-8 encoded
    QString newData = QString::fromUtf8(buffer, static_cast<int>(numbytes));
    // Append the new header data to the provided QString
    header->append(newData);
    // Return the number of bytes processed
    return numbytes;
}
