# Crash course in Qt for C++ developers, part 2

__Blog post published on [cleanqt.io](www.cleanqt.io)__

In the last post we learnt about the event loop and how and why you should avoid blocking it. Another concept that you might have already stumbled upon is the __meta-object system__. If you have gone through the compile output of a Qt project you should have seen some traces of it; the output looks a bit different compared to compiling standard C++ code. If you haven't done it already, try to compile one of the Qt examples. See if you can spot something unusual about the output.

This is the second post in the series "Crash course in Qt for C++ developers" covering the __Meta-object system (including QObject and MOC)__. The other topics are listed below.

1. [Events and the main event loop](https://www.cleanqt.io/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-1)
2. Meta-object system (including QObject and MOC)
3. [Signals and slots - communication between objects](https://www.cleanqt.io/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-3)
4. [Hierarchy and memory management](https://www.cleanqt.io/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-4)
5. [MVC or rather model/view and delegate programming](https://www.cleanqt.io/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-5)
6. [Choose your camp Quick/QML-camp or Widgets-camp](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-6)
7. Qt Quick/QML example
8. Qt Widgets example
7. Tooling, e.g. Qt Creator
8. Remaining good-to-know topics
9. Where to go from here?

Let's go back to the Qt example. Did you really compile it? Did you see something like...?
> Automatic MOC for target...

Qt includes some C++ extension compilers which automatically generate extra code and files before compilation and linking. The __Meta-Object Compiler__, or __MOC__ for short, is a tool that parses all header files in the project. Depending on what's defined in them, the tool might (see `Q_OBJECT` below) generate a companion file for a class named moc\_class-name.cpp. To enable the MOC step, have a look at the [Qt documentation](http://doc.qt.io/qt-5/moc.html) or if you're familiar with CMake: just add `AUTOMOC ON` as a property to the target. 

Qt has been criticised by some developers for this extra compilation step. However, it's done for good reasons. If you're interested in why such a decision was made, Qt has composed an interesting article which has [got you covered](http://doc.qt.io/qt-5/why-moc.html).

The generated moc\_-files implement functions which are used for several different features. The perhaps most important ones are the _signals and slots mechanism_, the _run-time type information (RTTI)_, and the _dynamic property system_. 

The first feature, signals and slots, is the main reason for introducing the MOC system and is also a big subject on its own. Therefore, a whole post will be dedicated to it; look for the next post in the series. This post will cover the last two: run-time type information and the dynamic property system as well as a very special object called the `QObject`. 

### The man, the myth, the QObject

You might wonder what's so special about the `QObject`. In order to do any of the aforementioned concepts in Qt you must subclass a `QObject` and for signals and slots you'll also have to define the macro `Q_OBJECT` in the class. Actually, just to get a sense of how important the object is, all [Qt identity objects](http://doc.qt.io/qt-5/object.html) inherit from the `QObject`. And there are many of them in the framework. 

Although the `Q_OBJECT` is optional when subclassing a `QObject`, the [official documentation](http://doc.qt.io/qt-5/qobject.html) recommends to always do it: without it, some functions may result in unexpected behaviour. The macro `Q_OBJECT` is what the MOC is looking for in order to generate the moc\_-file. A typical example of a Qt class will look like this:

```cpp
#include <QObject>

class MyClass : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY(MyClass)
  // ... properties ...
  
public:
  MyClass();
  // ... functions and member variables ...

};
``` 

Actually, this is probably what _most_ of your Qt classes will look like. By subclassing `QObject` we have now enabled the _meta-object system_ and can now retrieve some runtime information about the class, such as the name of it:

```cpp
MyClass myclass;
qDebug() << myclass.metaObject()->className();
```

`qDebug()` provides an output stream which can be used for debugging information, i.e. the code above will output:

> MyClass

You might now think "This is nothing new! It is already supported in C++ using `typeid(myclass).name()`". 

Yes, true! However, the Qt system doesn't require native RTTI compiler support. In addition, without a RTTI supported compiler, it's still possible to perform dynamic casts on `QObjects` using [qobject_cast()](http://doc.qt.io/qt-5/metaobjects.html) instead of the traditional `C++ dynamic_cast()`. `qobject_cast` even works across dynamic library boundaries.

Did you see the macro `Q_DISABLE_COPY(MyClass)` defined in the class? It is, not surprisingly, used to prevent users from copying or moving the object. This is by design and I've written another blog post which is covering the reasons behind this: [why qobject subclasses are not copyable](https://www.cleanqt.io/blog/why-qobject-subclasses-are-not-copyable).

### Qt's sophisticated property system

In addition to RTTI, the class can now be extend with properties. Qt's property system is extremely flexible and works cross-platform without relying on any non-standard compiler features. In fact  Qt's property system is so powerful that you can dynamically create properties during run-time, such as: 

```cpp
MyClass myclass;
myclass.setProperty("text","Hello world");
qDebug() << myclass.property("text").toString();
```

which will, as expected, output:
> Hello world

There are [several reasons](http://doc.qt.io/qt-5/properties.html) why you'd want to extend your class with properties instead of using the standard member variables. I believe the main usage is to export them to the QML engine, which will be covered in a future post. To add properties to the class, the macro `Q_PROPERTY` is used, for example:

```
Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)
```

where the first argument is the type followed by the name of the property. The __READ__ and __WRITE__ accessors corresponds to getters and setters, whereas __NOTIFY__ is optional and used to specify a signal (more on this in the next post). These are only some of the basic functionality that the property system provides, however it can be extended with many more features which you can read more about [here](http://doc.qt.io/qt-5/properties.html).

Let's extend our class with a couple of properties:

```cpp
class MyClass : public QObject {
  ...
  Q_PROPERTY(QString name READ name WRITE setName)
  Q_PROPERTY(QString creator READ creator WRITE setCreator)
  ...
public:
  ...
  const QString& name() const;
  void setName(QString name);

  const QString& creator() const;
  void setCreator(QString creator);

private:
  QString m_name{"DeLorean"};
  QString m_creator{"Dr. Emmett Brown"};
};
```

We've only added two getters and setters; each getter and setter pair for each property. It's now possible to, for example, loop through the properties by using the `QMetaObject` and print them:

```cpp
 MyClass myclass;
 const QMetaObject* metaobject = myclass.metaObject();
 int count = metaobject->propertyCount();
 for (int i{0}; i < count; ++i) {
   QMetaProperty metaproperty = metaobject->property(i);
   const char* name = metaproperty.name();
   QVariant value = myclass.property(name);
   qDebug() << name << ": " << value.toString();
 }
```

which outputs:
> name:  "DeLorean" <br/>
> creator:  "Dr. Emmett Brown"

values are stored as [QVariants](http://doc.qt.io/qt-5/qvariant.html). A `QVariant` is used as union for most Qt data types.

If you'd like to explore the full example code, feel free to fork the following GitHub project:
> https://github.com/Fagrell/cleanqt.io-posts

and browse to `/04-sep-2018/example`.

### QObject enables even more features

In this post we've covered MOC, Qt's RTTI, qobject_cast and the property system. These features are very impressive alone, however by using the `meta-object system` you'll gain [many more](http://doc.qt.io/qt-5/object.html). Here are some of them:

* `QObjects` enable translations of strings for internationalisation using [tr()](http://doc.qt.io/qt-5/qobject.html#tr).
* Remember the events that were covered in the previous post? All `QObjects` can receive and filter `QEvents`. 
* `QObjects` can be used with [guarded pointers](http://doc.qt.io/qt-5/qpointer.html) which are automatically set to 0 after deletion.
* `QObjects`, `Q_OBJECT` and the MOC enable the perhaps most powerful feature of them all: the seamless inter-object communication `signals and slots`, which will be covered in the next post.

As a final point, I would like to mention that, although the `QObject` is extremely useful, the overhead of subclassing a `QObject` is [rather significant](https://stackoverflow.com/questions/15763088/how-heavy-is-qobject-really). It should probably be avoided if you're not going to use any of its features. Plain data structures shouldn't also inherit from `QObjects` since they then won't be copyable or movable.

See you next time!