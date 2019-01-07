# Crash course in Qt for C++ developers, part 7

__Blog post published on [cleanqt.io](www.cleanqt.io)__

By now, you might have already gone through some of the examples provided by Qt and have a rough idea on how to create a Qt application. However, most Qt examples are designed around a particular usage or to demonstrate a specific class. And often, it's unclear how to expand and scale them. The purpose of this post is to demonstrate how to structure a typical reasonable large Qt application. We'll start from scratch by setting up the file structure and later on show how a [MVC-variant](https://www.cleanqt.io/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-5) can be used to separate the core components. By the time you've read through this post, you should have a pretty good understanding on how to organise your files and components and how to scale the project into a larger application.

This is the seventh post in the series "Crash course in Qt for C++ developers" covering __How to organise and structure a Qt application__. The other topics are listed below.

1. [Events and the main event loop](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-1)
2. [Meta-object system (including QObject and MOC)](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-2)
3. [Signals and slots - communication between objects](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-3)
4. [Hierarchy and memory management](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-4)
5. [MVC or rather model/view and delegate programming](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-5)
6. [Choose your camp Quick/QML-camp or Widgets-camp](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-6)
7. Qt Quick/QML example (including how to organise and structure)
8. Qt Widgets example (including how to organise and structure)
7. Tooling, e.g. Qt Creator
8. Remaining good-to-know topics
9. Where to go from here?

This post kicks off a mini series consisting of three parts, which are all based on an example application. The first part covers the structure and design of said application. The following two posts will discuss two front-ends using different technologies: one with Quick/QML and another using Widgets. I recommend to read through the posts and subsequently try to recreate the application from scratch. I believe this is a good way to learn and will enforce you to possibly make mistakes along the way and then learn from them. In addition, it might be helpful to poke around a bit in the code while reading through. The example application is a simple code editor and the source code can be found [on GitHub](https://github.com/Fagrell/qml-editor).

Before we jump into the design, it also goes without saying that there are many different ways to shape the architecture of a Qt application. The solution presented here is only one of many, and it's most likely not a perfect fit for all domains. However, I do want to empathise that it is important to consider the structure and architecture early on in the development cycle. If there is no structure in place, the code can quickly become unmanageable and difficult to scale. Similar to software development processes, it's less important which one followed, as long as one is adopted. 

Let's now dive into the example application. We'll begin with the file structure: 

### File structure

```
clean-editor (root)
├── CMakeLists.txt
├── lib
|   ├── CMakeLists.txt
│   ├── public
│   │   ├── controllers
│   │   ├── models
│   │   └── ...
│   ├── src
│   │   ├── controllers
│   │   ├── models
│   │   └── ...
│   └── tests
│        ├── CMakeLists.txt
│        └── src
├── ui-qml
│   ├── CMakeLists.txt
│   └── ...
└── ui-widgets (not implemented yet)
    ├── CMakeLists.txt
    └── ...
```     

As you can see, there are three folders in the root directory, each containing a project: The _lib_-directory, which is the logic and data behind the application, and two GUI front-ends: _ui-qml_ and _ui-widgets_. By separating the logic and data into an isolated unit, there will be a clear line drawn between the UI and the logic. There are several benefits to this approach, e.g.:
* The library is easy to test and verify since no GUI elements are involved.
* The library can be shared with multiple applications using different technologies (e.g. different programming languages) and interfaces (e.g. GUI and CLI).
* The library can be loaded dynamically by the application. This allows modifications to the library without needing to recompile the application, i.e. allowing a drop-in replacement. Obviously, this only works if the ABI is the same between builds.

Likewise, by digging into the _lib_-directory from the _root_, we can find three different directories: _public_, _src_ and _tests_. The _public_ directory includes all the headers files exposed by the library and used externally by the front-ends. The _src_ includes the remaining header files as well as all the source files. Not surprisingly, tests are found in the _tests_ directory. 

Those tests are part of a standalone application and are used to verify the public interface. This design encourages [black-box testing](https://en.wikipedia.org/wiki/Black-box_testing) as tests can be written without the source code, but simultaneously allows [white-box testing](https://en.wikipedia.org/wiki/White-box_testing) as it's part of the same project. The tests in the example application use [QTest](http://doc.qt.io/qt-5/qtest.html) but can easily be replaced by [Google Test](https://github.com/google/googletest), [Catch2](https://github.com/catchorg/Catch2) or [Boost.Test](https://www.boost.org/doc/libs/1_60_0/libs/test/doc/html/index.html) or whatever your favourite test framework is.

By drilling further down the file structure into the _public_ and _src_ directories, we can here find _controllers_ and _models_. Those two directories contain - wait for it - the different model and controller classes. They comprise of the flow, the logic and the data of the application. The concept _MVC_ has previously been covered in a [post](https://www.cleanqt.io/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-5) and won't be discussed here in details. However, if you haven't already, I would recommend to read it through before moving on to the next section as we're about to dive into the architecture.

### Overview of the example application

In order to understand how the architecture is implemented, let's start by looking at a screenshot of the application:

<center>
  <img src="screenshot.png?raw=true" width="600">
</center>
<br/>

The application consists of one window, or rather one view called the __Main View__. The view is composed of three components: The menu at the top, the file navigation to the left and lastly, the editor to the right. This allows a modular design where a view can be assembled using a set of components. And if it isn't obvious, the benefit of this design is that those components can be reused between views. 

### Model-View/Component-Controller
Each component and each view is connected to a single controller. The controller receives user-interactions and manages the flow of the application by forwarding calls and signals to the relevant model(s). Note that models might be shared between the components and views. The components subscribe to changes made on the model(s) in order to correctly present the current state and data. See the dependency diagram below:

<center>
  <img src="view-component-controller-model.png?raw=true" width="500">
</center>
<br/>

In the example application, the __Main View__ is hooked up to the `MainController` which handles the communication between all the components and their respective controllers: the `MenuController`, the `FileNavigationController` and the `EditorController`. One of those components is the __Menu Component__ which is using the `MenuController`. The `MenuController` recieves the user-interactions and routes the actions to the `MenuModel` or emit signals to the parent subscriber, i.e. the `MainController`. The __File Navigation Component__ and the __Editor Component__ both work in a similar manner as the __Menu Component__. 

Confusing? Perhaps an example might help.

### MenuModel/MenuController example - Show me some code!
To better understand the dynamic of this, let's look at the `MenuModel`/`MenuController` pair and how they interact. Let's assume that the front-end forwards the user-interactions to the controller and updates when changes are made to the model. Let's look at the model first:

```cpp
class QML_EDITOR_EXPORT MenuModel : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY(MenuModel)

  Q_PROPERTY(QString title READ title NOTIFY titleChanged)
  Q_PROPERTY(bool isNewFile READ isNewFile NOTIFY isNewFileChanged)

public:
  explicit MenuModel(QObject* parent = nullptr);

  void setDocument(CleanEditor::Logic::DocumentHandler* document_handler);

  QString title() const;
  bool isNewFile() const;

Q_SIGNALS:
  void titleChanged();
  void isNewFileChanged();

private:
  QPointer<CleanEditor::Logic::DocumentHandler> document_handler_;
};
```

We have here defined two properties in the model which are exposed to the front-end:
* The `title`, representing the name of the currently opened document, see _file-handler.cpp_ in the screenshot above.
* The `isNewFile`, which states whether the currently opened document is new, i.e. not opened and/or not saved. This is used by the UI to decide if a _save file_-dialog should be shown when saving. The dialog is only needed if the file is new (or if _save as_ is requested).

The `MenuModel` uses the currently opened document, a `DocumentHandler`, in order to detect changes and to store the aforementioned data. The implementation of `setDocoument()` looks like this:

```cpp
void MenuModel::setDocument(DocumentHandler* document_handler) {
  if (document_handler_ == document_handler) {
    return;
  }

  if (document_handler_) {
    disconnect(document_handler_.data(), &DocumentHandler::fileUrlChanged, this, &MenuModel::titleChanged);
    disconnect(document_handler_.data(), &DocumentHandler::isNewFileChanged, this, &MenuModel::isNewFileChanged);
  }

  document_handler_ = document_handler;
  if (!document_handler_) {
    return;
  }

  connect(document_handler_.data(), &DocumentHandler::fileUrlChanged, this, &MenuModel::titleChanged);
  connect(document_handler_.data(), &DocumentHandler::isNewFileChanged, this, &MenuModel::isNewFileChanged);

  emit titleChanged();
  emit isNewFileChanged();
}
```

As you can see, the relevant signals in the `DocumentHandler` are forwarded to the model which will in turn notify any listener, i.e. the __Menu Component__ in this scenario. The `setDocument()` is invoked by the controller `MenuController`, so let's look at it next.

```cpp
class QML_EDITOR_EXPORT MenuController : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY(MenuController)

public:
  explicit MenuController(QObject* parent = nullptr);

  //Takes ownership of model
  void setModel(CleanEditor::Models::MenuModel* model);

  void setDocument(CleanEditor::Logic::DocumentHandler* document_handler);

Q_SIGNALS:
  void newFileClicked();
  void openFileClicked(const QUrl& file_url);
  void saveFileClicked();
  void saveAsFileClicked(const QUrl& file_url);

private:
  QPointer<CleanEditor::Models::MenuModel> model_;
};
```

As you might have guessed, each signal corresponds to a user-action performed in the menu area: `newFileClicked()`, `openFileClicked(...)`, `saveFileClicked()` and `saveAsFileClicked(...)`. Perhaps you also observed that only three icons are available in the menu? This is because the `saveFile` action doesn't have a dedicated icon; it's emitted when using a keyboard shortcut instead.

The `MenuController`'s signals are not handled directly within the `MenuController` but propagated up to the `MainController`. The `MainController` then redirects the flow to a `DocumentsModel` which handles the actions for creating, opening and saving files. The reason the `MenuController` doesn't directly own and write to the `DocumentsModel` is because this particular model is shared between several components and should preferably only be handled by one controller, namely the parent `MainController` . I've found this constrained to be very helpful in order to understand and track the flow of the application. It's especially helpful when debugging.

Perhaps you noted that the `model` is created outside the controller and then passed in through the `setModel`-function. The main reason for this, is that the model could potentially be swapped up for another one. By using the [dependency inversion principle](https://en.wikipedia.org/wiki/Dependency_inversion_principle) we can decouple the components even further and use different models for the same controller. This technique is utilised in the `EditorController` where we need two different models in order to use it for both the QML component and the C++ widget:
```cpp
class QML_EDITOR_EXPORT EditorController : public QObject {
...
  //Takes ownership of model
  void setModel(CleanEditor::Models::AbstractEditorModel* model);
...
};
```

As the name implies, the `AbstractEditorModel` is an abstract class.

### Wrap up
Similar to the previous _MVC_ post this migt feel a bit overwelming at first. However, we'll come back to the example application in the next two posts and it will hopefully make more sense by then. That said, I would suggest you to go through the remaining components in the source code and investigate a bit. Possibly you could add another component and see how it would fit into the design?

If you're interested in more information on how to structure a Qt application, I would recommend to try to get hold of the [Learn Qt 5 book]( https://www.packtpub.com/web-development/learn-qt-5) written by Nicholas Sheriff. The book goes through a full example application created from scratch and incorporates similar techniques as described in this post. In addition to this, if you're interested in adapting Facebook's Flux design, [Benlau's quickflux has got you covered](https://github.com/benlau/quickflux). 

In the next post we'll cover the QML front-end for the example application in details. We'll learn more about the QML syntax and semantics and how it communicates with the C++-backend. We'll also explore how we can structure it in a scalable manner. See you next time!