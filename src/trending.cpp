#include "trending.h"
#include "ui_trending.h"
#include <QMessageBox>
#include <QMovie>
#include <QStandardPaths>
#include <QDebug>
#include <QDir>
#include <QHoverEvent>

Trending::Trending(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Trending)
{
    ui->setupUi(this);

    if(settings.value("trending_geometry").isValid()){
        this->restoreGeometry(settings.value("trending_geometry").toByteArray());
    }

    _loader = new WaitingSpinnerWidget(ui->results_listWidget,true,true);
    _loader->setRoundness(70.0);
    _loader->setMinimumTrailOpacity(15.0);
    _loader->setTrailFadePercentage(70.0);
    _loader->setNumberOfLines(10);
    _loader->setLineLength(8);
    _loader->setLineWidth(2);
    _loader->setInnerRadius(2);
    _loader->setRevolutionsPerSecond(3);
    _loader->setColor(QColor("#1e90ff"));

    ui->show_button->hide();
    ui->country_combobox->blockSignals(true);

    ui->country_combobox->addItem("USA","us");
    ui->country_combobox->addItem("France","fr");
    ui->country_combobox->addItem("Germany","de");

    ui->country_combobox->blockSignals(false);
    ui->country_combobox->setCurrentIndex(settings.value("trending_country_index",0).toInt());

    init_networkManager();

    ui->show_button->click();
}

Trending::~Trending()
{
    cancelAllRequest();
    if(m_netwManager!=nullptr)
        m_netwManager->deleteLater();
    clearResultWidget();
    delete ui;
}

void Trending::on_show_button_clicked()
{
    getResult(ui->country_combobox->currentData(Qt::UserRole).toString());
}

void Trending::init_networkManager()
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

void Trending::getResult(QString countryCode)
{
    if(m_netwManager!=nullptr)
    {
        qDebug()<<countryCode;
        QNetworkRequest request(QUrl("https://influencermarketinghub.com/wp-admin/admin-ajax.php"));
        request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/84.0.4147.125 Safari/537.36");
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

        //QString("action=load_popular_tt_ttrands&country="+countryCode).toUtf8()

        QUrlQuery query;
        query.addQueryItem("action","load_popular_tt_ttrands");
        query.addQueryItem("country",countryCode);

        QNetworkReply *reply = m_netwManager->post(request,query.toString().toUtf8());
        connect(reply,&QNetworkReply::finished,[=](){
            if(reply->error()==QNetworkReply::NoError){
                //parse download options
                parseVideos(reply->readAll());
            }else{
                if(reply->error() == QNetworkReply::InternalServerError
                        ||reply->error() == QNetworkReply::ContentNotFoundError){
                    requestError("Invalid video URL\n Server responded: "+reply->readAll());
                }else{
                    requestError(reply->errorString().replace("ktechpit.com/USS","onion://ttd:8880")+"\n Server responded: "+reply->readAll());
                }
            }
           reply->deleteLater();
        });
        operations.append(reply);
        _loader->start();
    }
}


void Trending::requestError(QString errorStr)
{
    errorStr.replace("ttdownloader.com","onion://ttd:8880");
    errorStr.replace("ktechpit.com","onion://ttd:8880");
    QMessageBox::critical(this,QApplication::applicationName()+"| "+
                          tr("Request error"),errorStr);
}

void Trending::getImage(QMovie *movie,QUrl url)
{
    if(m_netwManager!=nullptr)
    {
        QString tmpPath  = utils::returnPath("trending_tmp");
        QNetworkRequest request(url);
        QNetworkReply *reply = m_netwManager->get(request);
        connect(reply,&QNetworkReply::finished,[=](){
            if(reply->error()==QNetworkReply::NoError){
                QString fname = tmpPath+utils::generateRandomId(20);
                QFile tf(fname);
                if(tf.open(QIODevice::ReadWrite)){
                    tf.write(reply->readAll());
                }
                tf.close();
                movie->setFileName(tf.fileName());
                if(movie->isValid())
                    movie->start();
            }else{
                if(reply->error() == QNetworkReply::InternalServerError
                        ||reply->error() == QNetworkReply::ContentNotFoundError){
                    requestError("Invalid video URL\n Server responded: "+reply->readAll());
                }else{
                    requestError(reply->errorString().replace("ktechpit.com/USS","onion://ttd:8880")+"\n Server responded: "+reply->readAll());
                }
            }
           reply->deleteLater();
        });
        operations.append(reply);
    }
}

void Trending::parseVideos(const QByteArray rep)
{
    if(rep.contains("error")){
        QMessageBox::critical(this,QApplication::applicationName()+"| "+
                              tr("Request failed"),QString("There was an issue with the request.\n\nServer Responded: "+rep));
        return;
    }
    QJsonDocument	jsonResponse	= QJsonDocument::fromJson(rep);
    if(jsonResponse.isNull() == true){
        QMessageBox::critical(this,QApplication::applicationName()+"| "+
                              tr("Invalid Response"),"Results parse error");
        return;
    }

    clearResultWidget();
    QJsonArray jsonArray = jsonResponse.array();
    cols = 3;
    rows = jsonArray.count()/cols;
    foreach (const QJsonValue &val, jsonArray)
    {
       QJsonObject object = val.toObject();
       QUrl cover(object.value("covers").toObject().value("dynamic").toString());
       QString videoLink = object.value("webVideoUrl").toString();
       //add to listwidget or grid
       QLabel *label = new QLabel;
       label->setAlignment(Qt::AlignCenter);
       label->setFixedSize(211,300);
       label->setObjectName("video_link-"+videoLink);
       QMovie *movie = new QMovie(label);
       label->setMovie(movie);
       getImage(movie,cover);
       int row = ui->gridLayout->count()/cols;
       int col = ui->gridLayout->count() % cols;
       ui->gridLayout->addWidget(label,row,col);
       ui->gridLayout->itemAtPosition(row,col)->widget()->installEventFilter(this);
   }
}

bool Trending::eventFilter(QObject *obj, QEvent *event)
{
    if(obj->objectName().contains("video_link-")){
         const QHoverEvent* const he = static_cast<const QHoverEvent*>( event );
         switch(he->type())
             {
             case QEvent::HoverEnter:
                 show_option_for_downloaded(obj);
                 break;
             case QEvent::HoverLeave:
                 hide_option_for_downloaded(obj);

                 break;
             default:
                 break;
             }
    }
    return QWidget::eventFilter(obj,event);
}

void Trending::show_option_for_downloaded(QObject *obj)
{
    QWidget *objWidget = qobject_cast<QWidget*>(obj);
    QWidget *option_widget = obj->findChild<QWidget*>("option_"+obj->objectName());
     if(option_widget==nullptr){
        QWidget *option = new QWidget(objWidget);
        option->setStyleSheet("background:transparent");
        option->setObjectName("option_"+obj->objectName());
        trending_ui.setupUi(option);
        connect(trending_ui.download_button,&QPushButton::clicked,[=](){
            QString videoLink = obj->objectName().split("video_link-").last();
            emit loadFromTrendingUrls(videoLink);
        });
        option->setGeometry(objWidget->rect());
        option->show();
     }else{
         option_widget->show();
     }
}
void Trending::hide_option_for_downloaded(QObject *obj)
{
    QWidget *option_widget = obj->findChild<QWidget*>("option_"+obj->objectName());
    if(option_widget!=nullptr){
        option_widget->hide();
        option_widget->deleteLater();
    }
}


void Trending::clearResultWidget()
{
    //clear grid
    for (int i = 0; i < ui->gridLayout->columnCount(); i++)
    {
        GridLayoutUtil::removeColumn(ui->gridLayout,i,true);
    }
    //clear tmp image Path
    QString tmpPath  = utils::returnPath("trending_tmp");
    utils util;
    util.delete_cache(tmpPath);

}

void Trending::cancelAllRequest()
{
    foreach (QNetworkReply *rep,operations) {
        if(rep!=nullptr){
            rep->abort();
            rep->deleteLater();
        }
    }
}

void Trending::on_country_combobox_currentIndexChanged(int index)
{
    settings.setValue("trending_country_index",index);
    on_show_button_clicked();
}

void Trending::closeEvent(QCloseEvent *event)
{
    settings.setValue("trending_geometry",saveGeometry());
    //settings.setValue("trending_windowState", this->windowState());
    QWidget::closeEvent(event);
}
