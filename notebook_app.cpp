#include "notebook_app.hpp"
#include <QLineEdit>
#include <QLayout>
#include <QDebug>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <thread>
#include "startup_config.hpp"

NotebookApp::NotebookApp() {
    loadStartup();
    //initalize all the variables
    solved.store(false);
    pI.startThread(&mQ, &pQ, &rQ, &solved, &interp);
    QVBoxLayout * layout = new QVBoxLayout;
    input = new InputWidget;
    input->setObjectName("input");
     connect(input, &InputWidget::valueChanged, this, &NotebookApp::setData);
    output = new OutputWidget;
    output->setObjectName("output");
     connect(this, &NotebookApp::plotscriptResult, output, &OutputWidget::recievePlotscript);
     connect(this, &NotebookApp::plotscriptError, output, &OutputWidget::recieveError);
    controlpanel = new cPanel;
     connect(controlpanel->start, SIGNAL(clicked()), this, SLOT(handleStart()));
     connect(controlpanel->stop, SIGNAL(clicked()), this, SLOT(handleStop()));
     connect(controlpanel->reset, SIGNAL(clicked()), this, SLOT(handleReset()));
     connect(controlpanel->itrpt, SIGNAL(clicked()), this, SLOT(handleinterrupt()));
    layout->addWidget(controlpanel);
    layout->addWidget(input);
    layout->addWidget(output);
    setLayout(layout);
}

void NotebookApp::handleStart() {
    kernalRunning = true;
    if(pI.size() == 0) {
        pI.startThread(&mQ, &pQ, &rQ, &solved, &interp);
    }
}

void NotebookApp::handleStop() {
    kernalRunning = false;
    if(pI.size() > 0) {
        pQ.push(QString("%%%%%"));
        pI.joinAll();
    }
}

void NotebookApp::handleReset() {
    if(pI.size() > 0) {
        pQ.push(QString("%%%%%"));
        pI.joinAll();
    }
    interp.reset();
    output->scene->clear();
    input->clear();
    pI.startThread(&mQ, &pQ, &rQ, &solved, &interp);
    loadStartup();
}

void NotebookApp::handleinterrupt() {
    if(kernalRunning) {
        handleReset();
        emit plotscriptError("Error: interpreter kernel interrupted");
    }
}

void NotebookApp::loadStartup() {
    std::ifstream startup(STARTUP_FILE);
    if(!interp.parseStream(startup)) {
        emit plotscriptError("Error: Could not parse.");
    } else {
        try {
            Expression exp = interp.evaluate();
        } catch (const SemanticError & ex){
            std::string error = ex.what();
            emit plotscriptError(error);
        }
    }
}

void NotebookApp::setData(QString data) {
    m_parseData = data;
    input->setEnabled(false);
    repl(m_parseData);
    m_parseData = QString();
}

void NotebookApp::repl(QString data) {
    QString temp = data;
    if(kernalRunning) {
        pQ.push(data);
        Expression exp;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if(solved) {
            rQ.try_pop(exp);
            emit plotscriptResult(exp);
        } else {
            std::string error;
            mQ.try_pop(error);
            emit plotscriptError(error);
        }
    } else {
        emit plotscriptError("Error: interpreter kernel not running");
    }
    input->setEnabled(true);
}


void NotebookApp::closeEvent(QCloseEvent *event) {
    QCloseEvent *temp = event;
    if(temp == event) {
        if(pI.size() > 0) {
            pQ.push(QString("%%%%%"));
            pI.joinAll();
        }
    }
}

NotebookApp::~NotebookApp() {
    if(pI.size() > 0) {
        pQ.push(QString("%%%%%"));
        pI.joinAll();
    }
}
