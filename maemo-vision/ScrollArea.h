#ifndef SCROLL_AREA_H
#define SCROLL_AREA_H

#include <QWidget>
#include <vector>

/* A ScrollArea is a container widget that displays one child at a
 * time. */
class ScrollArea: public QWidget {
    Q_OBJECT

public:
    ScrollArea(QWidget *parent = 0);

    // Adds a new child widget. The child will be resized to be the
    // same size as the scroll area, and will have its parent set to
    // the scroll area instance
    virtual void addWidget(QWidget *) = 0;

public slots:
    // Teleport to a given child by index or pointer
    void jumpTo(int);
    void jumpTo(QWidget *);

signals:
    // These signals are emitted  due to a call to
    // jumpTo.
     void slidTo(int);
     void slidTo(QWidget *);

protected:
    QWidget *contents;
    std::vector<QWidget *> childWidgets;
    int current;
};

class HScrollArea : public ScrollArea {
public:
    HScrollArea(QWidget *parent = 0) : ScrollArea(parent) {}
    void addWidget(QWidget *);

};

class VScrollArea : public ScrollArea {
public:
    VScrollArea(QWidget *parent = 0) : ScrollArea(parent) {}
    void addWidget(QWidget *);

};

#endif
