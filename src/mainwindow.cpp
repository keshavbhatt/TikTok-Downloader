#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopServices>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QApplication::applicationName());
    setWindowIcon(QIcon(":/icons/app/icon-128.png"));

    //theme settings mapped to values of indexes of themeCombo widget in SettingsWidget class
    int theme = settings.value("themeCombo",1).toInt();
    setStyle(":/qbreeze/"+QString(theme == 1 ? "dark" : "light") +".qss");

    //geometry and state load
    if(settings.value("mainwindow_geometry").isValid()){
        restoreGeometry(settings.value("mainwindow_geometry").toByteArray());
    }else{
        //this->resize(ui->mainToolBar->sizeHint().width(),500);
        this->setWindowState(Qt::WindowMaximized);
    }
    if(settings.value("mainwindow_windowState").isValid()){
        restoreState(settings.value("mainwindow_windowState").toByteArray());
    }else{
        QScreen* pScreen = QApplication::primaryScreen();
        QRect availableScreenSize = pScreen->availableGeometry();
        this->move(availableScreenSize.center()-this->rect().center());
    }

    ui->url_lineEdit->addAction(QIcon(":/icons/links-line.png"),QLineEdit::LeadingPosition);
    ui->url_lineEdit->setText(settings.value("lastUrl","").toString());

    ui->download_button->setEnabled(isValidUrl(ui->url_lineEdit->text().trimmed()));

    //init history
    history = new History(this);
    history->setWindowTitle(QApplication::applicationName()+" |"+tr(" History"));
    history->setWindowModality(Qt::ApplicationModal);
    history->setWindowFlags(Qt::Dialog);
    connect(history,&History::loadFromHistoryUrls,[=](QString url)
    {
        ui->url_lineEdit->setText(url);
        history->close();
    });
    ui->unlock_toolButton->setStyleSheet(toolButtonStyle());
    ui->aboutButton->setStyleSheet(toolButtonStyle()+";border:none;");
    init_account();
}

void MainWindow::setStyle(QString fname)
{
    QFile styleSheet(fname);
    if (!styleSheet.open(QIODevice::ReadOnly))
    {
        qWarning("Unable to open file");
        return;
    }
    qApp->setStyleSheet(styleSheet.readAll());
    styleSheet.close();
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette palette;
    palette.setColor(QPalette::Window,QColor(53,53,53));
    palette.setColor(QPalette::WindowText,Qt::white);
    palette.setColor(QPalette::Disabled,QPalette::WindowText,QColor(127,127,127));
    palette.setColor(QPalette::Base,QColor(42,42,42));
    palette.setColor(QPalette::AlternateBase,QColor(66,66,66));
    palette.setColor(QPalette::ToolTipBase,Qt::white);
    palette.setColor(QPalette::ToolTipText,QColor(53,53,53));
    palette.setColor(QPalette::Text,Qt::white);
    palette.setColor(QPalette::Disabled,QPalette::Text,QColor(127,127,127));
    palette.setColor(QPalette::Dark,QColor(35,35,35));
    palette.setColor(QPalette::Shadow,QColor(20,20,20));
    palette.setColor(QPalette::Button,QColor(53,53,53));
    palette.setColor(QPalette::ButtonText,Qt::white);
    palette.setColor(QPalette::Disabled,QPalette::ButtonText,QColor(127,127,127));
    palette.setColor(QPalette::BrightText,Qt::red);
    palette.setColor(QPalette::Link,QColor("#3DAEE9"));
    palette.setColor(QPalette::Highlight,QColor(49,106,150));
    palette.setColor(QPalette::Disabled,QPalette::Highlight,QColor(80,80,80));
    palette.setColor(QPalette::HighlightedText,Qt::white);
    palette.setColor(QPalette::Disabled,QPalette::HighlightedText,QColor(127,127,127));
    qApp->setPalette(palette);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    settings.setValue("mainwindow_geometry",saveGeometry());
    settings.setValue("mainwindow_windowState", saveState());
    QWidget::closeEvent(event);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_help_button_clicked()
{
    Help *help_widget = new Help(this);
    help_widget->setWindowTitle(QApplication::applicationName()+" |"+tr(" Help"));
    help_widget->setWindowModality(Qt::ApplicationModal);
    help_widget->setWindowFlags(Qt::Dialog);
    help_widget->setAttribute(Qt::WA_DeleteOnClose);
    connect(help_widget,&Help::loadFromExampleUrls,[=](QString url){
       ui->url_lineEdit->setText(url);
       help_widget->close();
    });
    help_widget->show();
}

void MainWindow::on_url_lineEdit_textChanged(const QString &arg1)
{
    if(isValidUrl(arg1)){
        ui->download_button->setEnabled(true);
        settings.setValue("lastUrl",arg1);
    }else{
        ui->download_button->setEnabled(false);
    }
}

bool MainWindow::isValidUrl(QString arg1)
{
    if(arg1.contains("tiktok") && arg1.trimmed().isEmpty()==false)
    {
        return true;
    }else{
        return false;
    }
}

void MainWindow::on_download_button_clicked()
{
    DownloadPage *m_details = new DownloadPage(this,history,accountWidget);
    eff =  new QGraphicsOpacityEffect(m_details);

    m_details->setAttribute(Qt::WA_DeleteOnClose);
    m_details->setWindowFlags(Qt::Widget);
    m_details->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    m_details->setGeometry(this->rect());

    if(eff!=nullptr){
        m_details->setGraphicsEffect(eff);
        QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
        a->setDuration(250);
        a->setStartValue(0);
        a->setEndValue(1);
        a->setEasingCurve(QEasingCurve::Linear);
        connect(a,&QPropertyAnimation::finished,[=](){
           eff->deleteLater();
        });
        a->start(QPropertyAnimation::DeleteWhenStopped);
        //make request
        m_details->getResult(QUrl(ui->url_lineEdit->text().trimmed()));
        m_details->show();
    }
}

QString MainWindow::toolButtonStyle()
{
    QColor rgb = QColor("#266A85");
    QString r = QString::number(rgb.red());
    QString g = QString::number(rgb.green());
    QString b = QString::number(rgb.blue());

    QString widgetStyle= "border-radius:4px;background-color:"
                         "qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,"
                         "stop:0.129213 rgba("+r+", "+g+", "+b+", 40),"
                         "stop:0.38764 rgba("+r+", "+g+", "+b+", 136),"
                         "stop:0.679775 rgba("+r+", "+g+", "+b+", 94),"
                         "stop:1 rgba("+r+", "+g+", "+b+", 30));";
    return widgetStyle;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    DownloadPage *m_details = this->findChild<DownloadPage*>();
    if(m_details){
        m_details->setGeometry(this->rect());
    }
    QWidget::resizeEvent(event);
}


void MainWindow::on_history_button_clicked()
{
    history->refresh();
    history->show();
}

void MainWindow::init_account()
{
    //work around to fix recieve created_signal from account class
    QPushButton *pb = new QPushButton("pop",this);
    pb->setObjectName("push");
    pb->hide();
    connect(pb,&QPushButton::clicked,[=](){
       qDebug()<<"PUSH BUTTON clicked";
       QTimer::singleShot(2000,[=](){
           if(accountWidget!=nullptr){
               if(!accountWidget->isVisible()){
                   accountWidget->show();
               }
           }
       });
    });
    if(accountWidget==nullptr)
    {
        accountWidget = new account(this);
        accountWidget->setObjectName("accountWidget");
        accountWidget->setWindowFlags(Qt::Dialog);
        accountWidget->adjustSize();
        connect(accountWidget,&account::showAccountWidget,[=](){
           accountWidget->show();
           accountWidget->flashPurchaseButton();
        });
        //signal based check to enable pro on live check
        connect(accountWidget,&account::disablePro,[=](){
           disablePro();
        });
        connect(accountWidget,&account::enablePro,[=](){
           enablePro();
        });
        //manual check to enable pro if not conencted
        accountWidget->check_pro();
        if(accountWidget->pro == true){
            enablePro();
        }else{
            disablePro();
        }
    }
}

void MainWindow::enablePro()
{
    ui->unlock_toolButton->setIcon(QIcon(":/icons/account-pin-circle-line.png"));
    ui->unlock_toolButton->setText("Your\nAccount");

    ui->f1->setPixmap(QPixmap(":/icons/lock-unlock-line.png"));
    ui->f2->setPixmap(QPixmap(":/icons/lock-unlock-line.png"));
    ui->f3->setPixmap(QPixmap(":/icons/lock-unlock-line.png"));

}

void MainWindow::disablePro()
{
    ui->unlock_toolButton->setIcon(QIcon(":/icons/lock-unlock-line.png"));
    ui->unlock_toolButton->setText("Unclock all\nfeatures now");

    ui->f1->setPixmap(QPixmap(":/icons/lock-line.png"));
    ui->f2->setPixmap(QPixmap(":/icons/lock-line.png"));
    ui->f3->setPixmap(QPixmap(":/icons/lock-line.png"));
}

void MainWindow::on_unlock_toolButton_clicked()
{
    accountWidget->show();
}

void MainWindow::on_aboutButton_clicked()
{
    QDialog *aboutDialog = new QDialog(this,Qt::Dialog);
    aboutDialog->setWindowModality(Qt::WindowModal);
    QVBoxLayout *layout = new QVBoxLayout;
    QLabel *message = new QLabel(aboutDialog);
    layout->addWidget(message);
    connect(message,&QLabel::linkActivated,[=](const QString linkStr){
        if(linkStr.contains("about_qt")){
            qApp->aboutQt();
        }else{
            QDesktopServices::openUrl(QUrl(linkStr));
        }
    });
    aboutDialog->setLayout(layout);
    aboutDialog->setAttribute(Qt::WA_DeleteOnClose,true);
    aboutDialog->show();

    QString mes =
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><img src=':/icons/app/icon-64.png' /></p>"
                 "<p align='center' style='-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><br /></p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Designed and Developed</p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>by <span style=' font-weight:600;'>Keshav Bhatt</span> &lt;keshavnrj@gmail.com&gt;</p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Website: https://ktechpit.com</p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Runtime: <a href='http://about_qt'>Qt Toolkit</a></p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Version: "+QApplication::applicationVersion()+"</p>"
                 "<p align='center' style='-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><br /></p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><a href='https://snapcraft.io/search?q=keshavnrj'>More Apps</p>"
                 "<p align='center' style='-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><br /></p>";

    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    message->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
    a->setDuration(1000);
    a->setStartValue(0);
    a->setEndValue(1);
    a->setEasingCurve(QEasingCurve::InCurve);
    a->start(QPropertyAnimation::DeleteWhenStopped);
    message->setText(mes);
    message->show();
}
