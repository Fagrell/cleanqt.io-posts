# Crash course in Qt for C++ developers, part 3

__Blog post published on [cleanqt.io](www.cleanqt.io)__

Envision you've decided to go solo and start an indie game studio to fulfil your lifelong dream - to design and create a turn-based strategy game. You begin your endeavour by outlining the different components and sketch on the overall architecture of the game. The game will have some units which can be moved and controlled by using the mouse. There will be an AI component which will perhaps kick in after the units have completed their movements. The AI will then decide how the enemies should respond.

In other words, a sequence of actions need to happen: the mouse click, then the movement of units and then the response of the enemies. However, you probably don't want the mouse logic to directly depend on the unit-classes; it will be used for much more. For the same reason the unit-classes shouldn't need to rely on the AI nor the enemy logic; possibly the same AI will be used for many different enemies. I'm sure you're a great developer so you're aiming to decouple these different components. Nonetheless, the components need to communicate with each other and you need some kind of callback-mechanism. And this, ladies and gentlemen, this is where Qt's __signals and slots__ comes to the rescue.

This is the third post in the series "Crash course in Qt for C++ developers" covering the __signals and slots__ mechanism. The other topics are listed below.

1. [Events and the main event loop](https://www.cleanqt.io/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-1)
2. [Meta-object system (including QObject and MOC)](https://www.cleanqt.io/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-2)
3. Signals and slots - communication between objects
4. Hierarchy and memory management
5. MVC or rather model/view and delegate programming
6. Choose your camp Quick/QML-camp or Widgets-camp
7. Tooling, e.g. Qt Creator
8. Remaining good-to-know topics
9. Where to go from here?

The most common usage of the signals and slots mechanism is for inter-object communication and has been around since the birth of Qt. It's a system where an object broadcasts a signal with data and any listeners, including the sender itself, can subscribe to that signal. The listeners then redirect the data to a member function, or what's so called the _slot_, where it's then processed.

You might wonder how this system is different from using a [standard callback-mechanism](http://doc.qt.io/qt-5/signalsandslots.html) used in many other GUI tools. One benefit of Qt's solution is that the sender is not dependent on the listener, it's just firing off the signal. In a callback mechanism the sender usually needs to know which listeners to notify. Another benefit, in contrast to many other callback implementations, is that the signals and slots functions are type safe.

OK - enough text, let's look at some code.

### The basics

#### Signals
Let's start with signals. In order to enable signals for a class, it has to inherit from `QObject` and use the `Q_OBJECT` macro. We'll define a signal called `signalName` with three arguments. The arguments are used to pass in data, which will later be available in the slots.

```cpp
class MySender : public QObject {
  Q_OBJECT
  ...

signals:
  void signalName(const QString& use, int different, float types);
};
```

The `signals` keyword is essentially a simple macro defined to `public` but is also used by the MOC (remember from the [previous post](https://www.cleanqt.io/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-2)?) to generate the implementation of the member function `signalName()`. The implementation can be found in the generated file moc\_mysender.cpp. The signal is declared similarly to a regular function, except for a constraint on the return value - it has to be `void`. To emit the signal, we'll only call the function with the addition of appending `emit`, i.e.:

```cpp
void MySender::functionToEmitSignal() {
  emit signalName("Important data that will be sent to all listeners", 42, 1.618033);
}
```

`emit` is a macro defined to do... nothing. It's only used for documentation and readability purposes. You could omit it: the code would compile fine and the outcome would be the same, but again, we want to be explicit that it's a signal. Also, notice that we don't need to depend on any of the listeners, we're only so far just emitting a signal. So how would an object listen to it?

#### Slots
A listener object will redirect the signal to one of its slots. However, slots are not limited to member functions but can also consist of lambdas, non-member functions and functors. The arguably most common use is to define the slot within an object, so let's start by creating a new class `MyReceiver` which has a slot called `slotName()`:

```cpp
class MyReceiver : public QObject {
  Q_OBJECT
  ...

public slots:
  void slotName(const QString& use, int different, float types) {
    qDebug() << use << " " << different << " " << types;
  }
};
```
Similarly to the signal object, the receiver class needs to inherit from `QObject` and also use the `Q_OBJECT` macro. Furthermore, `slots` is another macro declared to do nothing, but in contrast to `emit` it's used by the MOC to generate introspection code.  _Slots-function_ can be specified as public, protected and private; Signals are always public.

Did I mention that the system is type safe? The signal's and slot's signature must find a match in order to link them: the arguments must be of the same types and qualifiers and declared in the same order.  So how do we connect our signal to our slot?

#### Connect

We'll use the static function [QObject::connect()](http://doc.qt.io/qt-5/qobject.html#connect):

```cpp
MySender sender;
MyReciever receiver;

connect(&sender, &MySender::signalName, &receiver, &MyReciever::slotName);
```
`QObject::connect()` takes five arguments, the last one has a default value and will be ignored for now. From left to right:

* Start with the object which emits the signal - the sender.
* Next we have a pointer to a member function - the signal.
* Hook it up to the listener - the receiver.
* Last, we have another pointer to a member function - the slot.

Now, if we emit out signal, the slot will be notified and we get the following output:

> Important data that will be sent to all listeners 42 1.618033

You may connect many different signals to the same slot, or use the same signal for many different slots. It's your choice - the system is very flexible. As mentioned, the system even allows to connect functions and lambdas directly, without using a listener, e.g:

```cpp
connect(&sender, &MySender::signalName, [](const QString& use, int different, float types) {
  ...
}));
```
The connection is automatically disconnected if either the sender or the receiver becomes deleted. However, as you might have guessed it's also possible to manually disconnect it by using [disconnect](http://doc.qt.io/qt-5/qobject.html#disconnect-5):

```cpp
disconnect(&sender, &MySender::signalName, &receiver, &MyReciever::slotName);
```
<div></div>
#### Heads-up when capturing data in a lambda
Be extra careful when capturing data using lambdas or functors as only the sender will automatically control the lifetime of the connection.  Consider the following case:

```cpp
connect(&sender, &MySender::signalName, [&data](Param p) {
  p.use(data);
}));
```
What if `data` has been deleted prior to emitting the signal? The connection might still be alive (if it hasn't been manually disconnected) and we'll have a naughty bug in our application. However, Qt provides another overload of the `connect()` function where it's possible to provide a __context object__. This is used like so:
```cpp
connect(&sender, &MySender::signalName, &data, [&data](Param p) {
  p.use(data);
}));
```
Notice the third argument. We're now also providing the same captured object as a context object. If the context object is deleted, the connection will automatically disconnect. Note that the context object has to inherit from `QObject` for this to work.

We've now come to an end of the basics. There should be enough information here to cover the most common cases. But down the development road you might hit some issues and will need some additional information in the future. Perhaps the next section will help you at those times.

### Tips and tricks

* In case you have multiple overloads of a signals or slots you'll need to perform a cast within the `QObject::connect()`, e.g.:

```cpp
//For C++14, from Qt5.7
connect(&sender, qOverload<int>(&MySender::overloadedSignal), &receiver, qOverload<int>(&MyReciever::overloadedSlot);

//For C++11, from Qt5.7
connect(&sender, QOverload<int>::of(&MySender::overloadedSignal), &receiver, QOverload<int>::of(&MyReciever::overloadedSlot);
```

* Although it should be avoided, it's possible to delete a listener (or schedule it for deletion) within its slot by using [deletelater()](http://doc.qt.io/qt-5/qobject.html#deleteLater).

* There are several different types of connections. The type is defined by the fifth argument in the `connect()`-function. The type is an enum called [Qt::ConnectionType](http://doc.qt.io/qt-5/qt.html#ConnectionType-enum) and is defaulted to `Qt::AutoConnection`. In a single threaded application, the default behaviour is for the signal to directly notify the listeners. However, in a multi threaded application, usually the connections between the threads are queued up on each respective [event loop](https://www.cleanqt.io/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-1). Threads are outside the scope of this series but is very well [documented by Qt](http://doc.qt.io/qt-5/threads-qobject.html#signals-and-slots-across-threads). 

* It's possible to connect a signal directly to another object's signal. This can be used, for example, to forward signals:

```cpp
connect(&domino, &Domino::tumbled, &secondDomino, &Domino::tumbled);
```

* You might find a different syntax that's used for the `connect()` function by other tutorials, blogs or even the Qt documentation, see code below. This style was the main syntax used up until Qt5. The syntax shown in this post provides [several benefits](https://wiki.qt.io/New_Signal_Slot_Syntax) over the old style. Personally, I believe the best advantage is that it allows compile-time checking of the signals and slots, which wasn't previously possible. However, in [some cases](http://doc.qt.io/qt-5/signalsandslots-syntaxes.html) the old syntax must be used, for example when connecting C++ functions to QML functions.

```cpp
connect(sender, SIGNAL(valueChanged(QString, QString)), receiver,  SLOT(updateValue(QString)));
```
* Do you remember the `slots` keyword above? It's actually only necessary to define when using the old `connect()` syntax mentioned in the previous point. The type checking for the old syntax is done at run-time by using type introspection. The MOC  generates the introspection code, but only if the `slots` keyword is defined.

### Back to the game architecture
You might now have a better understanding on how signals and slots can be used to separate the different game components. For example, let's say that we define four components which should be decoupled: the _MouseComponent_, the _UnitComponent_, the _AIComponent_ and lastly the _EnemyComponent_. Furthermore, we'll use signals in each component to communicate and notify the other components. However to solve the separation, we'll have to introduce another component, the _MainController_, which will be used to create all the connections and define the game flow. The _MainController_ will obviously need to depend on the different components, however the components are now well isolated from each other and we've achieved our goal. Do you see how this can be used in a GUI application to separate the visuals and user interaction from the actual logic?

Lastly, if you're interested in reading more about the inner-works of the signals and slots mechanism woboq.com has written a great blog series which has [got you covered](https://woboq.com/blog/how-qt-signals-slots-work.html). Let me know if something is unclear or perhaps if I missed something out.

See you next time!