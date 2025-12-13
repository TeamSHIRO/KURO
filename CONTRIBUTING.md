# Contributing to KURO
We welcome contributions from the community!

KURO is a community-driven project, and we actively welcome contributions. Your involvement is essential to our success.

Whether you're fixing a bug, adding a new feature, or improving documentation, your help is appreciated. Here are some guidelines to get you started:

## Table of Contents
- [**Terminology**](#terminology)
- [**Code of Conduct**](#code-of-conduct)
- [**Contributing Guide 101**](#contributing-guide-101)
    - [**Prerequisites**](#prerequisites)
    - [**Contributing in a Nutshell**](#contributing-in-a-nutshell)
    - [**Git tags**](#git-tags)
    - [**Branching**](#branching)
    - [**Commits**](#commits)
    - [**Developer Certificate of Origin**](#developer-certificate-of-origin)
    - [**Style Guide**](#style-guide)
    - [**Pull Request Checklist**](#pull-request-checklist)
- [**Thanks for Contributing!**](#thanks-for-contributing)

## Terminology
The following terms are used throughout this document.

* **Contributor:** Anyone who submits a pull request to the project.
* **Maintainer:** A member of the !SHIRO GitHub organization who has the privilege to merge pull requests.
* **Source file:** A file that contains source code, Including headers. This includes but not limited to files with the
  following extensions: `.hpp`, `.cpp`, `.h`, `.c`. Build scripts or Shell scripts or any configuration files are **not** considered source files
  (e.g., `CMakeLists.txt`, `something.cmake`, `do_something.sh`, `README.md`).

## Code of Conduct
Please note that this project is released with a [**Contributor Code of Conduct**](CODE_OF_CONDUCT.md). By participating in
this project, you agree to abide by its terms.


## Contributing Guide 101
If you're ready to dive into the codebase and submit new features or bug fixes, this section is for you.

These guides assume that you have a basic understanding of Git, GitHub, and C development tools.

---

### Prerequisites
Please look at the [**Building**](README.md#building) and [**Prerequisites**](README.md#prerequisites) section in the [**README**](README.md). To submit a pull request, you must have
a **GitHub** account.

Additionally, you might need these tools to run the checks:
- clang-format

There are no explicit version requirements; however, you should ensure that every configuration works with your current tool version.

Please make sure all the above tools are locatable from your shell with `which` and is in `PATH` for the scripts inside [`automated/`](automated) to work properly.

---

### Contributing in a Nutshell
Here are the steps to contribute to the project:
1. If you didn't have write access to the repository, fork the repository to your own account.
2. Create a new branch from `main`.
3. Implement your feature or fix in your branch.
4. Open a Pull Request (PR) back into `main`.
5. The maintainer will review your changes and provide feedback.

---

### Git Tags
Git tags with `v` prefix are not allowed. Because it is reserved for release. Any other tag is allowed, but please keep it to the minimum.

---

### Branching
We recommend naming a branch to be descriptive of the feature or bug fix you are working on.

Prefer `kabab-case` for branch names.

---

### Commits
Please make sure your commit messages are not overly verbose and are concise.

---

### Developer Certificate of Origin
By contributing to this project, you agree that your contributions will be licensed under the project's license and that
you have the right to make those contributions. Please ensure you sign the Developer Certificate of Origin (DCO) by
adding a "Signed-off-by" line to your commit messages.

```text
Developer Certificate of Origin
Version 1.1

Copyright (C) 2004, 2006 The Linux Foundation and its contributors.
1 Letterman Drive
Suite D4700
San Francisco, CA, 94129

Everyone is permitted to copy and distribute verbatim copies of this
license document, but changing it is not allowed.


Developer's Certificate of Origin 1.1

By making a contribution to this project, I certify that:

(a) The contribution was created in whole or in part by me and I
    have the right to submit it under the open source license
    indicated in the file; or

(b) The contribution is based upon previous work that, to the best
    of my knowledge, is covered under an appropriate open source
    license and I have the right under that license to submit that
    work with modifications, whether created in whole or in part
    by me, under the same open source license (unless I am
    permitted to submit under a different license), as indicated
    in the file; or

(c) The contribution was provided directly to me by some other
    person who certified (a), (b) or (c) and I have not modified
    it.

(d) I understand and agree that this project and the contribution
    are public and that a record of the contribution (including all
    personal information I submit with it, including my sign-off) is
    maintained indefinitely and may be redistributed consistent with
    this project or the open source license(s) involved.
```

**Any unsigned commits will be rejected during the PR process.**

Tips: You can use `git commit -s` to sign your commits.

---

### Style Guide

- **Macro and Enumerator names should be in `UPPER_SNAKE_CASE`**
- **Anything else should be in `snake_case`**
- **License headers:**
    - Every source file must have a license header at the top of the file.
    - The license header must follow the format specified in the [**license-header-template**](docs/license-header-template.md) file.
- **File names:**
    - Every file should be in `lowercase` where words are separated by any of the following characters or none: `-`, `_`.
    - Every C file (source or header) must have a `.c` or `.h` extension.
    - Every NASM file must have a `.asm` extension and `.inc` for include files.
- **Every global variable should be prefixed with `g_`**

---

### Pull Request Checklist
To ensure that your pull request is pass the checks, please ensure that you have completed the following:
- [ ] Your code follows the style guide outlined above.
- [ ] You have signed the Developer Certificate of Origin (DCO) for all your commits.
- [ ] You have run clang-format on your code, and it passes all checks.

---

## Thanks for Contributing!
Thanks for contributing to KURO! We look forward to your contributions.
