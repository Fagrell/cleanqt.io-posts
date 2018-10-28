# Crash course in Qt for C++ developers, part 6

__Blog post published on [cleanqt.io](www.cleanqt.io)__

We have now come to a point in the learning journey when we have to make an important decision. We've approached a junction that can lead us to two different paths. Which path we decide to follow will depend on our requirements and desired look and feel of the application. It will most likely be very costly and time consuming to turn back and change path half way through. We have to choose between the Qt Quick/QML-path and the Qt Widgets-path. This post will cover the differences between the two and guide you through when one might be preferred over the other in different scenarios.

This is the sixth post in the series "Crash course in Qt for C++ developers" covering the __Choose your camp Quick/QML-camp or Widgets-camp__ in Qt. The other topics are listed below.

1. [Events and the main event loop](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-1)
2. [Meta-object system (including QObject and MOC)](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-2)
3. [Signals and slots - communication between objects](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-3)
4. [Hierarchy and memory management](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-4)
5. [MVC or rather model/view and delegate programming](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-5)
6. Choose your camp Quick/QML-camp or Widgets-camp
7. Qt Quick/QML example
8. Qt Widgets example
7. Tooling, e.g. Qt Creator
8. Remaining good-to-know topics
9. Where to go from here?

Essentially, both Qt Widgets and Qt Quick/QML are two different technologies that are used to create and design [user interfaces](http://doc.qt.io/qt-5/topics-ui.html). Despite what one might conceive - after reading the preamble - it's possible to incorporate both technologies within a single application. One such application is the [Qt Creator](http://doc.qt.io/qtcreator/index.html). However, as previously mentioned, whenever you decide to use one technology for a certain area, it's difficult to change your mind subsequently. It's not just a drop in replacement. Therefore, it's good to know what the differences are between the two.

To begin with, if you're interested in mobile development alone, forget about Qt Widgets. Just use Qt Quick. Qt Widgets are specifically designed to be used in a desktop environment and the UI controls have been designed to interact with a mouse and a keyboard. However, Qt Quick also works well in a desktop environment and is arguably therefore more useful to master. So let's jump straight into it first and then afterwards explore Qt Widgets.

### Quick/QML
[Qt Quick](https://doc.qt.io/qt-5.11/qtquick-index.html) provides the foundation types when designing user interfaces with QML and was released at the end of 2010. QML (__Q__t __M__odeling __L__anguage) is a declarative object description language that is used to create dynamic and fluid UIs. A QML document is, similarly to [HTML](https://en.wikipedia.org/wiki/HTML), a description of an hierarchical object tree. The code structure looks very similar to a definition of a [JSON](https://en.wikipedia.org/wiki/JSON) object. QML objects can be styled, similarly to [CSS](https://en.wikipedia.org/wiki/Cascading_Style_Sheets), and JavaScript code can be inlined to handle assertive aspects. If you're familar with front-end web developement you'll grasp QML in no time.  This is how a _hello world_-example looks like in QML:

```c
Rectangle {
  width: 640
  height: 480
  color: "blue"

    Text {
      anchors.centerIn: parent
      text: "Hello World!"
    }
}
```
The example will draw a blue rectangle and display the text "Hello World!". I'm not going to cover further details about the aforementioned example, since a full example application will be shown in the next post. Perhaps I should now mention, before I scare you off from ever getting near Qt Quick, most Qt Quick projects use a combination of QML and C++.  And as we'll see in the next post, it's very easy to pass information between them. However, one might wonder why Qt decided to introduce another language instead of just keep using C++? I have a couple of theories regarding this.

Firstly, by incorporating QML into the development, there will be a natural separation of the project for the front-end and the back-end. The front-end consists of the look and feel of the application as well as the user interaction, which can be written in QML alone. QML enables a quick tool for prototyping and the language is most likely easier to understand for a designer than a low-level language such as C++. That said, as mentioned in the previous section, it's easy to combine QML and C++ in a project. The performance critical aspects of the software usually belongs to the back-end, which make C++ a good candidate in this part of the stack. However, the back-end is also much less relevant for a designer.

Secondly, it's effortless to bind data between different objects in QML. If you for example require a rectangle to always be twice as wide as another rectangle, it's as simple as setting `width = 2*rectangle2.width`. The first rectangle will automatically and dynamically update its width depending on the second rectangle's width. This binding can be chained between different objects so that one change can trigger a lot of updates simultaneously. The system is so powerful that you can even bind a property to a JavaScript expression or statement such as `width: Math.min(rectangle2.width, rectangle2.height)`. Recreating this behaviour in C++ (or rather Qt Widgets) will most likely require a lot of boiler plate code. It's also perhaps more difficult to get the full picture of the UI structure  when using a procedural code compared to a declarative language.

Through-out the years, Qt Quick/QML has specifically been tailored for mobile touchscreen devices, and supports features that are expected from such devices. Some of these features are touch-friendly controls, animated transitions and graphical effects such as a [particle system](http://doc.qt.io/qt-5/qtquick-effects-particles.html). Even though the system has been heavily designed for the mobile world, it is easily adaptable to a desktop environment. For example, the ready-made controls come with the [fusion style](http://doc.qt.io/qt-5/qtquickcontrols2-fusion.html), which is a platform-agnostic style that offers near-native desktop look and feel.

Speaking of controls, you'll find two different controls in Qt Quick: Quick Controls [1](http://doc.qt.io/qt-5/qtquickcontrols-index.html) and [2](http://doc.qt.io/qt-5/qtquickcontrols2-index.html). The main differences between the two are that the first version of Qt Quick Controls are implement in QML alone whereas the second one is a combination of both QML and C++. This enables the Controls 2 to [perform much](https://appbus.wordpress.com/2016/04/06/qt-quick-2-qt-labs-controls/) better than 1. In addition, starting from Qt 5.11 the Quick Controls 1 will be deprecated. For this reasons, I would suggest to focus on the second version and forget about the first. However, if you insist on using Quick Controls 1 or perhaps a combination of both, a full comparison between the two can be found on [this Qt page](https://doc.qt.io/qt-5.11/qtquickcontrols2-differences.html).

As you now have learnt, QML is a language that is very different from C++. Using Qt Quick therefore comes with the hurdle of learning another language. There might be a natural biased towards using Qt Widgets as it will most likely feel more closer to a C++ developer's heart. However, I encourage you to give Qt Quick/QML a chance. QML comes with a gradual learning curve and it's really a delight to use when you become familiar with it. That said, there are still some use cases for Qt Widgets. Let's see what those are.

### Widgets
The [Qt Widgets module](http://doc.qt.io/qt-5/qtwidgets-index.html) contains a set of UI components which offer a desktop-oriented native look. The module is essentially a library that contains [a range of widgets](https://doc.qt.io/qt-5.11/qtwidgets-module.html#details) for almost all standard graphical desktop components. Those widgets are usually rather static and contain very few animations. For these reasons it will be easier to design a native looking application on desktop environments compared to doing it with Qt Quick. Although as previously mentioned, it's gradually becoming easier to achieve this with Qt Quick by applying the fusion style. A minimal hello example using Qt Widgets looks like this:

```cpp
int main(int argc, char *argv[]) {
  QApplication app{argc, argv};
  QWidget window;
  new QLabel{"Hello World!", &window};
  window.show();
  return app.exec();
}
```

Running the example will display a window with a text "Hello World!". Similarly to the Qt Quick example, a future post will cover a full example application using Qt Widgets and I won't go into more details here.

In contrast to Qt Quick, the Qt Widgets module has been around since the inception of Qt. Some developers have argued that since Qt Widgets lifetime is much longer than Qt Quick, the module must therefore be more mature and stable. This was definitely true just a few years ago, however I would argue that is not the case any more. Qt Quick is becoming more and more reliable for every new release. With the latest long term support version, I've only encountered a couple of problems down the development road and there are usually workarounds for those. However, some annoying bugs still exists, such as [QML window application resize not smooth anymore](https://bugreports.qt.io/browse/QTBUG-46074), which might be a game changer when deciding between using Qt Widgets or Qt Quick.

Some other developers have also argued that it's easier to maintain and refactor big projects based on the Qt Widgets module compared to a Qt Quick project, because of how QML is structured. I personally don't fully agree with this statement and it usually comes from developers who have worked with projects that have contained messy and unstructured QML code. If the same effort is put into keeping QML as clean as C++, I believe it will be easy to maintain a Qt Quick project. That said, there are far more tools (and static analysers) available for C++ than QML which certainly help when refactoring. 

Another difference between the two modules is how they are rendered. QWidgets are rendered using a [QBackingStore](http://doc.qt.io/qt-5/qbackingstore.html) which is a software rasteriser. Qt Quick, in contrast, is nowadays using a scene graph based on OpenGL (ES) 2.0, which provides better performance compared to the Qt widgets back-end. Even so, for most application and in everyday use, the performance difference between the two is usually not noticeable.

### Wrap up and recommendation
In summary, what I would suggest is to use Qt Widgets if you only aim to design a native looking desktop application without animations, fancy transitions or touch inputs. It will still allow you to integrate Qt Quick/QML in some parts of the UI by using [QQuickWidget](http://doc.qt.io/qt-5/qquickwidget.html). For everything else, I would recommend to use Qt Quick/QML. 

Need a modern fluid interface? Use Quick/QML.<br/>
Need a touch-friendly interface? Use Quick/QML.<br/>
Needs a language which designers can tinker with? Use Quick/QML. 

In addition, KDAB is currently adding Qt Widgets support to QML, have look at the following [GitHub project](https://github.com/KDAB/DeclarativeWidgets) for further details. Also perhaps you prefer the Qt Widgets API and don't want to touch QML, but would rather use the back-end and look and feel of Qt Quick? Check out [QSkinny](https://github.com/uwerat/qskinny) which offers a similar API to the Qt Widgets' but runs on top of the Qt Quick graphics stack.

Have you recently started a new project using either Qt Quick or Qt Widgets? What did you decide to use? Why? Let me know in the comments!
