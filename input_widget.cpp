#include "input_widget.hpp"
#include <QKeyEvent>
#include <QLayout>
#include <QDebug>

InputWidget::InputWidget(QWidget* parent) {
    setParent(parent);
    text = new QPlainTextEdit();
    auto layout = new QGridLayout();
    layout->addWidget(text, 0, 0);
    setLayout(layout);
}

bool InputWidget::event(QEvent *e) {
    QKeyEvent* key = static_cast<QKeyEvent*>(e);
    if ((key->key() == Qt::Key_Return) && (key->modifiers() == Qt::SHIFT)) {
        m_rawData = text->toPlainText();
        emit valueChanged(m_rawData);
        text->clear();
    }
    return true;
}
