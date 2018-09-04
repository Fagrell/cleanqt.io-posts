#pragma once

#include <QObject>

class MyClass : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY(MyClass)
  Q_PROPERTY(QString name READ name WRITE setName)
  Q_PROPERTY(QString creator READ creator WRITE setCreator)

public:
  MyClass();
  const QString& name() const;
  void setName(QString name);

  const QString& creator() const;
  void setCreator(QString creator);

private:
  QString m_name{"DeLorean"};
  QString m_creator{"Dr. Emmett Brown"};
};
