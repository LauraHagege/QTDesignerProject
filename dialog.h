#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QString>
#include <QGraphicsScene>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT
   // QString Filepath;

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
    //QString getFilepath();
    char *getStudyName();
    char *getDicomDirPath();
    char *getStudyPath();
    int getStudyNumber();
    QGraphicsScene *welcomingScene;
    QPixmap welcomeImg;



private slots:
    void on_pushButton_clicked();

private:
    Ui::Dialog *ui;
    char studyName[10];
    char dicomdirpath[150];
    char studyPath[150];
    int studyNumber;
};

#endif // DIALOG_H
