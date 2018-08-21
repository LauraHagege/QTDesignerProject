#include "dialogpinaccess.h"
#include "ui_dialogpinaccess.h"
#include <cstring>

DialogPinAccess::DialogPinAccess(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPinAccess)
{
    ui->setupUi(this);
    setWindowTitle( tr("Pin access") );
}

DialogPinAccess::~DialogPinAccess()
{
    delete ui;
}


void DialogPinAccess::setPassword(char* pw){
    strcpy(password,pw);
}

//void DialogPinAccess::checkPassword(QString pw){

//    if(!strcmp(password,newpw))
//        this->close();
//    else
//        this->open();

//}

void DialogPinAccess::on_accessData_clicked()
{
    QString Qpw = ui->password->text();
    QByteArray ba = Qpw.toLatin1();
    char *pw;
    strcpy(pw,ba.data());

    emit checkPassword(pw);
}
