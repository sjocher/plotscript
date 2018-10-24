#ifndef INPUT_WIDGET_HPP
#define INPUT_WIDGET_HPP

#include <QWidget>
#include <QPlainTextEdit>

class InputWidget: public QPlainTextEdit {
    Q_OBJECT
public:
    InputWidget(QWidget* parent);
    void keyPressEvent(QKeyEvent* ev);
private:
    QString m_rawData;
signals:
    void valueChanged(QString newValue);
};
#endif

