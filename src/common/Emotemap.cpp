#include "Emotemap.hpp"

#include "Application.hpp"
#include "singletons/Settings.hpp"

namespace chatterino {

EmoteData::EmoteData(Image *image)
    : image1x(image)
{
}

// Emotes must have a 1x image to be valid
bool EmoteData::isValid() const
{
    return this->image1x != nullptr;
}

Image *EmoteData::getImage(float scale) const
{
    int quality = getApp()->settings->preferredEmoteQuality;

    if (quality == 0) {
        scale *= getApp()->settings->emoteScale.getValue();
        quality = [&] {
            if (scale <= 1)
                return 1;
            if (scale <= 2)
                return 2;
            return 3;
        }();
    }

    Image *_image;
    if (quality == 3 && this->image3x != nullptr) {
        _image = this->image3x;
    } else if (quality >= 2 && this->image2x != nullptr) {
        _image = this->image2x;
    } else {
        _image = this->image1x;
    }

    return _image;
}

}  // namespace chatterino
