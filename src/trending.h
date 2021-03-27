#ifndef TRENDING_H
#define TRENDING_H

#include <QLabel>
#include <QWidget>
#include <QtNetwork>
#include "widgets/waitingSpinner/waitingspinnerwidget.h"
#include "cookiejar.h"
#include "gridlayoututil.h"
#include "utils.h"
#include <QSettings>
#include "ui_trending_option.h"

namespace Ui {
class Trending;
}

class Trending : public QWidget
{
    Q_OBJECT

public:
    explicit Trending(QWidget *parent = nullptr);
    ~Trending();

signals:
    void loadFromTrendingUrls(QString url);

protected slots:
    bool eventFilter(QObject *obj, QEvent *event);
    void closeEvent(QCloseEvent *event);
private slots:
    void on_show_button_clicked();

    void init_networkManager();
    void cancelAllRequest();
    void parseVideos(const QByteArray rep);
    void requestError(QString errorStr);
    void getResult(QString countryCode);
    void clearResultWidget();
    void getImage(QMovie *movie, QUrl url);
    void on_country_combobox_currentIndexChanged(int index);

    void show_option_for_downloaded(QObject *obj);
    void hide_option_for_downloaded(QObject *obj);
private:
    Ui::Trending *ui;
    Ui::trending_option trending_ui;
    WaitingSpinnerWidget *_loader = nullptr;
    QNetworkAccessManager  *m_netwManager = nullptr;

    QString _cache_path;
    QList<QNetworkReply*>operations;

    int rows,cols;
    QSettings settings;
};

#endif // TRENDING_H
