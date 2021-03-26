#include "help.h"
#include "ui_help.h"

#include <QStandardItemModel>
#include <QDebug>

Help::Help(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Help)
{
    ui->setupUi(this);
    QStringList exampleUrls;
    exampleUrls   <<"https://www.tiktok.com/@jacquelinef_143/video/6795515905272040706"
                  <<"https://www.tiktok.com/@lip.sync.queen/video/6793852017136667910"
                  <<"https://www.tiktok.com/@celinedept/video/6787046505741176069"
                  <<"https://www.tiktok.com/@brentrivera/video/6765579840998378758"
                  <<"https://www.tiktok.com/@theshilpashetty/video/6780539812735732998"
                  <<"https://www.tiktok.com/@mridulmadhok/video/6797360482757070081"
                  <<"https://www.tiktok.com/@lovesoreal/video/6788990914279771398"
                  <<"https://www.tiktok.com/@kajalvp/video/6788094467481046274"
                  <<"https://www.tiktok.com/@thunthunskittles/video/6659414339272838405"
                  <<"https://www.tiktok.com/@awezdarbar/video/6783195616752209154"
                  <<"https://www.tiktok.com/@sampepper/video/6758193960218316037"
                  <<"https://www.tiktok.com/@djbravo47/video/6766440692777536770"
                  <<"https://www.tiktok.com/@samuelgrubbs/video/6760793690362400006"
                  <<"https://www.tiktok.com/@pj_3132/video/6751795112520600834";

    QStandardItemModel *model = new QStandardItemModel(this);
    foreach (QString str, exampleUrls) {
        model->appendRow(new QStandardItem(str));
    }
    ui->listView->setModel(model);

    if(settings.value("currentHelpTab").isValid()){
        ui->tabWidget->setCurrentIndex(settings.value("currentHelpTab",0).toInt());
    }
}

Help::~Help()
{
    delete ui;
}

void Help::on_listView_doubleClicked(const QModelIndex &index)
{
    auto url = ui->listView->model()->data(index,Qt::DisplayRole).toString();
    emit loadFromExampleUrls(url);
}

void Help::on_tabWidget_currentChanged(int index)
{
    settings.setValue("currentHelpTab",index);
}
