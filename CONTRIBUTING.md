# Contributing
## Ways to Contribute
The following list items are recomendations; the list is by no means exhaustive:
1. Solve existing bugs/issues via the issue tracker.
2. Implement tests.
3. Write documentation - either line/code comments or more formal `doc`-folder documents.
4. Propose features via the issue tracker (using the discussion label).
5. Work on existing feature proposals (unless they've already been assigned).
## General Code Contribution Guidelines
Though it is the responsibility of maintainers to ensure guideline compliance, adherence to guidelines can help expedite the review and implementation process. It is thus highly recomended that contributors familiarize themselves with the following:
### Commits & Pull Requests
Commits should be issued on every modified file, with a message that sufficiently describes the modifications therein. Pull requests should be issued on the `main` branch with a title that likewise describes changes to the codebase. For example, if a file `example.cpp` is modified and another file `example2.cpp` is created, then:
```
    Commit 1: dir/example.cpp   Message: "Added comment to describe xyz..."
    Commit 2: dir/example2.cpp  Message: "Created dir/example2.cpp"
```
With both commits going on a separate, contributor-created fork/branch, and with the pull request issued on `main`.
### Discuss Big Changes as Issues
Feature proposals and large changes should be opened as discussion issues (using the discussion label) prior to any work/implementation. This affords the maintainers the opportunity to comment on whether the modification fits the scope and vision of the project (hence saving both the maintainer's and contributor's time). Alternatively, feel free to fork the project and work on your proposals on your own - provided that you respect the project's GPL 3.0 license
## C++ Syntax & Formatting
The Rinto programming language tries to adheres to the [Linux Kernel Coding Style](https://www.kernel.org/doc/html/latest/process/coding-style.html), with some exceptions:
### Camel case naming
Instead of Linux's C-style underscore naming convention, Rinto uses camel case naming. For example:
```C++
int my_variable; // Incorrect
int myVariable;  // Correct
```
Names should not be capitalized unless otherwise stated.
<<<<<<< HEAD
<<<<<<< HEAD
=======
### Packages/libraries
Separate programs should be written in separate folders (i.e 'packages'). Separate packages should be written under their own namespace, with the exception of classes:
```C++
namespace MyLibrary
{

int myFunction()
{
    // ...
    return 0;
}

}
```
If a package has both member functions and a separate class, then the class should inherit the name of the package and the member functions should use a namespace of the same name. For example:
```C++
namespace MyLibrary
{
int memberFunction();
int memberFunction2();
}

class MyLibrary
{
    // ...
}

```
This way, everything related to `MyLibrary` is addressible either through `MyLibrary.ClassAttribute` or `MyLibrary::memberFunction`.
#### Namespaces are important
Namespace curly braces should begin on their own new lines.
>>>>>>> 5d74d0b (created CONTRIBUTING.md)
=======
>>>>>>> 3693974 (created CONTRIBUTING.md)
### Classes
#### Header files
Classes should always be templated in `.h` files.
#### Class names are always capitalized
Class names should always be capitalized, see below:
```c++
class myClass {}; // Incorrect
class MyClass {}; // Correct
```
#### Class brackets
Like functions in the Linux C style, classes are important. So, curly braces should start on their own lines:
```c++
// Incorrect:
class MyClass {
  // ...
};

// Correct:
class MyClass
{
 // ...
};
```
#### Private variable class attributes
Private variable class attributes are always capitalized to differentiate them from other function parameters. So, for example:
```c++
class MyClass
{
private:
     int notCapitalised;  // Not correct!
     int CapitalizedAttr; // Correct!
     // ...
};
```
