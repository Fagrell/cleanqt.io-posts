# Qt Containers

The following post will dive into world of containers and explore the ones Qt provides and how they compare to the standard library's (STL). The post will cover the different types and which ones are unique to each library. API, performance and some internal implementation details will also be covered. The goal of the post is to identify which library might be preferred over the other.

According to Qt's wiki page about the Qt library: 
> The Qt library provides a set of general purpose template-based container classes.  

> These container classes are designed to be lighter, safer, and easier to use than the STL containers. If you are unfamiliar with the STL, or prefer to do things the "Qt way", you can use these classes instead of the STL classes.

In a talk by [Giuseppe D'Angelo](https://www.youtube.com/watch?v=uZ68dX1-sVc) he explained that there are historical reasons why the Qt containers exist: 

* Qt supported platforms that didn't provide a standard library.
* Qt didn't want to expose the STL symbols from its ABI.

However, since Qt5, it's required to use a working STL implementation.


## Container types

Let's start by looking into the available types for each library. The following tables list the Qt containers and the STL counterparts for each container type: sequence containers, container adaptors, associated containers (ordered and unordered) and lastly classes which resemble containers. As you will note, some containers are only available in Qt and also the other way around is true. Each section will cover the unique Qt types as well as some additional information.

### Sequence containers:

| Qt | STL |
| --- | --- |
| - | `std::array` |
| `QVector` | `std::vector` |
| - | `std::deque` |
| `QLinkedList`|`std::list` |
| `QList` | - |
| - | `std::forward_list` |

##### Algorithmic Complexity

| Type | Random Access | Insertion | Prepending | Appending |
| ---- | --- | --- | --- | --- | --- |
| `QLinkedList` | O(n) | O(1) | O(1) | O(1) |
| `QList` | O(1)| O(n)| Amortised O(1) | Amortised O(1) |
| `QVector` | O(1) | O(n) | O(n) | Amortised O(1) |

The corresponding STL containers have the same complexity.

#### QList

`QList` is a bit dodgy as it, at first glance, looks like a linked list, i.e. a `std::list`, yet it's a very different container.
The `QList` is essentially an array of `void*` allowing fast random access, insertion, prepending and appending (see Algorithmic Complexity above). Each `void*` points to the added element which are allocated on the heap. However if `sizeof(T) <= sizeof(void*)` and the type has been declared to be either a `Q_MOVABLE_TYPE` or a `Q_PRIMITIVE_TYPE` the `QList` will internally use an array of `T` instead of `void*`. Because of this design the QList is a [memory waster](https://marcmutz.wordpress.com/effective-qt/containers/#containers-qlist) for most types compared to a `QVector`.

The official documentation recommended developer to use QList as the 'default' container in [Qt4](http://doc.qt.io/archives/qt-4.8/qlist.html), where as from [Qt5](http://doc.qt.io/qt-5/qlist.html) the official documentation encourage developers to use `QVector` instead:
>QVector should be your default first choice. QVector<T> will usually give better performance than QList<T>, because QVector<T> always stores its items sequentially in memory, where QList<T> will allocate its items on the heap...

### Container adaptors
| Qt | STL |
| --- | --- |
| `QStack` | `std::stack` |
| `QQueue` | `std::queue` |
| - | `std::priority_queue` |

* `QStack` inherits from `QVector`, where as `std::stack's` underlying container can be any container that meet some [requirements](https://en.cppreference.com/w/cpp/container/stack). By default `std::stack` uses `std::deque` as underlying container.

* `QQueue` inherits from `QList`. Similar to `std::stack` the `std::queue` as well as `std::priority_queue` can use any underlying container that meets some criteria, see [std::queue](https://en.cppreference.com/w/cpp/container/queue) and [std::priority_queue](https://en.cppreference.com/w/cpp/container/priority_queue). By default `std::queue`'s underlying container is `std::deque` and `std::priority_queue` uses a `std::vector`.


### Associative containers
| Qt | STL |
| --- | --- |
| - | `std::set` |
| `QSet` | `std::unordered_set` |
| - | `std::multiset` |
| - | `std::unordered_multiset` |
| `QMap` | `std::map` |
| `QMultiMap` | `std::multimap` |
| `QHash` | `std::unordered_map` |
| `QMultiHash` | `std::unordered_multimap` |

Note that `QSet's` STL counterpart is `std::unordered_set` and not `std::set`.

#####Algorithmic Complexity
| Type | Key Access | | Insertion | 
| --- | --- | --- | --- | --- | --- |
| | __Average__ | __Worst Case__ | __Average__ | __Worst Case__ |
| `QMap` | O(log n) | O(log n) | O(log n) | O(log n) |
| `QMultiMap` | O(log n) | O(log n) | O(log n) | O(log n) |
| `QHash` | Amortised O(1) | O(n) | Amortised O(1) | O(n) |
| `QSet` | Amortised O(1) | O(n) | Amortised O(1) | O(n) |

The corresponding STL containers have the same complexity.

### Other classes which resemble containers in Qt
The following three template classes are related to containers but don't expose any iterators.

* `QVarLengthArray`
* `QCache`
* `QContiguousCache`


#### QVarLengthArray
The `QVarLengthArray` container is a low-level array with a variable length, used for memory optimisation. On construction, the container allocates a predefined array of length N (default to 256) on the stack. If the amount of elements increases above N, the elements will be moved automatically to a heap memory and then function similarly to a `QVector`. 

A typical use case is when a temporary array is needed to be constructed multiple time, but the element size is varying:

```cpp
int func(const int n) {
  //int data[n + 1]; //won't compile
  //QVector data(n + 1); //func() is called many times, 
                         //requiring a lot of heap allocations.
  QVarLengthArray<int, 1024> data(n + 1); //Most cases n < 1024, 
                                          //and only a few heap allocations.
  ...
  return data[n];
}

```
 
#### QCache and QContiguousCache
`QCache` resembles a `QHash` (the underlying container is even a `QHash`) and is used for memory optimisation. The class takes ownership of the passed in elements and deletes them automatically when a specified maximum is reach. When adding new elements and the maximum is reached, the last used elements will be deleted. 

`QContiguousCache` function in a similar manner to `QCache` but restrict the elements in the container to be contiguous. This enables the class to be more memory efficient than the `QCache` and use fewer processor cycles.

## API
The Qt containers provide two sets of API: the "Qt Way" as well as the "STL Compatible Way". Personally, I think the Qt Way is more readable, however by using the STL API it will be easier to swap out a Qt container for its STL counterpart if desired in the future. See examples below for some of the API differences using a `QVector`:

```cpp
QVector data {0, 1, 2};
data.count() //or .size()
data << 3; //or .append() or .push_back()
if (data.isEmpty()) {} // or .empty()
```
The Qt containers have also been extended with some convenience functions which are not available in the STL counterparts. See example below:

```cpp
//Find an element in a vector
QVector qt_vector {0, 1, 2};
if (qt_vector.contains(1)) {
  ...
}

std::vector stl_vector {0, 1, 2};
if (std::find(std::begin(stl_vector), std::end(stl_vector), 1) != std::end(stl_vector)) {
  ...
}

```

The Qt containers also expose two types of iterators: ["java-style" iterators](http://doc.qt.io/qt-5/containers.html#java-style-iterators) and ["STL-style" iterators](http://doc.qt.io/qt-5/containers.html#stl-style-iterators). See example below:

```cpp
QVector<QString> names {"Alex", "Chris", "Andy", "Swati"};

//java-style iterator
QVectorIterator<QString> it{names};
while (it.hasNext()) {
  qDebug() << it.next();
}

//stl-style iterator
for (auto it = std::begin(names); it != std::end(list); ++it) {
  qDebug() << *it;
}
```
Also note that java-style iterators can't be used together with STL's generic algorithms.

Although the Qt containers have quite a few additional features they also lack some fundamental functionality compared to the STL containers:

* Qt containers don't provide range-construction nor range-insertion.
* It's not possible to use a custom allocator in any of the Qt containers. 
* None of the C++11 (or above) features/API have been implemented such as `emplace()` or rvalue-`push_back()`.
* No exception handling.
* The types must implement the default constructor and copy-constructor in order to be used in a Qt container.
* Move-only types are not (and will never be) supported because of Qt containers' principle of implicit sharing (see below for details about implicit sharing).

## Implicit sharing
[Most of Qt's containers](http://doc.qt.io/qt-5/implicit-sharing.html#list-of-classes) use implicit data sharing which is an optimisation to maximise resource usage by minimising copying. Basically the containers use reference counting and copy-on-write internally. What this means is that when a Qt container is copied, it's only a _shallow copy_ and the "real", _deep copy_, is done when a non-const function is called, best way to understand this is with an example:

```cpp
QVector<int> data {1, 1}; //Reference count = 1
auto data_copy = data; //Pointing to the same payload. Reference count = 2
auto element = data_copy[0]; //Still shared payload, const function called.
data_copy[1] = 2; //Detaches and performs a deep-copy. Reference count = 1
```

This design results in lower memory usage and less duplication of data as the program avoids unnecessary data copies. However, because of this behaviour it might result in some difficult-to-spot bugs. Example:

```cpp
QStringList names {"Alex", "Chris"};
auto &name = names[0];
auto data_copy = data;
name = "Andy"; //Even data_copy[0] has been changed to "Andy". 
```
Another example, which might cause some head-scratches is when using using a range-based for-loop. The loop might force the Qt container to detach and create deep copies unintentionally:

```cpp
QStringList getNames() {
  return QStringList{} << "Alex" << "Chris";
}

for (const auto& name : getNames()) { //non-const begin() is called 
    qDebug() << name;                 //resulting in a deep copy
}

//Preferred
const auto names = getNames();
for (const auto& name : names) { //const begin() is called 
	qDebug() << name;            //still shared payload.
}

//or in Qt 5.7
for (const auto& name : qAsConst(getNames())) {
    qDebug() << name;
}

};
```

Note: [avoid using Q_FOREACH (or foreach)](https://www.kdab.com/goodbye-q_foreach/).


## Conclusion... STL or Qt? 
My recommendation is to use the STL containers as your deault containers for the following reasons:

* Qt itself is moving away from the Qt containers and have replaced most of it's internal structures with the STL ones.
* As mentioned previously, none of the C++11 (or beyond) features have been implemented, and some of the C++98 are still missing (custom allocators, range-construction and range-insertion). 
* There are subtle bugs that might be difficult to find because of the implicit sharing.
* The STL library are usually implemented by the same people who implements the C++ compiler and are therefore most likely better optimised.

However, if you prefer the Qt API and are aware of their drawbacks, I personally don't believe that they are much worse than their STL counterparts.

Also, some of Qt's API returns Qt containers (`QStringList` is very common) and in those cases I recommend to avoid converting it to a STL counterpart.

## Read more
* [Qt's official documentation on the containers](http://doc.qt.io/qt-5/containers.html)
* [STL containers - cppreference.com](https://en.cppreference.com/w/cpp/container)
* [Marc mutz very detailed blog post series about understanding Qt containers inner-work](https://marcmutz.wordpress.com/effective-qt/containers/)