#include "downloadpage.h"
#include "ui_downloadpage.h"

#include <QFileDialog>
#include <QToolButton>

DownloadPage::DownloadPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DownloadPage)
{
    ui->setupUi(this);

    _loader = new WaitingSpinnerWidget(this,true,true);
    _loader->setRoundness(70.0);
    _loader->setMinimumTrailOpacity(15.0);
    _loader->setTrailFadePercentage(70.0);
    _loader->setNumberOfLines(10);
    _loader->setLineLength(8);
    _loader->setLineWidth(2);
    _loader->setInnerRadius(2);
    _loader->setRevolutionsPerSecond(3);
    _loader->setColor(QColor("#1e90ff"));

    ui->progress_widget->hide();
    ui->results_widget->hide();

    QString path = settings.value("download_location",
                       QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)
                                      +"/"+QApplication::applicationName()).toString();
    QFileInfo pathInfo(path);
    if(pathInfo.exists()==false){
        QDir dir;
        if(dir.mkpath(path)){
            ui->downloadLocation->setText(path);
        }
    }else{
      ui->downloadLocation->setText(path);
    }

    init_networkManager();
}

DownloadPage::~DownloadPage()
{
    cancelAllRequest();
    if(m_netwManager!=nullptr)
        m_netwManager->deleteLater();
    delete ui;
}

void DownloadPage::on_close_button_clicked()
{
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    this->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
    a->setDuration(300);
    a->setStartValue(1);
    a->setEndValue(0);
    a->setEasingCurve(QEasingCurve::Linear);
    connect(a,&QPropertyAnimation::finished,[=](){
        this->close();
    });
    a->start(QPropertyAnimation::DeleteWhenStopped);
}

void DownloadPage::init_networkManager()
{
    _cache_path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QString cookieJarPath =  _cache_path+"/cookiejar_tkd.dat";

    m_netwManager = new QNetworkAccessManager(this);

    QNetworkDiskCache* diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory(_cache_path);
    m_netwManager->setCache(diskCache);
    m_netwManager->setCookieJar(new CookieJar(cookieJarPath,m_netwManager));
    connect(m_netwManager,&QNetworkAccessManager::finished,[=](QNetworkReply* rep){
        _loader->stop();
        operations.removeOne(rep);
        if(rep->error() != QNetworkReply::NoError){
            requestError("Unable to load: "+rep->request().url().toString()+"\n "+rep->errorString());
        }
    });
}

void DownloadPage::requestError(QString errorStr)
{
    QMessageBox::critical(this,QApplication::applicationName()+"| "+
                          tr("Request error"),errorStr);
}

void DownloadPage::getResult(QUrl url)
{
    if(m_netwManager!=nullptr)
    {
        bool preferCache = false;
        foreach (QNetworkCookie cookie, m_netwManager->cookieJar()->cookiesForUrl(QUrl("https://ttdownloader.com/"))) {
            if(cookie.isSessionCookie()==false && (cookie.expirationDate()<QDateTime::currentDateTime())){
                preferCache = false;
            }
        }
        QNetworkRequest request("http://ktechpit.com/USS/TTD/app/index.php?url="+url.toString());
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
           preferCache ? QNetworkRequest::PreferCache : QNetworkRequest::AlwaysNetwork);
        request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/84.0.4147.125 Safari/537.36");
        QNetworkReply *reply = m_netwManager->get(request);
        connect(reply,&QNetworkReply::finished,[=](){
           parseDownloadOptions(reply->readAll());
           reply->deleteLater();
        });
        operations.append(reply);
        _loader->start();
    }
}

void DownloadPage::parseDownloadOptions(QByteArray rep)
{
    QJsonDocument	jsonResponse	= QJsonDocument::fromJson(rep);
    if(jsonResponse.isNull() == true){
        QMessageBox::critical(this,QApplication::applicationName()+"| "+
                              tr("Invalid Response"),"Results parse error");
        return;
    }
    ui->results_widget->show();
    ui->progress_widget->hide();
    QJsonObject		jsonObj			= jsonResponse.object();
    QJsonArray		resultsJsonArray		= jsonObj["results"].toArray();
    QJsonArray		cookiesJsonArray		= jsonObj["cookies"].toArray();
    //add results
    for (int i = 0; i < resultsJsonArray.count(); ++i)
    {
        QJsonObject obj =  resultsJsonArray.at(i).toObject();
        QString  name =  obj.value("name").toString();
        QString  url  =  obj.value("url").toString();

        QToolButton *btn = new QToolButton(this);
        btn->setText(tr("Download with")+"\n"+name);
        btn->setIcon(QIcon(":/icons/download-line.png"));
        btn->setIconSize(QSize(24,24));
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        connect(btn,&QToolButton::clicked,[=](){
           download(url);
        });
        ui->download_result_widget->layout()->addWidget(btn);
    }
    //set cookies
    auto cookiejar = static_cast<CookieJar*>(m_netwManager->cookieJar());
    cookiejar->clearCookieJar();
    for (int i = 0; i < cookiesJsonArray.count(); ++i)
    {
        QJsonObject obj =  cookiesJsonArray.at(i).toObject();
        QNetworkCookie cookie = QNetworkCookie();
        cookie.setDomain(obj.value("domain").toString());
        cookie.setName(obj.value("name").toString().toUtf8());
        cookie.setExpirationDate(QDateTime::fromString(obj.value("expiration").toString()));
        cookie.setHttpOnly(obj.value("httponly").toBool());
        cookie.setPath(obj.value("path").toString());
        cookie.setSecure(obj.value("secure").toBool());
        cookie.setValue(obj.value("value").toString().toUtf8());
        m_netwManager->cookieJar()->insertCookie(cookie);
    }
}

void DownloadPage::download(QString url)
{
    ui->progress_label->setText("Loading...");
    if(m_netwManager!=nullptr)
    {
        QNetworkRequest request;
        request.setUrl(QUrl(url));
        request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/84.0.4147.125 Safari/537.36");

        QNetworkReply *reply = m_netwManager->get(request);
        ui->progressBar->setRange(0,0);
        ui->progressBar->setValue(-1);
        ui->progress_widget->show();
        ui->results_widget->hide();
        ui->done_button->setEnabled(false);
        connect(reply,&QNetworkReply::finished,[=]()
        {
            if(reply->error() == QNetworkReply::NoError){
                //qDebug()<<reply->request().rawHeader("Cookie");
                QString disposition = reply->rawHeader("Content-Disposition");
                if(disposition.isEmpty()==false){
                    QString fileName = disposition.split("filename=").last();
                    if(fileName.trimmed().isEmpty()){
                        //TODO ask user for filename
                    }
                    auto location = settings.value("download_location",
                                                   QStandardPaths::writableLocation(
                                                   QStandardPaths::DownloadLocation)
                                                   +QApplication::applicationName()).toString();

                    QFileInfo pathInfo(location);
                    if(pathInfo.exists()==false){
                        QDir dir;
                        dir.mkpath(location);
                    }
                    // Save the file here
                    QFile file(location+QDir::separator()+fileName);
                    file.open(QIODevice::WriteOnly);
                    file.write(reply->readAll());
                    file.close();
                }
                ui->progressBar->setRange(0,100);
                ui->progressBar->setValue(100);
                ui->done_button->setEnabled(true);
            }else{
                qDebug()<<reply->error();
            }
            //delete reply
            reply->deleteLater();
        });
        connect(reply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(downloadProgress(qint64,qint64)));
        operations.append(reply);
    }
}

void DownloadPage::downloadProgress(qint64 got,qint64 tot)
{
    ui->progress_widget->show();
    ui->results_widget->hide();

    double downloaded_Size = (double)got;
    double total_Size = (double)tot;
    double progress = (downloaded_Size/total_Size) * 100;
    int intProgress = static_cast<int>(progress);
    //qDebug()<<"DOWNLOAD Progress:"<<intProgress;
    if(intProgress == 100){
        ui->progress_label->setText("Downloaded "+humanReadableSize(downloaded_Size)
                                    +" of "+humanReadableSize(downloaded_Size));
    }else if(intProgress < 100){
        ui->progress_label->setText("Downloaded "+humanReadableSize(downloaded_Size)
                                    +" of Unknown Size");
    }else{
        ui->progressBar->setValue(intProgress);
    }
}


void DownloadPage::cancelAllRequest()
{
    foreach (QNetworkReply *rep,operations) {
        if(rep!=nullptr){
            rep->abort();
            rep->deleteLater();
        }
    }
}


void DownloadPage::on_done_button_clicked()
{
    ui->progress_widget->hide();
    ui->results_widget->show();
    ui->done_button->setEnabled(false);
}

void DownloadPage::on_changeDownloadLocation_clicked()
{
    QString path = settings.value("download_location",
                      QStandardPaths::writableLocation(QStandardPaths::DownloadLocation))
                      .toString()+"/"+QApplication::applicationName();

       QString destination = QFileDialog::getExistingDirectory(this, tr("Select destination directory"), path,QFileDialog::ShowDirsOnly);
       QFileInfo dir(destination);
       if(dir.isWritable()){
           ui->downloadLocation->setText(destination);
       }else{
           QMessageBox::critical(this,"Error","Destination directory is not writable.<br>Please choose another.");
       }
}

void DownloadPage::on_downloadLocation_textChanged(const QString &arg1)
{
    if(arg1.isEmpty()==false){
            settings.setValue("download_location",arg1);
    }
}
