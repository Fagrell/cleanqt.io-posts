# Crash course in Qt for C++ developers, part 5

__Blog post published on [cleanqt.io](www.cleanqt.io)__

Have you ever heard of [__M__odel-__V__iew-__C__ontroller](https://en.wikipedia.org/wiki/Model-view-controller), or MVC for short? If not - not to worry - as you might have never worked with a GUI toolkit before. But perhaps you've heard of the SoC principle, i.e. [__S__eparation __o__f __C__oncerns](https://en.wikipedia.org/wiki/Separation_of_concerns)? Essentially MVC is a design pattern used in GUI applications which follows the SoC principle by isolating and decoupling different components. In other words, each component: the Model, the View and the Controller are addressing different concerns, which give developers a greater flexibility and reusage. MVC originates from smalltalks, and was formulated by [Trygve Reenskaug](http://heim.ifi.uio.no/~trygver/themes/mvc/mvc-index.html). Although the design pattern is rather old, I dare say that most GUI toolkits rely on a variant of it, such as [MVP](https://en.wikipedia.org/wiki/Model-view-presenter) and [MVVM](https://en.wikipedia.org/wiki/Model-view-viewmodel). Qt's MVC variant is called the __Model/View__ which plays a central role in any data-driven Qt application. This post will discuss the importance of this pattern and how it's implemented in the Qt framework.

This is the fifth post in the series "Crash course in Qt for C++ developers" covering the __MVC or Model/View programming__ in Qt. The other topics are listed below.

1. [Events and the main event loop](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-1)
2. [Meta-object system (including QObject and MOC)](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-2)
3. [Signals and slots - communication between objects](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-3)
4. [Hierarchy and memory management](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-4)
5. [MVC or rather model/view and delegate programming](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-5)
6. [Choose your camp Quick/QML-camp or Widgets-camp](/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-6)
7. Qt Quick/QML example
8. Qt Widgets example
7. Tooling, e.g. Qt Creator
8. Remaining good-to-know topics
9. Where to go from here?

### What? Why?
Perhaps you didn't understand any of that. That's absolutely fine! We're just about to explore what it is and why it's useful. Let's start with the purpose - what problem is MVC solving? Obviously, a non-trivial application needs a good architecture to be scalable. And many good architectures are fundamentally based on the SoC principle. By incorporating MVC, the programs are more adaptable to an ever changing specification and can easily be tested and extended with additional functionality. These capabilities are core ingredients in a good architecture formula. Let's see what those concerns are in the MVC pattern. The three components consist of:

* __The Model__ which is the core of MVC. It handles the data and logic independently from the user interface.
* __The View__ which is the screen representation and renders the model data. Basically the visual components.
* __The Controller__ which handles user inputs and translate those to interactions on the model. 

By using this separation, it's easy to change the presentation and the visuals without changing the underlying logic. It's also simple to write unit tests targeting the logic without involving the UI. Another benefit of this design is that the same model can be used for multiple views simultaneously without synchronising data. Note that there are many variants of the MVC architecture which can be significantly different from the aforementioned description. [Some GUI frameworks](https://developer.apple.com/library/archive/documentation/General/Conceptual/DevPedia-CocoaCore/MVC.html) even use MVC as the core architecture for the whole application whereas in Qt, it is only used to present one data structure. A typical example is to visualise data from a database in a table. Let's explore how that works.

### Qt's Model/view 

Similarly to MVC, Qt's __Model/view__ design pattern is essentially separated into three components:

* The __Model__, which can either work as an [adaptor](https://en.wikipedia.org/wiki/Adapter_pattern) to the data, or store the data directly. It defines an interface which is used by the view and delegate to access data. The interface is designed in such a way that the data or the elements can be presented as a list, table or tree. 
* The __View__, which is used to define how all the elements in the model are arranged e.g. in a tree? In a table?   
* The model and view are joined by the  __delegate__ which is used to define the presentation of each individual element in the data. It also handles the editing of an element.

<center>
<img style="max-width:100%; max-height:100%; width:250px;" alt="Clean Qt Model/view" src="../../../assets/blog_images/model-view.svg">
</center>

You might wonder where MVC's Controller comes in, in this design. The controller is essentially merged into the view and the delegate of the model/view design as they are both receiving user inputs. It's also worth pointing out that both the delegate and the view in Qt's design act as the view in the traditional MVC description: they are both responsible for the presentation of the elements. Qt's design is quite different from MVC but the core concept is still there, the representation of the data is separated from the presentation of it. 

This might still be very confusing, and is perhaps best understood with a simple example.

### Basic example
Before we jump into the example below, I want to point out that it specifically covers _model/view_ using the [Qt Widgets](http://doc.qt.io/qt-5/qtwidgets-index.html) module. That said, similar behaviour and techniques can be done by using the [Qt Quick](https://doc.qt.io/qt-5.11/qtquick-index.html) module and QML. By focussing on _Qt Widgets_, I believe the concepts discussed here will be easier to grasp for someone who's coming from a C++ background. However, [this tutorial](http://doc.qt.io/qt-5/qtquick-modelviewsdata-modelview.html) from Qt and [this chapter](https://qmlbook.github.io/en/ch06/index.html#) from the QML book are great reads should you be interested in using _model/view_ with _Qt Quick_. 

OK, let's now start with the code. 

```cpp
int main(int argc, char *argv[]) {
  QApplication app{argc, argv};
  QSplitter splitter{Qt::Vertical};

  auto model = new QStringListModel{&app};
  model->setStringList(QStringList{"Gandalf", "Aragorn", "Legolas", "Samwise Gamgee" ,"Gimli", "Bilbo Baggins", "Peregrin Took", "Boromir"});
  auto combo_box_view = new QComboBox{&splitter};
  combo_box->setModel(model);

  auto list_view = new QListView{&splitter};
  list->setModel(model);

  splitter.show();
  return app.exec();
}
```
<br/>
#### The View
The example creates two presentations using one set of data. The [QSplitter](http://doc.qt.io/qt-5/qsplitter.html) is not part of the _model/view_ pattern but is used here to display the two views at the same time. 

The two presentations, or rather, the views are:

* [QComboBox](http://doc.qt.io/qt-5/qcombobox.html) which displays a dropdown list when it's selected. Each element acts as a button and emits a signal when clicked.
* [QListView](http://doc.qt.io/qt-5/qlistview.html) which displays the data in a list, similarly to a table with only one column. Each element can be selected and edited.
<center>
<img style="max-width:100%; max-height:100%; width:500px;" alt="Clean Qt Model/view example" src="../../../assets/blog_images/qt-model-view-example.png">
</center>

Those views are among some of the ready-made ones provided by Qt. Some other commonly used classes are [QTableView](http://doc.qt.io/qt-5/qtableview.html) and [QTreeView](http://doc.qt.io/qt-5/qtreeview.html). They are each based on the [QAbstractItemView](http://doc.qt.io/qt-5/qabstractitemview.html) abstract base class. The `QAbstractItemView` is essentially an interface that can be subclassed and utilised in order to detect when changes are made to the data in the model. The communication between the view and the model is done using the signals and slots mechanism which enables the model to be fully decoupled from the view. This mechanism was previously covered in [another post in the series](https://www.cleanqt.io/blog/crash-course-in-qt-for-c%2B%2B-developers,-part-3). 

Imagine you would now edit one of the elements in the list. What do you think would happen to the data in the combo box? Let's give it a go!

In order to edit one of the elements, select it in the list and start typing. We have now changed the data in the model used by the `QListView`. In this instance, I've changed _Gandalf_ to _Gandalf the White_ and _Boromir_ to _Faramir_. By selecting the combo box we can now inspect all the elements:
<center>
<img style="max-width:100%; max-height:100%; width:250px;" alt="Clean Qt Model/view example" src="../../../assets/blog_images/qt-combo-box-example.png">
</center>
<br/>

Great! It has automatically detected that updates have been made to the data and that the `QComboBox` has been notified accordingly. This works because the model is shared across both views.

#### The model
Similar to the views, Qt provides some ready-made models. The model we are using in this example is one of the simplest ones: the [QStringListModel](http://doc.qt.io/qt-5/qstringlistmodel.html) which obviously only stores a list of strings. There are many more models in Qt, such as the [QFileSystemModel](http://doc.qt.io/qt-5/qfilesystemmodel.html), the [QSqlTableModel](http://doc.qt.io/qt-5/qsqltablemodel.html) and the [QStandardItemModel](http://doc.qt.io/qt-5/qstandarditemmodel.html). I believe both `QFileSystemModel` and `QSqlTableModel` are pretty self-explanatory, however the `QStandardItemModel` is a bit more interesting to discuss. 

The `QStandardItemModel` is a generic model where custom data can be stored. Each element corresponds to an item which is created individually. By using this model it's easy to represent a simple tree, table or list structure e.g:

```cpp
static const constexpr int kTableSize{2};

// Create a table showing the OR states for two binaries
QStandardItemModel or_table_model{kTableSize, kTableSize};
for (auto row{0}; row < kTableSize; ++row) {
  for (auto column{0}; column < kTableSize; ++column) {
    auto or_gate = row || column ? "y" : "n";
    auto item = new QStandardItem{QString{or_gate}};
    or_table_model.setItem(row, column, item);
  }
}
```

This model is useful when only a few elements are needed, but will perform quite badly with a large dataset. The main reasons for this are that each element is dynamically allocated individually and the whole dataset is usually created even though only a handful are displayed at the same time. If you're using a big dataset, you'll most likely want to subclass [QAbstractItemModel](http://doc.qt.io/qt-5/qabstractitemmodel.html) or any of the other convenient classes, such as [QAbstractListModel](http://doc.qt.io/qt-5/qabstractlistmodel.html) and [QAbstractTableModel](http://doc.qt.io/qt-5/qabstracttablemodel.html). Subclassing one of those classes is a big subject on its own and won't be covered in this post. However, as usual, [Qt provides good documentation](http://doc.qt.io/qt-5/model-view-programming.html#model-subclassing-reference) which covers this. 

We have now talked about the basics regarding the _view_ and the _model_. But what happened to the delegate?

#### The delegate

You might have noticed that the basic example doesn't include a delegate. However, there is actually a default delegate attached to the `QListView`. That's, for example, why it's possible to edit one of the elements in the list. But what if we would like to customise the look and the behaviour when editing? Perhaps instead of just displaying the names in the list, we would also want to display a small icon showing each movie character. And perhaps instead of allowing each element to be changed to any text, we could limit the editing to a list of available characters. This can be done by customising the delegate. To customise the delegate you'll need to subclass a [QAbstractItemDelegate](http://doc.qt.io/qt-5/qabstractitemdelegate.html) or any of the convenient classes [QStyledItemDelegate](http://doc.qt.io/qt-5/qstyleditemdelegate.html) or [QItemDelegate](http://doc.qt.io/qt-5/qitemdelegate.html). A typical delegate subclass looks like this:

```cpp
class MyDelegate: public QStyledItemDelegate
{
  Q_OBJECT
  
public:
  MyDelegate(QObject* parent = nullptr);
  
  // Typically these two functions are needed to override if custom display rendering is desired
  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;
  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;

  // Typically these four functions are needed to override if custom editing is desired
  QWidget *createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                        const QModelIndex &index) const override;

  void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  void setModelData(QWidget* editor, QAbstractItemModel* model,
                    const QModelIndex& index) const override;

  void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, 
                            const QModelIndex& index) const override;
};
```

To add the character icon we'll need to override the [paint()](http://doc.qt.io/qt-5/qstyleditemdelegate.html#paint) and the [sizeHint()](http://doc.qt.io/qt-5/qstyleditemdelegate.html#sizeHint) functions. For more information on how this can be done, see [this example](http://doc.qt.io/qt-5/qtwidgets-itemviews-stardelegate-example.html) provided by Qt. Similarly, the functions [createEditor()](http://doc.qt.io/qt-5/qstyleditemdelegate.html#createEditor), [setEditorData()](http://doc.qt.io/qt-5/qstyleditemdelegate.html#setEditorData), [setModelData()](http://doc.qt.io/qt-5/qstyleditemdelegate.html#setModelData) and lastly [updateEditorGeometry()](http://doc.qt.io/qt-5/qstyleditemdelegate.html#updateEditorGeometry) are typically used when custom editing is required. Also, in this case, Qt has got you covered with [an example](http://doc.qt.io/qt-5/qtwidgets-itemviews-spinboxdelegate-example.html).

### Still Confused?
I fully understand if this feels a bit overwhelming and a lot to take in. __Model/view__ requires some time to get used in order to fully understand the different concepts. In addition to this, it's also quite easy to get it wrong when you're implementing your own model. If you're interested in doing just that, I would suggest to have a look at [this post by KDAB](https://www.kdab.com/new-in-qt-5-11-improvements-to-the-model-view-apis-part-1/) as it covers some of the problems that you might encounter along the way. Lastly, I strongly recommend to also go through the [official tutorial by Qt](https://doc.qt.io/qt-5/modelview.html). Good luck!
