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
    DownloadPage *m_details = new DownloadPage(this);
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

void MainWindow::resizeEvent(QResizeEvent *event)
{
    DownloadPage *m_details = this->findChild<DownloadPage*>();
    if(m_details){
        m_details->setGeometry(this->rect());
    }
    QWidget::resizeEvent(event);
}

