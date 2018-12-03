#include "cpanel.hpp"
#include "notebook_app.hpp"
#include <QLayout>

cPanel::cPanel() {
    start = new QPushButton("Start Kernel");
    start->setObjectName("start");
    stop = new QPushButton("Stop Kernel");
    stop->setObjectName("stop");
    reset = new QPushButton("Reset Kernel");
    reset->setObjectName("reset");
    itrpt = new QPushButton("Interrupt");
    itrpt->setObjectName("interrupt");
    auto layout = new QHBoxLayout();
    layout->addWidget(start);
    layout->addWidget(stop);
    layout->addWidget(reset);
    layout->addWidget(itrpt);
    setLayout(layout);
}
