#include "AccountController.hpp"

#include "controllers/accounts/AccountModel.hpp"

namespace chatterino {

AccountController::AccountController()
{
    this->twitch.accounts.itemInserted.connect([this](const auto &args) {
        this->accounts_.insertItem(std::dynamic_pointer_cast<Account>(args.item));
    });

    this->twitch.accounts.itemRemoved.connect([this](const auto &args) {
        if (args.caller != this) {
            auto &accs = this->twitch.accounts.getVector();
            auto it = std::find(accs.begin(), accs.end(), args.item);
            assert(it != accs.end());

            this->accounts_.removeItem(it - accs.begin());
        }
    });

    this->accounts_.itemRemoved.connect([this](const auto &args) {
        switch (args.item->getProviderId()) {
            case ProviderId::Twitch: {
                auto &accs = this->twitch.accounts.getVector();
                auto it = std::find(accs.begin(), accs.end(), args.item);
                assert(it != accs.end());

                this->twitch.accounts.removeItem(it - accs.begin(), this);
            } break;
        }
    });
}

void AccountController::initialize(Application &app)
{
    this->twitch.load();
}

AccountModel *AccountController::createModel(QObject *parent)
{
    AccountModel *model = new AccountModel(parent);

    model->init(&this->accounts_);
    return model;
}

}  // namespace chatterino
