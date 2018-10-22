#include "notebook_app.hpp"
#include <QLineEdit>
#include <QLayout>
#include <QDebug>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "startup_config.hpp"

NotebookApp::NotebookApp(QWidget * parent): QWidget(parent) {
    loadStartup();
    auto *layout = new QGridLayout();
    auto input = new InputWidget();
    input->setObjectName("input");
        connect(input, &InputWidget::valueChanged, this, &NotebookApp::setData);
    auto output = new OutputWidget();
    output->setObjectName("input");
        connect(this, &NotebookApp::plotscriptResult, output, &OutputWidget::recievePlotscript);
        connect(this, &NotebookApp::plotscriptError, output, &OutputWidget::recieveError);
    layout->addWidget(input, 0, 0);
    layout->addWidget(output, 1, 0);
    setLayout(layout);
}

void NotebookApp::loadStartup() {
    std::ifstream startup(STARTUP_FILE);
    if(!interp.parseStream(startup)) {
        emit plotscriptError("Invalid Startup. Could not parse.");
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
    repl(m_parseData);
}

void NotebookApp::repl(QString data) {
    std::istringstream expression(data.toStdString());
    if(!interp.parseStream(expression)){
        emit plotscriptError("Invalid Startup. Could not parse.");
    }
    else{
        try{
            Expression exp = interp.evaluate();
            m_result = exp;
            emit plotscriptResult(m_result);
        }
        catch(const SemanticError & ex){
            std::string error = ex.what();
            emit plotscriptError(error);
        }
    }
}
