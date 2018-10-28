#include "myclass.h"

MyClass::MyClass(QObject* parent) :
  QObject{parent} {
}

const QString& MyClass::name() const {
  return m_name;
}

void MyClass::setName(QString name) {
  m_name = std::move(name);
}

const QString& MyClass::creator() const {
  return m_creator;
}

void MyClass::setCreator(QString creator) {
  m_creator = std::move(creator);
}



GrandParent::GrandParent() {}

void GrandParent::createFamily() {
   auto parent = new QObject{this}; //parent->setParent(this); also works
   auto child = new QObject{parent}; //child->setParent(parent); ditto
   Q_UNUSED(child)
}
