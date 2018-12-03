#ifndef CPANEL_HPP
#define CPANEL_HPP

#include <QWidget>
#include <QPushButton>

class cPanel: public QWidget {
    Q_OBJECT
public:
    cPanel();
    QPushButton * start;
    QPushButton * stop;
    QPushButton * reset;
    QPushButton * itrpt;
};

#endif
