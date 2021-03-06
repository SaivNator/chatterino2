#include "Application.hpp"
#include "common/NetworkManager.hpp"
#include "singletons/NativeMessaging.hpp"
#include "singletons/Paths.hpp"
#include "singletons/Updates.hpp"
#include "util/DebugCount.hpp"
#include "widgets/dialogs/LastRunCrashDialog.hpp"

#include <QAbstractNativeEventFilter>
#include <QApplication>
#include <QFile>
#include <QLibrary>
#include <QStringList>
#include <QStyleFactory>
#include <pajlada/settings/settingmanager.hpp>

#include <fstream>
#include <iostream>

#ifdef Q_OS_WIN
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#endif

#ifdef C_USE_BREAKPAD
#include <QBreakpadHandler.h>
#endif

int runGui(QApplication &a, int argc, char *argv[]);
void runNativeMessagingHost();
void installCustomPalette();

//
// Main entry point of the application.
// Decides if it should run in gui mode, daemon mode, ...
// Sets up the QApplication
//
int main(int argc, char *argv[])
{
    // set up the QApplication flags
    QApplication::setAttribute(Qt::AA_Use96Dpi, true);
#ifdef Q_OS_WIN32
    QApplication::setAttribute(Qt::AA_DisableHighDpiScaling, true);
#endif
    //    QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL, true);

    // instanciate the QApplication
    QApplication a(argc, argv);

    // FOURTF: might get arguments from the commandline passed in the future
    chatterino::Paths::initInstance();

    // read args
    QStringList args;

    for (int i = 1; i < argc; i++) {
        args << argv[i];
    }

    // run native messaging host for the browser extension
    if (args.size() > 0 &&
        (args[0].startsWith("chrome-extension://") || args[0].endsWith(".json"))) {
        runNativeMessagingHost();
        return 0;
    }

    // run gui
    return runGui(a, argc, argv);
}

int runGui(QApplication &a, int argc, char *argv[])
{
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    installCustomPalette();

    // Initialize NetworkManager
    chatterino::NetworkManager::init();

    // Check for upates
    chatterino::Updates::getInstance().checkForUpdates();

    // Initialize application
    chatterino::Application::instantiate(argc, argv);
    auto app = chatterino::getApp();

    app->construct();

#ifdef C_USE_BREAKPAD
    QBreakpadInstance.setDumpPath(app->paths->settingsFolderPath + "/Crashes");
#endif

    auto &pathMan = *app->paths;
    // Running file
    auto runningPath = pathMan.miscDirectory + "/running_" + pathMan.applicationFilePathHash;

    if (QFile::exists(runningPath)) {
#ifndef C_DISABLE_CRASH_DIALOG
        chatterino::LastRunCrashDialog dialog;

        switch (dialog.exec()) {
            case QDialog::Accepted: {
            }; break;
            default: {
                _exit(0);
            }
        }
#endif
    } else {
        QFile runningFile(runningPath);

        runningFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
        runningFile.flush();
        runningFile.close();
    }

    app->initialize();

    // Start the application
    // This is a blocking call
    app->run(a);

    // We have finished our application, make sure we save stuff
    app->save();

    // Running file
    QFile::remove(runningPath);

    // Save settings
    pajlada::Settings::SettingManager::gSave();

    // Deinitialize NetworkManager (stop thread and wait for finish, should be instant)
    chatterino::NetworkManager::deinit();

    // None of the singletons has a proper destructor
    _exit(0);
}

void runNativeMessagingHost()
{
    auto *nm = new chatterino::NativeMessaging;

#ifdef Q_OS_WIN
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif

#if 0
    bool bigEndian = isBigEndian();
#endif

    std::atomic<bool> ping(false);

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&ping] {
        if (!ping.exchange(false)) {
            _exit(0);
        }
    });
    timer.setInterval(11000);
    timer.start();

    while (true) {
        char size_c[4];
        std::cin.read(size_c, 4);

        if (std::cin.eof()) {
            break;
        }

        uint32_t size = *reinterpret_cast<uint32_t *>(size_c);
#if 0
        // To avoid breaking strict-aliasing rules and potentially inducing undefined behaviour, the following code can be run instead
        uint32_t size = 0;
        if (bigEndian) {
            size = size_c[3] | static_cast<uint32_t>(size_c[2]) << 8 |
                   static_cast<uint32_t>(size_c[1]) << 16 | static_cast<uint32_t>(size_c[0]) << 24;
        } else {
            size = size_c[0] | static_cast<uint32_t>(size_c[1]) << 8 |
                   static_cast<uint32_t>(size_c[2]) << 16 | static_cast<uint32_t>(size_c[3]) << 24;
        }
#endif

        std::unique_ptr<char[]> b(new char[size + 1]);
        std::cin.read(b.get(), size);
        *(b.get() + size) = '\0';

        nm->sendToGuiProcess(QByteArray::fromRawData(b.get(), static_cast<int32_t>(size)));
    }
}

void installCustomPalette()
{
    // borrowed from
    // https://stackoverflow.com/questions/15035767/is-the-qt-5-dark-fusion-theme-available-for-windows
    QPalette darkPalette = qApp->palette();

    darkPalette.setColor(QPalette::Window, QColor(22, 22, 22));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Base, QColor("#333"));
    darkPalette.setColor(QPalette::AlternateBase, QColor("#444"));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Dark, QColor(35, 35, 35));
    darkPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
    darkPalette.setColor(QPalette::Button, QColor(70, 70, 70));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));

    qApp->setPalette(darkPalette);
}
