#ifndef INPUT_WIDGET_HPP
#define INPUT_WIDGET_HPP

#include <QWidget>
#include <QKeyEvent>
#include <QPlainTextEdit>

class InputWidget: public QPlainTextEdit {
    Q_OBJECT
public:
    InputWidget(QWidget * parent = nullptr);
    QPlainTextEdit * text;
    bool event(QEvent* e);
    QString getParseData();
private:
    QString m_rawData;
    
signals:
    void valueChanged(QString newValue);
};
#endif

