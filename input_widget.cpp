#include "input_widget.hpp"
#include <QPlainTextEdit>
#include <QWidget>
#include <QKeyEvent>
#include <QDebug>


InputWidget::InputWidget() {}

void InputWidget::keyPressEvent(QKeyEvent *ev) {
    if ((ev->key() == Qt::Key_Return) && (ev->modifiers() == Qt::SHIFT)) {
        m_rawData = this->document()->toPlainText();
        emit valueChanged(m_rawData);
    } else {
        QPlainTextEdit::keyPressEvent(ev);
    }
}
