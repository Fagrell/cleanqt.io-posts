#include "myclass.h"

MyClass::MyClass() {
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
