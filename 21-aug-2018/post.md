# CMake it modern using C++ and Qt, Part 1

__Blog post published on [cleanqt.io](www.cleanqt.io)__ 

Does your CMake projects look similar to the example below? If yes - I'm positive you'll find this post helpful as it's is especially composed for you. This post will cover the core of what's considered modern CMake and will be part of a blog post series.

```
...
add_definitions("-Wall")

include_directories(${PROJECT_SOURCE_DIR}/src/)
set(CMAKE_CXX_FLAGS -std=c++11)

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
...
``` 

Before we dive into the details of potential problems with the code above, one might wonder 'What is __Modern__ CMake?'. It seems that the consensus of what's considered modern begins from version 3 and above when __targets__ and __properties__ were more feature complete. These concepts introduced a new paradigm which improved upon the build structure including project scoping and the handling of transitive dependencies. Version 3.0.0 was released four years ago (as of 2018), but somehow, this new powerful paradigm never really took off. Depressingly, the official documentation from KitWare is still providing [tutorials](https://cmake.org/cmake-tutorial/) using old methods and practices. 

First time I heard about modern CMake was a couple of years ago, but I really started to embrace it after watching an excellent talk by [Daniel Pfeifer](https://www.youtube.com/watch?v=bsXLMQ6WgIk) last year. He opened my eyes to how good, modern and clean CMake looks like; perhaps you'll appreciate it as much as I did. Some of these concepts and guidelines, including an example project with Qt, will be presented in this series.

### Target and Properties

As aforementioned, the main concepts in modern CMake are targets and properties. But what are they? 

A target can be thought of as a class with encapsulation. The target is for example an executable, a shared library or a header-only library. 

In a similar analogy, properties can be thought of as member variables and methods in a specific target. These can either be used internally when the project is built, or exposed to, and inherited by, other projects when they  link to the target. Examples of properties are the target's source code, the required compiler options as well as the libraries that the target needs to link to. 

In modern CMake a project will be defined by one or more targets which building blocks consist of the properties. 

### Property scope

Whether properties are used internally, externally or both, i.e. which scope they belong to, is an important concept in modern CMake and will be covered next. 

Internal properties belong to the __PRIVATE__ scope whereas external properties should be defined in the __INTERFACE__ scope. There is also a third scope: __PUBLIC__, which is used when properties are needed internally as well externally. PRIVATE properties define the build requirements whereas the INTERFACE define the usage requirements.

To define properties in an INTERFACE scope, the CMake command is either prepended with INTERFACE or passed in as an argument. Similar rules apply to the PRIVATE scope. This can be best understood with the help of a few examples:

__Example 1.__ The target, `Foo`, is a shared library which internally uses some C++11 features and therefore requires to be built by a C++11 compatible compiler. Although the internal library is using C++11 features the API itself doesn't expose any. Hence, the compiler feature `cxx_std_11` should be declared in the __PRIVATE__ scope:

```
target_compile_features(Foo PRIVATE cxx_std_11) 

# Same as above. All features defined in compile_features() are private.
# Note that private_ should not be prepended to the command.
compile_features(cxx_std_11)
```

__Example 2.__ The target, `Foo`, is a shared library and exposes variadic template functions. These are used internally as well as externally, therefore the compiler feature `cxx_variadic_templates` needs to be part of the __PUBLIC__ scope:

```
target_compile_features(Foo PUBLIC cxx_variadic_templates)
```

__Example 3.__ The target, `Foo`, is a header-only library and exposes variadic template functions. Similarly to example two, `cxx_variadic_templates` needs to be exposed externally. However, since the library doesn't need to be built - again it's a header-only library - it's sufficient to only expose it externally using __INTERFACE__:

```
target_compile_features(Foo INTERFACE cxx_variadic_templates)

# Same as above. All features defined in interface_compile_features() are exposed externally.
# Note that for external exposure, we prepend with interface_
interface_compile_features(cxx_variadic_templates)
```

### Back to the code from the past
With these concepts in mind, let's go through some of the lines in the code above and explore them more in details.

```
add_definitions("-Wall")
include_directories(${PROJECT_SOURCE_DIR}/src/)
set(CMAKE_CXX_FLAGS -std=c++11)
```

Let's start with the first line. The `-Wall` compiler option is used to to enable all GCC compiler's warning messages. However, it will not only apply to the current sources but also to all _directories and sources below_. Perhaps this is the intention, but it's likely to lead to some issues along the development road.

Imagine that you're working on a big project with a hierarchy of many targets and the top level project has added the `-Wall` flag. In your specific subproject you're only interested in certain warnings. How would you solve this? You could potentially add `remove_definitions("-Wall")` in your CMake file or perhaps temporary remove the `"-Wall"` flag from the top level. But what if you accidentally commit the change? Also someone who's new to CMake might struggle more when resources are leaked from one project to another. A modern approach is to apply the property on the target itself by declaring it within the PRIVATE scope:

```
target_compile_options(Foo PRIVATE -Wall)
```

Next, let's tackle the second line. The `include_directories()`-command will add directories to the search paths, which are used for some compilers when including files. Similarly to the previous command, this command is leaked to all targets in the same scope. In case the target is a shared library, only the public API directories needs to be exposed to other targets. For this instance it would be better to define both the public and private scope separately using `target_include_directories()`:

```
target_include_directories(Foo
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/public
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

```

I'm sure you already know the potential problem with the third line by now: setting the `CMAKE_CXX_FLAGS` will automatically apply to all other targets in the same scope. The target alternative command was already covered in _example 3_ above.

### `target_linked_libraries()` solves transitive dependencies
For a similar reason the global command `linked_libraries()` should also be avoided when linking to libraries. The target alternative command is `target_linked_libraries()`. Not only does it handle scoping but it will also solve transitive dependencies. For example, when you need to link to a target you'll also have to to link to its dependencies. And the dependencies might have dependencies. These transitive dependecies will be solved by the target command as it automatically traverse the dependency tree.

### Warning about `file(GLOB ...)`

The information above has covered global commands and targets, so what's wrong with `file(GLOB ...)`?

```
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
```
The arguably problem with `file(GLOB ...)` is that some build systems will not automatically include newly created files without re-running CMake. This could potentially lead to confusion and build issues for other developers when a new file has been committed to the source control. 

The reason CMake needs to be re-run is because it is not a build system: it's a build system __generator__. The build system doesn't need to know anything about CMake - it's just generated by it. In reality, some build systems are aware of the new files and automatically re-runs CMake.

Although it's somewhat conceptually broken, I prefer to use `file(GLOB ...)` instead of manually adding every new file to CMake. Policies such as "Always manually re-run CMake after a checkout or when adding add new files" can be used in order to avoid the potential problems. Also note that in CMake 3.12 a new flag [CONFIGURE_DEPENDS](https://cmake.org/cmake/help/latest/command/file.html) will be added that improves the usage:

>If the CONFIGURE_DEPENDS flag is specified, CMake will add logic to the main build system check target to rerun the flagged GLOB commands at build time. If any of the outputs change, CMake will regenerate the build system.


### Summary

* Reduce the usage of global variables and embrace __targets__ and __properties__.
* Be selective with the scope - if possible, avoid exposing parameters in the __INTERFACE__ scope.
* Avoid using global commands such as `include_directories()` and `linked_libraries()`.
* `target_linked_libraries()` automatically solves transitive dependencies,
* Be aware of `file(GLOB ...)` and if you do use it - learn and share about the problems to the team.

I've only touch the surface of modern CMake as this post only covers the core of it. If you're intrested in learning more, an excellent resource is the following [GitHub project](https://cliutils.gitlab.io/modern-cmake/). In the next part we'll go through an example with Qt and how a library can be exported using modern methodologies. 

Also, my best advice is to keep CMake code clean - treat it as it would be production code. It will save time in the long run!

