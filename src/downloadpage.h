#ifndef DOWNLOADPAGE_H
#define DOWNLOADPAGE_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QtNetwork>
#include <QStandardPaths>
#include <QSettings>
#include <QMessageBox>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include "widgets/waitingSpinner/waitingspinnerwidget.h"
#include "cookiejar.h"

namespace Ui {
class DownloadPage;
}

class DownloadPage : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadPage(QWidget *parent = nullptr);
    ~DownloadPage();

public slots:
    void getResult(QUrl url);

private slots:
    void on_close_button_clicked();

    void init_networkManager();
    void cancelAllRequest();
    void requestError(QString errorStr);
    void parseDownloadOptions(QByteArray rep);
    void download(QString url);
    void downloadProgress(qint64 got, qint64 tot);
    void on_done_button_clicked();

    static QString humanReadableSize(double bytes)
    {
        double cache_size = static_cast<double>(bytes);
        QString cache_unit;
        if(cache_size > 1024*1024*1024)
        {
            cache_size = cache_size/(1024*1024*1024);
            cache_unit = " GB";
        }else if(cache_size > 1024*1024)
        {
            cache_size = cache_size/(1024*1024);
            cache_unit = " MB";
        }
        else if(cache_size > 1024)
        {
            cache_size = cache_size/(1024);
            cache_unit = " kB";
        }
        else
        {
            cache_unit = " B";
        }
        return QString::number(cache_size,'f',2) + cache_unit;
    }
    //static on demand path maker
    QString returnPath(QString pathname)
    {
        auto sepe = QDir::separator();
        QString _data_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
        if(!QDir(_data_path).exists()){
            QDir d;
            d.mkpath(_data_path);
        }
        if(!QDir(_data_path+sepe+pathname).exists()){
            QDir d(_data_path+sepe+pathname);
            d.mkpath(_data_path+sepe+pathname);
        }
        return _data_path+sepe+pathname+sepe;
    }


    void on_changeDownloadLocation_clicked();

    void on_downloadLocation_textChanged(const QString &arg1);

private:
    Ui::DownloadPage *ui;
    WaitingSpinnerWidget * _loader = nullptr;
    QSettings settings;

    QNetworkAccessManager * m_netwManager = nullptr;
    QString _cache_path;
    QList<QNetworkReply*>operations;
    QString __cfduid,PHPSESSID;


};

#endif // DOWNLOADPAGE_H
