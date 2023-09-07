# Contributing

The best way to improve nomacs is your contribution. You can submit simple changes such as formatting corrections, extend manuals and READMEs where appropriate, [translate nomacs](https://crowdin.com/project/nomacs), [donate](https://www.paypal.com/donate/?token=QVR8WZJpxnlbuYpoUfeKzpwW0tdiMFZaNLDVOaCFhFi9P97Jklz2jvs8iqGyXAMYz0755G&country.x=US&locale.x=en_US), fix bugs, or submit new features.

If you contribute source code, you should fork and then clone nomacs:

```git
git clone git@github.com:your-username/nomacs.git
```

- [Build](README.md) nomacs on your machine.
- Make your changes.
- Commit in small chunks (one feature/bug fix per commit).
- Push to your fork and submit a pull request.

We will comment on pull requests within one week. Here are some things that increase the chances of your PR being accepted:

- Write good commit messages.
- Follow our style guide.
- Don't break the [Travis](https://travis-ci.org/nomacs/nomacs) build.

## Style

nomacs might have a weird coding style at the first glance, however we try to follow at least some rules:

- `using namespace` is prohibited (even for `std`)
- Naming
  - Class names start with a capital letter (i.e. `DkBaseManipulator`).
  - Methods start with lowercase (i.e. `name()`).
  - Prefix members with an m (i.e. `int mAction = 0`).
  - Prefix classes with `Dk`.
- format C++ sources with `clang-format -i --style=file your_contribution.cpp`
- Encapsulate everything with the namespace `nmc`

Right:

```cpp
namespace nmc
{

class DllCoreExport DkBaseManipulator
{
public:
    DkBaseManipulator(QAction *action = 0);

    QString name() const;

private:
    QAction *mAction = 0;
};

}
```

Wrong:

```cpp
class DllCoreExport baseManipulator
{

public:
  baseManipulator(QAction* Action = 0);

  QString Name() const;

private:
  QAction* action = 0;
};
```
