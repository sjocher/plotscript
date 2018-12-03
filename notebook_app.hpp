#ifndef NOTEBOOK_APP_HPP
#define NOTEBOOK_APP_HPP

#include <QWidget>
#include <atomic>
#include "input_widget.hpp"
#include "output_widget.hpp"
#include "cpanel.hpp"
#include "interpreter.hpp"
#include "semantic_error.hpp"
#include "startup_config.hpp"
#include "guiParseInterp.hpp"


class NotebookApp: public QWidget {
    Q_OBJECT
public:
    NotebookApp();
    ~NotebookApp();
    void repl(QString data);
private:
    QString m_parseData;
    void loadStartup();
    InputWidget * input;
    OutputWidget * output;
    cPanel * controlpanel;
    bool kernalRunning = true;
    Interpreter interp;
    parseQueue pQ;
    resultQueue rQ;
    messageQueue mQ;
    guiParseInterp pI;
    std::atomic_bool solved;
    void closeEvent(QCloseEvent *event);
public slots:
    void setData(QString data);
    void handleStart();
    void handleStop();
    void handleReset();
    void handleinterrupt();
signals:
    void plotscriptResult(Expression result);
    void plotscriptError(std::string error);
};
#endif
