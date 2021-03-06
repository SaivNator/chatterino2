#include "singletons/helper/GifTimer.hpp"

#include "Application.hpp"
#include "singletons/Settings.hpp"
#include "singletons/WindowManager.hpp"

namespace chatterino {

void GIFTimer::initialize()
{
    this->timer.setInterval(30);

    getApp()->settings->enableGifAnimations.connect([this](bool enabled, auto) {
        if (enabled) {
            this->timer.start();
        } else {
            this->timer.stop();
        }
    });

    QObject::connect(&this->timer, &QTimer::timeout, [this] {
        this->signal.invoke();
        // fourtf:
        auto app = getApp();
        app->windows->repaintGifEmotes();
    });
}

}  // namespace chatterino
