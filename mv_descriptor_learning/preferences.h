#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QWidget>
#include "params.h"


using namespace R4R;
using namespace std;

namespace Ui {
class Preferences;
}

class Preferences : public QWidget
{
    Q_OBJECT
    
public:
    explicit Preferences(QWidget *parent = 0);
    ~Preferences();


signals:

    void params_changed(CParameters params);

private slots:

    void on_saveButton_clicked();

    void on_loadButton_clicked();

    void on_aggComboBox_currentIndexChanged(int index);

public slots:
    void on_applyButton_clicked();

private:
    Ui::Preferences *ui;
    QWidget* m_parent;


};

#endif // PREFERENCES_H