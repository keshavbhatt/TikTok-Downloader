#include "downloadpage.h"
#include "ui_downloadpage.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QToolButton>


DownloadPage::DownloadPage(QWidget *parent, History *history_obj, account *account_obj) :
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

    //init history
    history = history_obj;
    m_account = account_obj;

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
    ui->play->disconnect();
    ui->open_location->disconnect();

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
    });
}

void DownloadPage::requestError(QString errorStr)
{
    errorStr.replace("ttdownloader.com","onion://ttd:8880");
    errorStr.replace("ktechpit.com","onion://ttd:8880");
    QMessageBox::critical(this,QApplication::applicationName()+"| "+
                          tr("Request error"),errorStr);
    on_close_button_clicked();
}

void DownloadPage::getResult(QUrl url)
{
    if(m_netwManager!=nullptr)
    {
        QNetworkRequest request("http://ktechpit.com/USS/TTD/app/index.php?url="+url.toString());
        request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/84.0.4147.125 Safari/537.36");
        QNetworkReply *reply = m_netwManager->get(request);
        connect(reply,&QNetworkReply::finished,[=](){
            if(reply->error()==QNetworkReply::NoError){
                //add url to history
                history->setValue(QCryptographicHash::hash(url.toString().toUtf8(),QCryptographicHash::Md5),url.toString());
                //parse download options
                parseDownloadOptions(reply->readAll());
            }else{
                if(reply->error() == QNetworkReply::InternalServerError
                        ||reply->error() == QNetworkReply::ContentNotFoundError){
                    requestError("Invalid video URL\n\nServer Responded: "+reply->readAll());
                }else{
                    requestError(reply->errorString().replace("ktechpit.com/USS","onion://ttd:8880")+"\n\nServer Responded: "+reply->readAll());
                }
            }
           reply->deleteLater();
        });
        operations.append(reply);
        _loader->start();
    }
}

QString DownloadPage::toolButtonStyle()
{
    QColor rgb = QColor("#266A85");
    QString r = QString::number(rgb.red());
    QString g = QString::number(rgb.green());
    QString b = QString::number(rgb.blue());

    QString widgetStyle= "border-radius:2px;background-color:"
                         "qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,"
                         "stop:0.129213 rgba("+r+", "+g+", "+b+", 40),"
                         "stop:0.38764 rgba("+r+", "+g+", "+b+", 136),"
                         "stop:0.679775 rgba("+r+", "+g+", "+b+", 94),"
                         "stop:1 rgba("+r+", "+g+", "+b+", 30));";
    return widgetStyle;
}

void DownloadPage::parseDownloadOptions(QByteArray rep)
{
    m_account->check_pro(true);

    if(rep.contains("error")){
        QMessageBox::critical(this,QApplication::applicationName()+"| "+
                              tr("Download failed"),QString("There was an issue with the TikTok link you submitted. Please try again.\n\nServer Responded: "+rep));
        on_close_button_clicked();
        return;
    }
    QJsonDocument	jsonResponse	= QJsonDocument::fromJson(rep);
    if(jsonResponse.isNull() == true){
        QMessageBox::critical(this,QApplication::applicationName()+"| "+
                              tr("Invalid Response"),"Results parse error");
        on_close_button_clicked();
        return;
    }
    ui->results_widget->show();
    ui->progress_widget->hide();
    QJsonObject		jsonObj                 = jsonResponse.object();
    QJsonArray		resultsJsonArray		= jsonObj["results"].toArray();
    QJsonArray		cookiesJsonArray		= jsonObj["cookies"].toArray();
    //add results
    for (int i = 0; i < resultsJsonArray.count(); ++i)
    {
        QJsonObject obj =  resultsJsonArray.at(i).toObject();
        QString  name =  obj.value("name").toString();
        QString  url  =  obj.value("url").toString();

        QToolButton *btn = new QToolButton(this);
        btn->setStyleSheet(toolButtonStyle());

        if(name.contains("audio",Qt::CaseInsensitive)){
            btn->setText(tr("Download ")+"\n"+name);
            btn->setIcon(QIcon(":/icons/music-2-line.png"));
        }else{
            if(name.contains("no watermark",Qt::CaseInsensitive)){
                name = "without Watermark";
                btn->setText(tr("Download video \n")+name);
            }else{
                btn->setText(tr("Download video with\n")+name);
            }
            btn->setIcon(QIcon(":/icons/movie-2-line.png"));
        }
        btn->setIconSize(QSize(24,24));
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);


        if(m_account->evaluation_used == true && m_account->pro == false)
        {
            connect(btn,&QToolButton::clicked,[=](){
               m_account->showPurchaseMessage();
            });
        }else{
            connect(btn,&QToolButton::clicked,[=](){
               download(url,name);
            });
        }
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

void DownloadPage::download(QString url,QString name)
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
                    fileName.replace("\"",""); //remove " from filename
                    //append no_watermark to filename of no watermark video
                    if(name.contains("without")){
                       QString fname = QString(fileName.split(".").first()).append("_no_watermark");
                       QString ext   = QString(fileName.split(".").last());
                       fileName      = fname+"."+ext;
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
                    qint64 bytesWritten = file.write(reply->readAll());
                    file.close();
                    if(bytesWritten != -1 || bytesWritten != 0 || bytesWritten > -1){
                        ui->open_location->setEnabled(true);
                        ui->play->setEnabled(true);
                        connect(ui->play,&QPushButton::clicked,[=]{
                           bool opened = QDesktopServices::openUrl(QUrl(location+QDir::separator()+fileName));
                           if(!opened){
                               QMessageBox::critical(this,QApplication::applicationName()+"| "+
                                                     tr("Process error"),"Unable to open with sensible Media Player");
                           }
                        });
                        connect(ui->open_location,&QPushButton::clicked,[=]{
                           bool opened =  QDesktopServices::openUrl(QUrl(location));
                           if(!opened){
                               QMessageBox::critical(this,QApplication::applicationName()+"| "+
                                                     tr("Process error"),"Unable to open with sensible File Manager");
                           }
                        });
                    }else{
                        QMessageBox::critical(this,QApplication::applicationName()+"| "+
                                              tr("Download failed"),"Please try again.");
                        on_close_button_clicked();
                    }
                }else{
                    QMessageBox::critical(this,QApplication::applicationName()+"| "+
                                          tr("Unable to download"),"Download file missing filename.");
                    on_close_button_clicked();
                }
                ui->progressBar->setRange(0,100);
                ui->progressBar->setValue(100);
                ui->done_button->setEnabled(true);
            }else{
                requestError(reply->errorString()+"\n\nServer Responded: "+reply->readAll());
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
        ui->progress_label->setText("Downloaded "+utils::humanReadableSize(downloaded_Size)
                                    +" of "+utils::humanReadableSize(downloaded_Size));
    }else if(intProgress < 100){
        ui->progress_label->setText("Downloaded "+utils::humanReadableSize(downloaded_Size)
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
    ui->open_location->setEnabled(false);
    ui->play->setEnabled(false);

    ui->play->disconnect();
    ui->open_location->disconnect();

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
