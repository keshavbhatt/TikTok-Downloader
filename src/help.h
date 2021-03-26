#ifndef HELP_H
#define HELP_H

#include <QWidget>
#include <QSettings>

namespace Ui {
class Help;
}

class Help : public QWidget
{
    Q_OBJECT

signals:
    void loadFromExampleUrls(QString url);

public:
    explicit Help(QWidget *parent = nullptr);
    ~Help();

private slots:
    void on_listView_doubleClicked(const QModelIndex &index);

    void on_tabWidget_currentChanged(int index);

private:
    Ui::Help *ui;
    QSettings settings;
};

#endif // HELP_H
