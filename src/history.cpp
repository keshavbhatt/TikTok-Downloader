#include "history.h"
#include "ui_history.h"

#include <QMessageBox>

History::History(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::History)
{
    ui->setupUi(this);
    settings = new QSettings(utils::returnPath("history")+"records.db",QSettings::NativeFormat,this);
    loadHistory();
}

void History::refresh()
{
    ui->historyList->clear();
    loadHistory();
    ui->clearall->setEnabled(ui->historyList->count() == 0 ? false:true);
}

void History::setValue(QString key,QVariant value)
{
    settings->setValue(key,value);
}

QVariant History::value(QString key)
{
    return settings->value(key,"");
}

void History::remove(QString key)
{
    settings->remove(key);
}

void History::loadHistory()
{
    foreach (QString key,settings->allKeys()) {
        ui->historyList->addItem(settings->value(key).toString());
    }
}

History::~History()
{
    delete ui;
}

void History::on_clearall_clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Question");
    msgBox.setText("Clear History");
    msgBox.setInformativeText("Do you want to clear all history items?");
    msgBox.setStandardButtons(QMessageBox::Yes |QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.setStyleSheet("QLabel{min-width:280 px; }");

    int ret = msgBox.exec();
    if(ret == QMessageBox::Yes){
        foreach (QString key,settings->allKeys()) {
            remove(key);
        }
        refresh();
    }
}

void History::on_historyList_itemDoubleClicked(QListWidgetItem *item)
{
    emit loadFromHistoryUrls(item->text().trimmed());
}

void History::on_close_clicked()
{
    this->close();
}
