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
        {"start", std::to_string((page - 1) * 20)}
    };

    QJsonArray responseJson = callAPI("https://api.nivodz.com/show/filter/WEB/3.2", data)["list"].toArray ();
    return parseShows(responseJson);

}

QList<ShowData> Nivod::search(QString query, int page, int type) {
    if (query.isEmpty())
        return QList<ShowData>();
    std::map<std::string, std::string> data = {
        {"keyword", query.toStdString()},
        {"start", std::to_string((page - 1) * 20)},
        {"cat_id", "1"},
        {"keyword_type", "0"}
    };
    filterSearched = false;
    QJsonArray responseJson = callAPI("https://api.nivodz.com/show/search/WEB/3.2", data)["list"].toArray();
    return parseShows(responseJson);
}

QList<ShowData> Nivod::parseShows(const QJsonArray &showArrayJson)
{
    QList<ShowData> results;
    foreach (const QJsonValue &value, showArrayJson) {
        QJsonObject item = value.toObject();
        int tvType;
        QString channelName = item["channelName"].toString();
        if (channelName == "电影") {
            tvType = ShowData::MOVIE;
        } else if (channelName == "电视剧") {
            tvType = ShowData::TVSERIES;
        } else if (channelName == "综艺") {
            tvType = ShowData::VARIETY;
        } else if (channelName == "动漫") {
            tvType = ShowData::ANIME;
        } else if (channelName == "纪录片") {
            tvType = ShowData::DOCUMENTARY;
        } else {
            qDebug() << "Log (Nivod): Cannot infer show type from" << channelName;
            tvType = ShowData::NONE;
        }

        QString title = item["showTitle"].toString();
        QString coverUrl = item["showImg"].toString();
        std::string link = item["showIdCode"].toString().toStdString ();
        QString latestTxt;
        if (!item["episodesTxt"].isUndefined() && !item["episodesTxt"].isNull()) {
            latestTxt = item["episodesTxt"].toString();
        }
        // QJsonDocument doc(item);
        // QString jsonString = doc.toJson(QJsonDocument::Indented);
        // qDebug().noquote() << jsonString;

        results.emplaceBack(title, link, coverUrl, this, latestTxt, tvType);

    }

    return results;
}


QJsonObject Nivod::getInfoJson(const std::string &link) const {
    return callAPI("https://api.nivodz.com/show/detail/WEB/3.3",
                   {{"show_id_code", link}, {"episode_id", "0"}})["entity"].toObject ();
}

void Nivod::loadDetails(ShowData &show) const {
    QJsonObject infoJson = getInfoJson(show.link);

    show.description = infoJson["showDesc"].toString ();
    show.genres += infoJson["showTypeName"].toString ();
    show.status = infoJson["episodesTxt"].toString ();
    //        auto actors =
    //        QString::fromStdString(infoJson["actors"].get<std::string>());
    show.releaseDate = QString::number(infoJson["postYear"].toInt(69));
    show.views = QString::number (infoJson["hot"].toInt (69));
    show.score = QString::number (infoJson["rating"].toInt (69));
    show.updateTime = infoJson["episodesUpdateDesc"].toString ();
    foreach (const QJsonValue &item, infoJson["plays"].toArray ()) {
        auto episode = item.toObject ();
        QString title = episode["displayName"].toString ();
        float number = -1;
        bool ok;
        float intTitle = title.toFloat(&ok);
        if (ok) {
            number = intTitle;
            title = "";
        }
        QString link = QString::fromStdString (show.link) + "&" + episode["playIdCode"].toString ();
        show.addEpisode(number, link, title);
    }
}



QList<Video> Nivod::extractSource(const VideoServer &server) const {
    auto codes = server.link.split ('&');
    std::map<std::string, std::string> data = {{"play_id_code", codes.last ().toStdString ()},
                                               {"show_id_code", codes.first ().toStdString ()},
                                               {"oid", "1"},
                                               {"episode_id", "0"}};
    auto responseJson = callAPI("https://api.nivodz.com/show/play/info/WEB/3.3", data);
    auto source = responseJson["entity"].toObject()["plays"].toArray()[0].toObject()["playUrl"].toString ();
    // QJsonDocument doc(responseJson);
    // QString jsonString = doc.toJson(QJsonDocument::Indented);
    // qDebug().noquote() << jsonString;

    // qDebug() << QString::fromStdString(playUrl);
    return { Video(source) };
}


QJsonObject Nivod::callAPI(const std::string &url, const std::map<std::string, std::string> &data) const {
    std::string sign = createSign(data);

    std::string postUrl = url + "?_ts=" + _mts +
                          "&app_version=1.0&platform=3&market_id=web_nivod&"
                          "device_code=web&versioncode=1&oid=" +
                          _oid + "&sign=" + sign;

    std::string response = NetworkClient::post(postUrl, mudvodHeaders, data).body;
    QJsonParseError error;
    QJsonDocument jsonData = QJsonDocument::fromJson(decryptedByDES(response).c_str (),&error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parsing error:" << error.errorString();
        return QJsonObject{};
    }
    return jsonData.object ();


}

std::string Nivod::createSign(const std::map<std::string, std::string> &bodyMap, const std::string &secretKey) const {

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



// QMap<QString, QString> Nivod::objKeySort(const QMap<QString, QString> &inputMap) const {

//     if (inputMap.isEmpty())
//         return inputMap;

//     QMap<QString, QString> sortedMap;
//     for (const auto &key : inputMap.keys()) {
//         const QString &value = inputMap.value(key);
//         if (key.isEmpty() || value.isEmpty() || key == "sign")
//             continue;
//         sortedMap.insert(key, value);
//     }
//     return sortedMap;
// }
