#include "ScrollArea.h"

ScrollArea::ScrollArea(QWidget *parent) :
    QWidget(parent) {

    contents = new QWidget(this);
    current = 0;
}


void ScrollArea::jumpTo(int idx) {
    if (idx < 0) jumpTo(0);
    if (idx >= (int)childWidgets.size()) jumpTo(childWidgets.size()-1);
    QWidget *w = childWidgets[idx];
    contents->setGeometry(-w->x(), -w->y(), contents->width(), contents->height());    
    current = idx;
    emit slidTo(current);
    emit slidTo(childWidgets[current]);
}

void ScrollArea::jumpTo(QWidget *w) {
    for (int i = 0; i < (int)childWidgets.size(); i++) {
        if (childWidgets[i] == w) {
            jumpTo(i);
            return;
        }
    }
    printf("Error: Asked to jump to a non-child widget\n");
}


void VScrollArea::addWidget(QWidget *w) {
    w->setParent(contents);
    w->setGeometry(0, childWidgets.size()*height(), width(), height());
    childWidgets.push_back(w);    
    contents->setGeometry(0, -current*height(), width(), height()*childWidgets.size());
    w->show();
}

void HScrollArea::addWidget(QWidget *w) {
    w->setParent(contents);
    w->setGeometry(childWidgets.size()*width(), 0, width(), height());
    childWidgets.push_back(w);    
    contents->setGeometry(-current*width(), 0, width()*childWidgets.size(), height());
}

