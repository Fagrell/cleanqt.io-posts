# Crash course in Qt for C++ developers, part 1

__Blog post published on [cleanqt.io](www.cleanqt.io)__

In a way to encourage more people to start using Qt I've decided to kick off yet another series: a crash course in Qt for C++ developers. The [Qt documentation](http://doc.qt.io) and the [wiki](https://wiki.qt.io/Main) is extremely well composed and contain a lot of information on how to get started and is an excellent resource when used as a reference manual. However, what I found lacking are the answers to the following questions "what is the gist of Qt?"; "what is the 20% that you need to know to develop 80% of the application?"; "what are the perhaps [unknown unknowns](https://youtu.be/GiPe1OiKQuk)?". The series aims to answer these.

In this series the following topics will be covered, each in a separate post.

1. Events and the main event loop
2. Meta-object system (including QObject and MOC)
3. Signals and slots - communication between objects
4. Hierarchy and memory management
5. MVC or rather model/view and delegate programming
6. Choose your camp Quick/QML-camp or Widgets-camp
7. Tooling, e.g. Qt Creator
8. Remaining good-to-know topics
9. Where to go from here?

The topics above have been carefully chosen based on what I believe is the core of Qt and perhaps the main unknown unknowns for C++ developers. The topics have also been tailored based on feedback that I've received when asked around on different Qt/C++ forums as well as from friends. That being said, if there is any topic that is missing and you believe it should really be part of the list above, let me know in the comments.

### What is Qt and why would you want to use it?
Before we jump into the first point, let's talk about what Qt is and why you would want to use it. I'm going to be lazy and shamefully copy-paste my description, with some adjustments, on what Qt is from the about page:

Qt (usually pronounced as "cute") is a collection of many C++ libraries and tools which drastically reduce the complexity of creating applications (especially GUI applications) on a range of different platforms. A few of the supported platforms are Linux, macOS, Windows, Android and iOS. You'll most likely be surprised how easy and quickly it is to design and implement a GUI application using Qt compared to writing it from scratch, or perhaps even compared to using other tool-kits. However, not only is Qt very impressive for its GUI capabilities, but it also excels in other areas for cross-platform development such as networking, databases and OpenGL. Implementing these from the get-go will most likely be very time-consuming.

In addition to C++, Qt includes an amazing extension language called QML. QML is part of the Qt Quick module, and is a declarative object description language to create dynamic and fluid UIs on mobile, embedded and desktop systems. By incorporating QML into the development, there will be a natural separation of the project for the front-end and the back-end. The front-end consists of the look and the design of the application as well as the user interaction, which is usually mainly written in QML. The logic and the data belongs to the back-end and can be implemented using C++. Although this separation is very useful, QML also integrates JavaScript which enables developers to write applications in QML alone.

### Getting started
Hopefully, you now have a better understanding of what Qt is and why it might be a good idea to learn more about it. Perhaps you feel excited about getting started and just want to explore some examples. This is nicely covered by the [Qt documentation](http://doc.qt.io/qt-5/gettingstarted.html) and  is outside  the scope of this series.

Even though you might want to skip the getting started part, I strongly recommend to reconsider it. I believe the best approach to understand the concepts in the series is to play around with some of the provided code examples. If you're feeling brave you could even jump straight into the examples provided by Qt. Qt's examples are excellent and will give you a good idea how easy it is to use and what's possible to do.

OK enough preambles, let's start the series!

### Events and the main event loop

When working with Qt it's important to understand the flow of the application. If the flow is not taken into account when developing, the user experience will eventually be negatively affected: the animations might be slow; the communication with a socket might get interrupted; the GUI doesn't react to user interactions. Those hiccups should be avoided. So what is the flow of a Qt application?

Similarly to many other GUI frameworks Qt is __event-driven__. This means that the flow of a Qt application is determined by events which could come from both within the application itself as well as externally. Example of events are user interactions such as mouse clicks or key presses, communication with other threads or perhaps a message from the operating system or another process.

An __event__ in Qt represents something important that occurred. All events in Qt inherit from the [QEvent](http://http://doc.qt.io/qt-5/qevent.html) class. Examples of those are [QKeyEvent](http://doc.qt.io/qt-5/qkeyevent.html) and [QMouseEvent](http://doc.qt.io/qt-5/qmouseevent.html) which are, wait for it,  events generated by key and mouse actions respectively. The event has a __target__ object which will handle the event and perhaps perform one or multiple actions.

To avoid interrupting the application flow, the target object doesn't receive the event as soon as it's generated. The event will instead be pushed back into an __event queue__. An __event loop__ will process the event queue and dispatch them to their respective target objects. A very simplified event loop looks like this:

```cpp
while (true) {
  dispatchEventsFromQueue();
  waitForEvents();
}
```

* `dispatchEventsFromQueue()` will loop through each event in the queue until it's empty.
* `waitForEvents()` will block the current thread until any external events occur, perhaps from another thread or a window manager activity.

If you've had the time to look through any of the examples provided by Qt you might have stumbled across the main function. The main function in most Qt applications looks similar to this:

```cpp
#include <QCoreApplication>

int main(int argc, char* argv[]) {
 QCoreApplication app(argc, argv);
 // ... instansiate resources using app ...
 return app.exec();
}
```
Qt's __main event loop__ is entered by running the `exec()` member function on an instance of a [QCoreApplication](http://doc.qt.io/qt-5/qcoreapplication.html), a [QGuiApplication](http://doc.qt.io/qt-5/qguiapplication.html) or a [QApplication](http://doc.qt.io/qt-5/qapplication.html). The differences between the three will not be covered in this post but feel free to explore them on your own. The termination of the loop will be performed when `exit()` or `quit()` is called on the `app` object.

The main event loop needs to be running in order to paint the GUI widgets as well as to interact with them. Without the loop, none of the events would be dispatched and the application wouldn't work as intended. The loop is also required when using other non-GUI components such as timers or all low-level network classes, e.g. [QTcpSocket](http://doc.qt.io/qt-5/qtcpsocket.html) and [QUdpSocket](http://doc.qt.io/qt-5/qudpsocket.html). The actual reading and writing in those classes are asynchronous and only take place in the event loop. What do you think happens if the main event loop takes a long time to dispatch an event?

Imagine that a user presses a button in your application resulting in a `QMouseEvent` which is pushed back to the event queue. The event loop will eventually dispatch that event to a target object. In case the target object performs an expensive (i.e. slow) operation the event loop will be blocked until that operation is completed. You can probably visualise what would happen in this case: widgets won't update and the GUI would be locked. But what if you have to do an expensive operation; what are your options?

The best solution is to move the expensive task into a separate thread. The thread will enter another event loop and communicate with the main thread by using signals and slots, which will be covered in a future post. Threading is outside the scope of this project but it is very well covered by the [qt documentation](http://doc.qt.io/qt-5/thread-basics.html).

Another method to handle time-consuming calculations is to force event dispatching by, for example, manually and repeatedly calling `QEventLoop::processEvents()` within the calculations themselves. However, this option is not recommended as it's difficult to identify how often `processEvents()` should be called in order to maximise usability and performance as it will depend on the hardware.

Hopefully by now you have a basic understanding of how the event system works in Qt and how to avoid event loop blocking. If you're interested in reading more about Qt's event system have a look at the [official documentation by qt](https://wiki.qt.io/Threads_Events_QObjects#Events_and_the_event_loop) as well as an [old, but still very informative post](https://doc.qt.io/archives/qq/qq11-events.html), by Trolltech.






