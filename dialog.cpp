#include "dialog.h"
#include "ui_dialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>
#include <QDebug>

using namespace std;

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButton_clicked()
{
   // QString filename=QFileDialog::getOpenFileName(this,tr("Open File"),
                                                  //"C://","All file (*.*)");

    Filepath = QFileDialog::getExistingDirectory(0, ("Select Output Folder"), QDir::currentPath());
    //QMessageBox::information(this,tr("Directory path"), Filepath);

    this->close();
    //qDebug() <<  OutputFolder << endl;


}

QString Dialog::getFilepath(){
    //QMessageBox::information(this,tr("Directory path"), Filepath);
    return Filepath;
}
