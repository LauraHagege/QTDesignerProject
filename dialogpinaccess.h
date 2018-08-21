#ifndef DIALOGPINACCESS_H
#define DIALOGPINACCESS_H

#include <QDialog>

namespace Ui {
class DialogPinAccess;
}

class DialogPinAccess : public QDialog
{
    Q_OBJECT

private:
    char password[11];

public:
    explicit DialogPinAccess(QWidget *parent = 0);
    ~DialogPinAccess();
    void setPassword(char* pw);
    //void checkPassword(QString pw);
    void setWrongPin();

private slots:
    void on_accessData_clicked();

signals:
    void checkPassword(char *pw);


private:
    Ui::DialogPinAccess *ui;

};

#endif // DIALOGPINACCESS_H
