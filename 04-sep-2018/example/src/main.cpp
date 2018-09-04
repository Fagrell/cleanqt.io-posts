#include <QDebug>
#include <QMetaProperty>

#include "myclass.h"

void printAllProperties(const QObject& object) {
  const QMetaObject* metaobject = object.metaObject();
  int count = metaobject->propertyCount();
  for (int i{0}; i < count; ++i) {
    QMetaProperty metaproperty = metaobject->property(i);
    const char* name = metaproperty.name();
    QVariant value = object.property(name);
    qDebug() << name << ": " << value.toString();
  }
}

int main() {
 MyClass myclass;
 qDebug() << myclass.metaObject()->className();

 printAllProperties(myclass);

 return 0;
}
