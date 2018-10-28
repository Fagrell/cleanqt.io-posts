# Crash course in Qt for C++ developers, part 4

__Blog post published on [cleanqt.io](www.cleanqt.io)__

As a modern C++ developer, I'm sure you're striving to use raw pointers as much as you can. Handling raw allocations and deallocations are for the professionals, smart pointers are for the nit-witted programmers. As they say - the more `new` and `delete` scattered all over the codebase the merrier. Right[⸮](https://en.wikipedia.org/wiki/Irony_punctuation)

Well... no. Please forgive me. However, in my defence, this is partially true for Qt. If you scan through the Qt examples you will see a lot of `new` and a few `delete`. In this post we'll explore why this is the case and how memory is managed the Qt-way.

This is the fourth post in the series "Crash course in Qt for C++ developers" covering the __Hierarchy and memory management__ in Qt. The other topics are listed below.

1. [Events and the main event loop](https://www.cleanqt.io/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-1)
2. [Meta-object system (including QObject and MOC)](https://www.cleanqt.io/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-2)
3. [Signals and slots - communication between objects](https://www.cleanqt.io/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-3)
4. Hierarchy and memory management
5. [MVC or rather model/view and delegate programming](https://www.cleanqt.io/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-5)
6. [Choose your camp Quick/QML-camp or Widgets-camp](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-6)
7. Qt Quick/QML example
8. Qt Widgets example
7. Tooling, e.g. Qt Creator
8. Remaining good-to-know topics
9. Where to go from here?

### Ownership model
Similarly to smart pointers, an important concept in Qt is the ownership. Many of Qt's objects are organised in a tree hierarchy. The relation between the objects in the hierarchy is called the parent-child relationship. The parent owns the children and each child belongs to a single parent. You might recognise this design pattern as it's similar to the [Composite pattern](https://en.wikipedia.org/wiki/Composite_pattern).

Setting the parent for a `QObject` is simply done by either passing it in during construction or calling `setParent()` after creation. A good practice when defining your own `QObject` is to declare a constructor that takes the parent:

```cpp
class GrandParent : public QObject {
  Q_OBJECT
...
public:
  explicit GrandParent(QObject* parent = nullptr) : QObject{parent} {}

  void createFamily() {
     auto parent = new QObject{this}; //parent->setParent(this); also works
     auto child = new QObject{parent}; //child->setParent(parent); ditto
  } //no memory leaked!
};
```

After invoking `createFamily()` the `GrandParent` object will then own (internally keeps a pointer to) the `parent` object. Likewise, the `parent` will then own the `child`. And here comes the beauty of the pattern - during the destruction of the `GrandParent` all it's children and grandchildren will recursively become deleted. It's also possible to manually delete a child without removing it from the parent, it's done automatically from the destructor of the child object.

Now you might wonder why such a design was introduced in Qt. It's actually quite understandable considering what Qt is especially designed for: creating GUI applications. Let me elucidate with an example. Imagine that you're developing a graphics editor. The graphics editor will consist of a few widgets embedded in the application's window: a menu, a tools panel, a history panel, a colour panel and the main view where the canvas will be. Each panel will have icons, text and other components. It doesn't really make sense for a specific icon to exist without its panel, neither does the panel without the window. However, what does make sense is for the window to own the panels, and they in turn own their respective icons and components. And voilà - the parent-child relationship was born. In fact, this pattern is even enforced when using _Qt Widgets_, which will be covered in a future post.

### Useful functions when navigating the hierarchy

As you perhaps realised in the example above, the hierarchy tree might become very big and it will be difficult to have a good understanding of its full length. By calling [QObject::dumpObjectTree()](http://doc.qt.io/qt-5/qobject.html#dumpObjectTree) it's possible to output the current hierarchy e.g.

```cpp
int main() {
  //Setting up a hiearchy of QObjects
  QObject jacqueline;
  jacqueline.setObjectName("Jacqueline Bouvier");
  auto marge = new QObject{&jacqueline};
  marge->setObjectName("Marge Bouvier");
  auto bart = new QObject{marge};
  bart->setObjectName("Bart Simpson");
  auto lisa = new QObject{marge};
  lisa->setObjectName("Lisa Simpson");
  auto maggie = new QObject{marge};
  maggie->setObjectName("Magge Simpson");
  auto patty = new QObject{&jacqueline};
  patty->setObjectName("Patty Bouvier");
  auto selma = new QObject{&jacqueline};
  selma->setObjectName("Selma Bouvier");
  auto ling = new QObject{selma};
  ling->setObjectName("Ling Bouvier");

  //Print the tree
  jacqueline.dumpObjectTree();
}
```

will output

>QObject::Jacqueline Bouvier</br>
>&nbsp;&nbsp;&nbsp;&nbsp;QObject::Marge Bouvier</br>
>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;QObject::Bart Simpson</br>
>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;QObject::Lisa Simpson</br>
>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;QObject::Magge Simpson</br>
>&nbsp;&nbsp;&nbsp;&nbsp;QObject::Patty Bouvier</br>
>&nbsp;&nbsp;&nbsp;&nbsp;QObject::Selma Bouvier</br>
>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;QObject::Ling Bouvier</br>

This is especially useful, together with [QObject::dumpObjectInfo()](http://doc.qt.io/qt-5/qobject.html#dumpObjectInfo), when an application looks or behaves oddly.

Other effective functions when navigating the tree are the following: [QObject::children()](http://doc.qt.io/qt-5/qobject.html#children), [QObject::findChild()](http://doc.qt.io/qt-5/qobject.html#findChild), [QObject::findChildren()](http://doc.qt.io/qt-5/qobject.html#findChildren) and [QObject::parent()](http://doc.qt.io/qt-5/qobject.html#parent). The function names are relatively self-explanatory but feel free to click on the links to dip into more details. By using `QObject::children` we can easily traverse the tree with a recursive function:

```cpp
void traverseTree(const QObject* parent) {
  if (!parent) return;

  for (const auto& child: qAsConst(parent->children())) {
    traverseTree(child);
  }
}

int main() {
  QObject jacqueline;
  ...
  traverseTree(&jacqueline);
}
```

Perhaps you have observed that the parent object `jacqueline` was created on the stack in both previous code snippets. This is fine as long as it doesn't have a parent. If the parent is set, we've most likely introduced an undefined behaviour. The parent will try to delete the stack object (if the object still exists) during destruction. Yikes! In other words, avoid setting parents on `QObjects` that have been created on the stack.

Another possibility to cause undefined behaviour is to use smart pointers together with Qt's parent-child relationship since it will result in conflicting ownership and it's easy to get into a dangling mess. Does that mean that smart pointers should never be used in a Qt application? No, not at all. Actually Qt even provides smart pointers long before they were standardised in C++. Let's look at them next!

### Smart pointers
Before we dive into the different smart pointers, let's discuss when smart pointers are usually needed in a Qt application. Well... not very often to be frank. In general, most of the cases are when a class doesn't need to inherit from a `QObject`, e.g. for data structures. One such example is when using the [PIMPL-idiom](https://wiki.qt.io/D-Pointer) which, in idiomatic C++, should be implemented using a smart pointer; preferably a `std::unique_ptr`. Two other examples of smart pointer usage are when using local objects and objects that require multiple ownership. Now that we've established the use cases, let's explore which smart pointers that are included in Qt. 

| Qt | STL counterpart |
| --- | --- |
| [QSharedDataPointer](http://doc.qt.io/qt-5/qshareddatapointer.html) | - |
| [QExplicitlySharedDataPointer](http://doc.qt.io/qt-5/qexplicitlyshareddatapointer.html) | - |
| [QSharedPointer](http://doc.qt.io/qt-5/qsharedpointer.html) | std::shared_ptr |
| [QWeakPointer](http://doc.qt.io/qt-5/qweakpointer.html) | std::weak_ptr |
| [QPointer](http://doc.qt.io/qt-5/qpointer.html) | - |
| [QScopedPointer](http://doc.qt.io/qt-5/qscopedpointer.html) | std::unique_ptr |
| [QScopedArrayPointer](http://doc.qt.io/qt-5/qscopedarraypointer.html) | - |

Feel free to research them on your own; covering all of them is outside the scope of this post. However, there is one particular pointer which I found a bit more useful and is worth mentioning for C++ developers: the `QPointer`.

#### QPointer
If your memory is exceptionally good, you might remember from a previous post...
> `QObjects` can be used with guarded pointers.

The guarded pointer is the `QPointer`. A `QPointer` doesn't own the object it's referring to and can be considered a _weak pointer_. It can only point to a `QObject` (or a subclass of it). It behaves similarly to a standard raw pointer except when the referenced object is deleted. Instead of just dangling it's automatically set to `0`, e.g:

```cpp
int main() {
  QObject parent;
  auto child = new QObject{&parent};
  QPointer<QObject> guarded_child{child};
  delete child;
  Q_ASSERT(!guarded_child); //OK
}
```
Of course, it goes without saying that this is useful to verify if an object is still alive.

#### STL or Qt?
Since we now have two implementations in our arsenal, one might wonder which ones should be used: STL's or Qt's smart pointers? Qt's smart pointers exist for historical reasons and only provide one benefit over the STL ones: Qt's guarantee ABI compatibility within a major version, STL's don't. However, my recommendation is to prefer the STL smart pointers simply because of these reasons:

* The STL smart pointers are usually implemented by the same people who implement the C++ compiler and are therefore most likely better optimised.
* Altough Qt's smart pointers mimic the interface of the STL's there are still some subtleties. Since more developers are perhaps more familiar with STL pointers, they are also the ones that should be preferred.

Similar to smart pointers Qt also provides its own sets of containers. This post won't cover those as I've already written another [post](https://www.cleanqt.io/blog/exploring-qt-containers) exploring them in details. However, I do want to give a suggestion: keep using the STL containers where it's possible. For similar reasons as mentioned above [(and many more)](https://www.cleanqt.io/blog/exploring-qt-containers) STL's alternatives are preferred over Qt's.

### Wrap up

We've now learnt the Qt-way of handling memory. If you're coming from a modern C++ background, perhaps you read through the whole post and felt very uncomfortable with all the raw `new`s and `delete`s. As it may be, it will take some time to adjust. Perhaps you'll only start to appreciate Qt's hierarchy model after you've used it for some time. Nonetheless, consider the main purpose of using smart pointers: to define ownership and handle memory. This is exactly what the Qt's parent-child relationship is designed to do. And speaking of ownership, it's again worth mentioning that the __parent-child relationship should never be mixed  with smart pointers that also handle ownership__, such as unique and shared pointers.  Mixing ownership will most likely lead to a tragedy and undefined behaviour.  If it helps, it's possible to design a whole Qt application without ever using `delete`. 

As they say "When in Rome, do as the Romans do". You'll be fine!
