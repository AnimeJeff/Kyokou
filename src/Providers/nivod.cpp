#include "nivod.h"

QList<ShowData> Nivod::filterSearch(int page, const QString &sortBy, int type,
                                    const QString &regionId,
                                    const QString &langId,
                                    const QString &yearRange) {
    std::string channel = type == ShowData::DOCUMENTARY ?  "6" : std::to_string(type);
    std::map<std::string, std::string> data = {
                                               {"sort_by", sortBy.toStdString()},
                                               {"channel_id", channel},
                                               {"show_type_id", "0"},
                                               {"region_id", "0"},
                                               {"lang_id", "0"},
                                               {"year_range", " "},
                                               {"start", std::to_string((page - 1) * 20)}};

    std::string response = callAPI("https://api.nivodz.com/show/filter/WEB/3.2", data);
    QList<ShowData> results = showsFromJsonArray(nlohmann::json::parse(response)["list"]);
    return results;
}

std::string Nivod::callAPI(const std::string &url, const std::map<std::string, std::string> &data) const {
    std::string sign = createSign(data);

    std::string postUrl = url + "?_ts=" + _mts +
                          "&app_version=1.0&platform=3&market_id=web_nivod&"
                          "device_code=web&versioncode=1&oid=" +
                          _oid + "&sign=" + sign;

    auto response = NetworkClient::post(postUrl, mudvodHeaders, data).body;
    try {
        return decryptedByDES(response);
    } catch (std::exception &e) {
        qDebug() << "Bad Input" << postUrl;
    }
    return "";
}

QList<ShowData> Nivod::search(QString query, int page, int type) {
    if (query.isEmpty())
        return QList<ShowData>();
    std::map<std::string, std::string> data = {
                                               {"keyword", query.toStdString()},
                                               {"start", std::to_string((page - 1) * 20)},
                                               {"cat_id", "1"},
                                               {"keyword_type", "0"}};
    filterSearched = false;
    std::string response = callAPI("https://api.nivodz.com/show/search/WEB/3.2", data);
    QList<ShowData> results = showsFromJsonArray(nlohmann::json::parse(response)["list"]);
    return results;
}

QList<ShowData> Nivod::popular(int page, int type) {
    return filterSearch(page, "1", type);
}

QList<ShowData> Nivod::latest(int page, int type) {
    return filterSearch(page, "4", type);
}

void Nivod::loadDetails(ShowData &show) const {
    nlohmann::json infoJson = getInfoJson(show.link);
    show.description =
        QString::fromStdString(infoJson["showDesc"].get<std::string>());
    if (infoJson.contains("episodesUpdateDesc") &&
        !infoJson.at("episodesUpdateDesc").is_null()) {
        show.updateTime = QString::fromStdString(
            infoJson["episodesUpdateDesc"].get<std::string>());
    }
    if (!infoJson["showTypeName"].is_null())
        show.genres +=
            QString::fromStdString(infoJson["showTypeName"].get<std::string>());
    //        auto actors =
    //        QString::fromStdString(infoJson["actors"].get<std::string>());

    if (!infoJson["hot"].is_null())
        show.views =
            QString::fromStdString(std::to_string(infoJson["hot"].get<int>()));
    if (!infoJson["rating"].is_null())
        show.rating = QString::fromStdString(
            std::to_string((infoJson["rating"].get<int>()) / 10));

    for (const auto &item : infoJson["plays"].items()) {
        auto episode = item.value();
        QString name =
            QString::fromStdString(episode["displayName"].get<std::string>());
        int number = -1;

        bool ok;
        int intTitle = name.toInt(&ok);
        if (ok) {
            number = intTitle;
            name = "";
        }
        std::string link =
            show.link + "&" + episode["playIdCode"].get<std::string>();
        show.addEpisode(number, link, name);
        show.totalEpisodes++;
    }
}

QList<VideoServer> Nivod::loadServers(const PlaylistItem *episode) const {
    // {{"referer", "https://www.nivod.tv/"}}/
    return { VideoServer{"default", episode->link} };
}

QMap<QString, QString> Nivod::objKeySort(const QMap<QString, QString> &inputMap) const {

    if (inputMap.isEmpty())
        return inputMap;

    QMap<QString, QString> sortedMap;
    for (const auto &key : inputMap.keys()) {
        const QString &value = inputMap.value(key);
        if (key.isEmpty() || value.isEmpty() || key == "sign")
            continue;
        sortedMap.insert(key, value);
    }
    return sortedMap;
}

nlohmann::json Nivod::getInfoJson(const std::string &link) const {
    auto response = callAPI("https://api.nivodz.com/show/detail/WEB/3.3",
                            {{"show_id_code", link}, {"episode_id", "0"}});
    // qDebug() << QString::fromStdString (response);
    return nlohmann::json::parse(response)["entity"];
}

int Nivod::getTotalEpisodes(const std::string &link) const {
    return getInfoJson(link)["plays"].size();
}

QString Nivod::extractSource(const VideoServer &server) const {
    auto codes = Functions::split(server.link, '&');
    std::map<std::string, std::string> data = {{"play_id_code", codes[1]},
                                               {"show_id_code", codes[0]},
                                               {"oid", "1"},
                                               {"episode_id", "0"}};
    auto playUrl = nlohmann::json::parse(
                       callAPI("https://api.nivodz.com/show/play/info/WEB/3.3",
                               data))["entity"]["plays"][0]["playUrl"]
                       .get<std::string>(); // video quality
    // qDebug() << QString::fromStdString(playUrl);
    return QString::fromStdString(playUrl);
}

std::string Nivod::createSign(const std::map<std::string, std::string> &bodyMap,
                              const std::string &secretKey) const {

    std::string signQuery = _QUERY_PREFIX;
    for (auto it = queryMap.begin(); it != queryMap.end(); ++it) {
        signQuery += it->first + "=" + it->second + "&";
    }

    signQuery += _BODY_PREFIX;
    for (auto it = bodyMap.begin(); it != bodyMap.end(); ++it) {
        signQuery += it->first + "=" + it->second + "&";
    }

    std::string input = signQuery + _SECRET_PREFIX + secretKey;
    //    qDebug() << input;
    return Functions::MD5(input);
}

std::string Nivod::decryptedByDES(const std::string &input) const {
    std::string key = "diao.com";
    std::vector<byte> keyBytes(key.begin(), key.end());
    std::vector<byte> inputBytes;
    for (size_t i = 0; i < input.length(); i += 2) {
        byte byte =
            static_cast<unsigned char>(std::stoi(input.substr(i, 2), nullptr, 16));
        inputBytes.push_back(byte);
    }

    size_t length = inputBytes.size();
    size_t padding = length % 8 == 0 ? 0 : 8 - length % 8;
    inputBytes.insert(inputBytes.end(), padding, 0);

    std::vector<byte> outputBytes(length + padding, 0);
    CryptoPP::ECB_Mode<CryptoPP::DES>::Decryption decryption(keyBytes.data(),
                                                             keyBytes.size());
    CryptoPP::ArraySink sink(outputBytes.data(), outputBytes.size());
    CryptoPP::ArraySource source(inputBytes.data(), inputBytes.size(), true,
                                 new CryptoPP::StreamTransformationFilter(
                                     decryption, new CryptoPP::Redirector(sink)));
    std::string decrypted(outputBytes.begin(), outputBytes.end());
    size_t pos = decrypted.find_last_of('}');
    if (pos != std::string::npos) {
        decrypted = decrypted.substr(0, pos + 1);
    }
    return decrypted;
}

int Nivod::getShowType(const std::string &channelName) const {
    if (channelName == "电影") {
        return ShowData::MOVIE;
    } else if (channelName == "电视剧") {
        return ShowData::TVSERIES;
    } else if (channelName == "综艺") {
        return ShowData::VARIETY;
    } else if (channelName == "动漫") {
        return ShowData::ANIME;
    } else if (channelName == "纪录片") {
        return ShowData::DOCUMENTARY;
    }
    qDebug() << "Log (Nivod): Cannot infer show type from"
             << QString::fromStdString(channelName);
    return ShowData::NONE;
}

QList<ShowData> Nivod::showsFromJsonArray(const nlohmann::json &showList,
                                          int type) {
    QList<ShowData> results;
    for (auto &el : showList.items()) {
        auto item = el.value();
        int tvType = getShowType(item["channelName"].get<std::string>());
        //            if (item["showTitle"].get<std::string>().find ("假面骑士"));
        QString title = QString::fromStdString(item["showTitle"].get<std::string>());
        QString coverUrl = QString::fromStdString(item["showImg"].get<std::string>());
        std::string link = item["showIdCode"].get<std::string>();
        QString latestTxt = "";
        if (!item["episodesTxt"].is_null()) {
            latestTxt = QString::fromStdString(item["episodesTxt"].get<std::string>());
            //                int status = ShowData::Status::Completed;
            //                if (results.last ().latestTxt.contains("更新")){
            //                    results.last ().status = ShowData::Status::Ongoing;
            //                }
        }
        results.emplaceBack(title, link, coverUrl, this, latestTxt, tvType);
        if (!item["postYear"].is_null()) {
            results.last().releaseDate =
                QString::fromStdString(std::to_string(item["postYear"].get<int>()));
        }
    }

    return results;
}
