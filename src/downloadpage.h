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

#include <QCryptographicHash>

#include "widgets/waitingSpinner/waitingspinnerwidget.h"
#include "cookiejar.h"
#include "utils.h"
#include "history.h"
#include "account.h"

namespace Ui {
class DownloadPage;
}

class DownloadPage : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadPage(QWidget *parent = nullptr,History *history_obj = nullptr,account *account_obj = nullptr);
    ~DownloadPage();

public slots:
    void getResult(QUrl url);

private slots:
    void on_close_button_clicked();

    void init_networkManager();
    void cancelAllRequest();
    void requestError(QString errorStr);
    void parseDownloadOptions(QByteArray rep);
    void download(QString url, QString name);
    void downloadProgress(qint64 got, qint64 tot);
    void on_done_button_clicked();
    void on_changeDownloadLocation_clicked();
    void on_downloadLocation_textChanged(const QString &arg1);
    QString toolButtonStyle();
private:
    Ui::DownloadPage *ui;
    WaitingSpinnerWidget * _loader = nullptr;
    QSettings settings;

    QNetworkAccessManager * m_netwManager = nullptr;
    QString _cache_path;
    QList<QNetworkReply*>operations;
    QString __cfduid,PHPSESSID;
    History *history = nullptr;
    account *m_account = nullptr;


};

#endif // DOWNLOADPAGE_H
