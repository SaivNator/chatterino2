#include "providers/twitch/TwitchApi.hpp"

#include "common/NetworkRequest.hpp"
#include "debug/Log.hpp"
#include "providers/twitch/TwitchCommon.hpp"

#include <QString>

namespace chatterino {

void TwitchApi::findUserId(const QString user, std::function<void(QString)> successCallback)
{
    QString requestUrl("https://api.twitch.tv/kraken/users?login=" + user);

    NetworkRequest request(requestUrl);
    request.setCaller(QThread::currentThread());
    request.makeAuthorizedV5(getDefaultClientID());
    request.setTimeout(30000);
    request.onSuccess([successCallback](auto result) mutable {
        auto root = result.parseJson();
        if (!root.value("users").isArray()) {
            Log("API Error while getting user id, users is not an array");
            successCallback("");
            return false;
        }
        auto users = root.value("users").toArray();
        if (users.size() != 1) {
            Log("API Error while getting user id, users array size is not 1");
            successCallback("");
            return false;
        }
        if (!users[0].isObject()) {
            Log("API Error while getting user id, first user is not an object");
            successCallback("");
            return false;
        }
        auto firstUser = users[0].toObject();
        auto id = firstUser.value("_id");
        if (!id.isString()) {
            Log("API Error: while getting user id, first user object `_id` key is not a "
                "string");
            successCallback("");
            return false;
        }
        successCallback(id.toString());
        return true;
    });

    request.execute();
}

}  // namespace chatterino
