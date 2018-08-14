
# Why QObject subclasses are not copyable

__Blog post published on [cleanqt.io](www.cleanqt.io)__ 

If you try to copy a class that derives from a `QObject` it will result in a compiler error, e.g.

```cpp
class MyClass : public QObject {
  Q_OBJECT
} my_class;

auto my_class_copy = my_class;
```

with Qt5 and using C+11 (supporting `=delete`):
>error: use of deleted function ‘MyClass::MyClass(const MyClass&)’

or with earlier versions:
>error: ‘QObject::QObject(const QObject&)’ is private within this context`

This behaviour is by design. But why is the copy constructor (as well as the assignment operator) deleted? What if you still want to copy it? If it's not copyable is it then movable? The following post will examine these questions as well as explore whether it's a good practice to repeat the deletion in the custom subclass. Let's dive in!

There are [several reasons](http://doc.qt.io/qt-5/object.html) why a `QObject` can't be copied. The two biggest reasons are:

* `QObjects` usually communicate with each other using the [signals and slots mechanism](http://doc.qt.io/qt-5/signalsandslots.html). It's unclear whether the connected `signals` and/or `slots` should be transferred over to the copy. If they would be transferred over, it would imply that other `QObjects` would automatically subscribe to the copy. This would most likely lead to confusion and unwanted side-effects for the developers.
* `QObjects` are organised in object trees. Usually one instance of a `QObject` has one parent and several children. Where should the copy be organised in this hierarchy? Should the children (and grandchildren...) also be copied?

Other reasons, but perhaps less critical, are:

* A `QObject` can be considered unique by giving it a name which could be used as a reference key, i.e. by setting the  `QObject::objectName()`. If the name is set, it's unclear which name should be given to the copy.
* `QObjects` can be extended with new properties during runtime. Should these new properties also be inherited by the copy?

In general, `QObjects` are referred to by their pointer address by other objects. For example, this is the case in the aforementioned `signals and slots mechanism`. Because of this, `QObjects` can't be moved; connections between them would then be lost. In the source code of `QObject`, we can see that the are no move constructor or move assignment operator declared. However, since the copy constructor is deleted, the move constructor won't be implicitly generated and an compiler error will be reported if a developer attempts to move a `QObject`.

So you can't copy and you can't move a `QObject`, but what if you desire to copy the underlying data (or properties)? Qt's documentation distinguish between two object types in the [Qt Object Model](http://doc.qt.io/qt-5/object.html): value and identity objects. Value objects, such as `QSize`, `QColor` and `QString` are objects that can be copied and assigned. In contrast, the identity objects can't be copied but can be cloned. As you might have guessed, an example of an identity object is the `QOBject` or any class that derives from it. The meaning of cloning can be read from the official documentation:

>Cloning means to create a new identity, not an exact copy of the old one. For example, twins have different identities. They may look identical, but they have different names, different locations, and may have completely different social networks.

My understanding of cloning is that you could expose a `clone()`-function in a subclass which creates a new identity but not a _real_ copy, i.e:

```cpp
class MyClass : public QObject {
  Q_OBJECT
  
public:
  MyClass* clone() {
    auto copy = new MyClass;
    //copy only data
    return copy;
  }
  
private:
  //data
};
...

auto my_class = new MyClass;
auto my_class_clone = my_class->clone();
```

Although this is possible to do, I wouldn't recommend it. It could lead to unwanted side-effects as Qt developers will most likely have assumptions about `QObjects`. If you have the need of creating a clone, I would suggest to have a look at your overall design and architecture instead. Perhaps the data could be decoupled or factored out?

### Repeating `Q_DISABLE_COPY(Class)` in the subclass
On [stackoverflow](https://stackoverflow.com/questions/19854371/repeating-q-disable-copy-in-qobject-derived-classes)  it has been suggested to always redeclare the macro `Q_DISABLE_COPY(Class)` in your own class, i.e.:

```cpp
class MyClass : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY(MyClass) // See macro below
  public:
    QObject() {}
};
```

```cpp
#define Q_DISABLE_COPY(Class) \
  Class(const Class &); \
  Class &operator=(const Class &);
#endif
```
The main reason, as mentioned in the stackoverflow post, is to improve the error message. Without the macro, the following error message is reported using Qt4:

>error: ‘QObject::QObject(const QObject&)’ is private within this context`

With the macro, it's reporting:
>error: ‘MyClass::MyClass(const MyClass&)’ is private within this context`

The last error message is far more easier to understand for someone who's new to Qt.
     
However from Qt5, the macro was changed and declared as:

```cpp
#ifdef Q_COMPILER_DELETE_MEMBERS 
# define Q_DECL_EQ_DELETE = delete
#else
# define Q_DECL_EQ_DELETE
#endif

#define Q_DISABLE_COPY(Class) \
  Class(const Class &) Q_DECL_EQ_DELETE;\
  Class &operator=(const Class &) Q_DECL_EQ_DELETE;
#endif
```
Without adding the macro in the subclass, the following error message is displayed:
>error: use of deleted function ‘MyClass::MyClass(const MyClass&)’

The copy constructor and assignment operator have now been declared with `=delete` instead of just being private, resulting in a preferred error message.

Even though the error message has improved, I still believe it's valuable to redeclare the macro in the derived class, as it  documents the behaviour of the class. Someone who's new to Qt can quickly understand the intended usage: the object shouldn't (and can't) be copied!
