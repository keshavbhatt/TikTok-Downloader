#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStyleFactory>
#include <QScreen>
#include <QSettings>
#include <QStandardPaths>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#include "downloadpage.h"
#include "help.h"
#include "history.h"
#include "account.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected slots:
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);
private slots:
    void setStyle(QString fname);
    void on_help_button_clicked();

    void on_url_lineEdit_textChanged(const QString &arg1);

    bool isValidUrl(QString arg1);
    void on_download_button_clicked();


    void on_history_button_clicked();

    QString toolButtonStyle();
    void on_unlock_toolButton_clicked();

    void init_account();

    void disablePro();
    void enablePro();
    void on_aboutButton_clicked();

private:
    Ui::MainWindow *ui;
    QSettings settings;
    QGraphicsOpacityEffect *eff = nullptr;
    History *history = nullptr;
    account *accountWidget = nullptr;

};

#endif // MAINWINDOW_H
