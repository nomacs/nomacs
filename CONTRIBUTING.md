# Contributing

The best way to improve nomacs is your contribution. There are several areas in need of contributors that do not involve programming tasks.

- **Issue Triage**
  - Confirm if latest version fixes the issue
  - Request or provide additional information, test files, screenshots etc
  - Provide workarounds
  - Test on different platforms
  - Find duplicates of other issues.
    - You can comment `Duplicate: #<issue>` in the newer issue. We would then close them.
  - Tag related issues
    - You can comment `Related: #<issue>` to help us group them.
  
- **Testing**
  - Try to break things
  - Compile nomacs and test the latest changes in-between releases
  
- **Documentation**
  - Add or update FAQ (search issues for ideas)
  - Document features (sync, batch, etc)
  - Describe workflows, use cases

- **[Translate Nomacs](https://crowdin.com/project/nomacs)**

## Contributing Code

To contribute source code, you should fork and then clone nomacs, and then make a branch for your changes:

```bash
git clone git@github.com:your-username/nomacs.git
cd nomacs
git checkout -b <my-feature> master
```

- [Build](README.md) nomacs on your machine.
- Make your changes.
- Commit in small chunks (one feature/bug fix per commit). Use separate commits for refactoring and build system changes. This makes changes easier to review and revise.
- Use clang-tidy, clang-format, gersemi to find errors, non-compliance to guidelines, and format sources/cmake files.
- Changed code should compile without warnings.
- Push to your fork and submit a pull request.

We will usually comment on pull requests within one week. Here are some things that increase the chances of your PR being accepted:

- Please try to test areas that the PR is changing or that depend on your changes.
- We support Windows, Linux, and macOS. If possible, please try to test on different platforms.
- We tend to avoid platform-specific features. Please discuss with us beforehand.
- Follow the style guide.  
- Pass all the Github action checks.
- Write good commit messages. Use [conventional commits](https://www.conventionalcommits.org/en/v1.0.0/#summary) as a guide, following existing commit messages for ideas. 
  - `fix[(scope)]: <describe bug>` if it fixes a bug.
  - `feat[(scope)]: <describe feature>` if it adds a feature
  - If you have fixed an issue, add `Fixes: #<num>` in the body, on one line.

## Code Style

nomacs might have a weird coding style at the first glance, however we try to follow at least some rules:

### Naming
- Class names start with a capital letter (i.e. `DkBaseManipulator`).
- Methods start with lowercase (i.e. `name()`).
- Prefix members with an m (i.e. `int mAction = 0`).
- Prefix classes with `Dk`.
### Formatting
- We format C++ sources `clang-format`. You can run `scripts/format-cpp.sh` or configure your editor to use `clang-format`.
- We format cmake sources with `gersemi`. You can run `scripts/format-cmake.sh`.
- All repository files must have **LF** line endings
### C++ guidelines
- Standard: C++17
- Encapsulate everything with the namespace `nmc`
- `using namespace` is prohibited (even for `std`) for portability
- Avoid using `SIGNAL` and `SLOT` macros in favor of function pointers.
  They enable compile time check of the signal and slot names and signature.
- Avoid `auto` unless the type name appears on the same line. This helps with readability.
  - Exception: loop variables: `for(const auto& x : container)`
  - Exception: iterators (but prefer the new loop syntax)
  - Use `auto` only to replace the type name. This means the `&`, `*`, or qualifiers (e.g. `const` and `volatile`) should always be included. For  example,
  
     ```cpp
     // Do write this
     auto *foo = new Foo(); 
     // instead of 
     auto foo = new Foo(); 
     ```
- Use clang-tidy, it formalizes rules we want to maintain and can check while you code if your editor supports it (see `ImageLounge/.clang-tidy`)

Right:

```cpp
namespace nmc
{

class DllCoreExport DkBaseManipulator
{
public:
    DkBaseManipulator(QAction *action = nullptr);
    virtual DkBaseManipulator() = default;

    QString name() const;

private:
    QAction *mAction = nullptr;
};

}
```

Wrong:

```cpp
class DllCoreExport baseManipulator
{

public:
  baseManipulator(QAction* Action = nullptr);
  virtual ~baseManipulator() {}

  QString Name() const;

private:
  QAction* action = nullptr;
};
```

