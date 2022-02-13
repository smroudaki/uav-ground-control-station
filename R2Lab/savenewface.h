#ifndef SAVENEWFACE_H
#define SAVENEWFACE_H


#include "dialog.h"

#include <QDialog>

#include <iostream>

using namespace std;

namespace Ui {
class SaveNewFace;
}

class SaveNewFace : public QDialog
{
    Q_OBJECT

public:
    explicit SaveNewFace(QWidget *parent = 0);
    ~SaveNewFace();

    vector<string> labels;

    bool is_exist_in_database(QString);

private slots:
    void on_btnSave_clicked();

signals:
    void notifyEnteredTextSent(const QString&);

private:
    Ui::SaveNewFace *ui_savenewface;
};

#endif // SAVENEWFACE_H
