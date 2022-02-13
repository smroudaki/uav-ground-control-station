#include "savenewface.h"
#include "ui_savenewface.h"


SaveNewFace::SaveNewFace(QWidget *parent) :
    QDialog(parent),
    ui_savenewface(new Ui::SaveNewFace)
{
    ui_savenewface->setupUi(this);

    ui_savenewface->txtImageName->clear();
    ui_savenewface->txtImageName->setFocus();
}

SaveNewFace::~SaveNewFace()
{
    delete ui_savenewface;
}

bool SaveNewFace::is_exist_in_database(QString name)
{
    for (size_t i = 0; i < labels.size(); i++)
        if (name.toStdString() == labels[i])
            return true;

    return false;
}

void SaveNewFace::on_btnSave_clicked()
{
    Dialog dialog;
    labels = dialog.getLabels;

    if (!is_exist_in_database(ui_savenewface->txtImageName->text()))
    {
        emit notifyEnteredTextSent(ui_savenewface->txtImageName->text());
        this->close();
    }
    else
    {
        cerr << "Entered name exists in database! Enter another name please." << endl;
        ui_savenewface->txtImageName->clear();
        ui_savenewface->txtImageName->setFocus();
    }
}
