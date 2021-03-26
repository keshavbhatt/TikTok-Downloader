#ifndef HISTORY_H
#define HISTORY_H

#include <QWidget>
#include <QSettings>
#include <QListWidgetItem>

#include "utils.h"

namespace Ui {
class History;
}

class History : public QWidget
{
    Q_OBJECT

signals:
    void loadFromHistoryUrls(QString url);

public:
    explicit History(QWidget *parent = nullptr);
    ~History();
public slots:
    void setValue(QString key, QVariant value);
    QVariant value(QString key);
    void remove(QString key);
    void loadHistory();
    void refresh();
private slots:
    void on_clearall_clicked();

    void on_historyList_itemDoubleClicked(QListWidgetItem *item);

    void on_close_clicked();

private:
    Ui::History *ui;
    QSettings *settings = nullptr;
};

#endif // HISTORY_H
