#-------------------------------------------------
#
# Project created by QtCreator 2021-03-21T11:51:15
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tiktok-downloader
TEMPLATE = app

# Set program version
VERSION = 0.0.1
DEFINES += VERSIONSTR=\\\"$${VERSION}\\\"


CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT


# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        account.cpp \
        claimoffer.cpp \
        cookiejar.cpp \
        downloadpage.cpp \
        help.cpp \
        history.cpp \
        main.cpp \
        mainwindow.cpp \
        request.cpp \
        rungaurd.cpp \
        trending.cpp \
        utils.cpp \
        widgets/elidedLabel/elidedlabel.cpp \
        widgets/waitingSpinner/waitingspinnerwidget.cpp

HEADERS += \
        account.h \
        claimoffer.h \
        cookiejar.h \
        downloadpage.h \
        gridlayoututil.h \
        help.h \
        history.h \
        mainwindow.h \
        request.h \
        rungaurd.h \
        trending.h \
        utils.h \
        widgets/elidedLabel/elidedlabel.h \
        widgets/waitingSpinner/waitingspinnerwidget.h

FORMS += \
        account.ui \
        claimoffer.ui \
        downloadpage.ui \
        help.ui \
        history.ui \
        mainwindow.ui \
        trending.ui \
        trending_option.ui

# Default rules for deployment.
isEmpty(PREFIX){
 PREFIX = /usr
}

BINDIR  = $$PREFIX/bin
DATADIR = $$PREFIX/share

target.path = $$BINDIR

icon.files = icons/tiktok-downloader.png
icon.path = $$DATADIR/icons/hicolor/512x512/apps/

desktop.files = tiktok-downloader.desktop
desktop.path = $$DATADIR/applications/

INSTALLS += target icon desktop

RESOURCES += \
    icons.qrc \
    theme/qbreeze.qrc

