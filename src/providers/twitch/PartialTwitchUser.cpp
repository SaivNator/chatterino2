#include "providers/twitch/PartialTwitchUser.hpp"

#include "common/NetworkRequest.hpp"
#include "debug/Log.hpp"
#include "providers/twitch/TwitchCommon.hpp"

#include <cassert>

namespace chatterino {

PartialTwitchUser PartialTwitchUser::byName(const QString &username)
{
    PartialTwitchUser user;
    user.username_ = username;

    return user;
}

PartialTwitchUser PartialTwitchUser::byId(const QString &id)
{
    PartialTwitchUser user;
    user.id_ = id;

    return user;
}

void PartialTwitchUser::getId(std::function<void(QString)> successCallback, const QObject *caller)
{
    assert(!this->username_.isEmpty());

    if (caller == nullptr) {
        caller = QThread::currentThread();
    }

    NetworkRequest request("https://api.twitch.tv/kraken/users?login=" + this->username_);
    request.setCaller(caller);
    request.makeAuthorizedV5(getDefaultClientID());

    request.onSuccess([successCallback](auto result) {
        auto root = result.parseJson();
        if (!root.value("users").isArray()) {
            Log("API Error while getting user id, users is not an array");
            return false;
        }

        auto users = root.value("users").toArray();
        if (users.size() != 1) {
            Log("API Error while getting user id, users array size is not 1");
            return false;
        }
        if (!users[0].isObject()) {
            Log("API Error while getting user id, first user is not an object");
            return false;
        }
        auto firstUser = users[0].toObject();
        auto id = firstUser.value("_id");
        if (!id.isString()) {
            Log("API Error: while getting user id, first user object `_id` key is not a "
                "string");
            return false;
        }
        successCallback(id.toString());

        return true;
    });

    request.execute();
}

}  // namespace chatterino
