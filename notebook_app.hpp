#ifndef NOTEBOOK_APP_HPP
#define NOTEBOOK_APP_HPP

#include <QWidget>
#include "input_widget.hpp"
#include "output_widget.hpp"
#include "interpreter.hpp"
#include "semantic_error.hpp"
#include "startup_config.hpp"

class NotebookApp: public QWidget {
    Q_OBJECT
public:
    NotebookApp(QWidget * parent = nullptr);
    void repl(QString data);
private:
    QString m_parseData;
    Interpreter interp;
    void loadStartup();
public slots:
    void setData(QString data);
signals:
    void plotscriptResult(Expression result);
    void plotscriptError(std::string error);
};
#endif
