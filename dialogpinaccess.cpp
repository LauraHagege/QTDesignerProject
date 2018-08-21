#include "dialogpinaccess.h"
#include "ui_dialogpinaccess.h"
#include <cstring>
#include <iostream>

using namespace std;

DialogPinAccess::DialogPinAccess(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPinAccess)
{
    ui->setupUi(this);
    setWindowTitle( tr("Pin access") );
    ui->wrongPin->setVisible(false);
}

DialogPinAccess::~DialogPinAccess()
{
    delete ui;
}


void DialogPinAccess::setPassword(char* pw){
    strcpy(password,pw);
}

void DialogPinAccess::setWrongPin(){
    ui->wrongPin->setVisible(true);
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
    cout << "password " << pw << endl;
    emit checkPassword(pw);
}
