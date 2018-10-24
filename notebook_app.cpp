#include "notebook_app.hpp"
#include <QLineEdit>
#include <QLayout>
#include <QDebug>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "startup_config.hpp"

NotebookApp::NotebookApp() {
    loadStartup();
    QVBoxLayout * layout = new QVBoxLayout;
    input = new InputWidget(this);
    input->setObjectName("input");
     connect(input, &InputWidget::valueChanged, this, &NotebookApp::setData);
    output = new OutputWidget(this);
    output->setObjectName("output");
     connect(this, &NotebookApp::plotscriptResult, output, &OutputWidget::recievePlotscript);
     connect(this, &NotebookApp::plotscriptError, output, &OutputWidget::recieveError);
    layout->addWidget(input);
    layout->addWidget(output);
    setLayout(layout);
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
    repl(m_parseData);
    m_parseData = QString();
}

void NotebookApp::repl(QString data) {
    std::istringstream expression(data.toStdString());
    if(!interp.parseStream(expression)){
        emit plotscriptError("Error: Could not parse.");
    }
    else{
        try{
            Expression exp = interp.evaluate();
            emit plotscriptResult(exp);
            exp = Expression();
        }
        catch(const SemanticError & ex){
            std::string error = ex.what();
            emit plotscriptError(error);
        }
    }
}
