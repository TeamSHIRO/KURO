# Contributing to KURO

We welcome contributions from the community!

KURO is a community-driven project, and we actively welcome contributions. Your involvement is essential to our success.

Whether you're fixing a bug, adding a new feature, or improving documentation, your help is appreciated. Here are some guidelines to get you started:

## Table of Contents

- [**Code of Conduct**](#code-of-conduct)
- [**Contributing Guide 101**](#contributing-guide-101)
    - [**Prerequisites**](#prerequisites)
    - [**Contributing in a Nutshell**](#contributing-in-a-nutshell)
    - [**Branching**](#branching)
    - [**Pull Request Checklist**](#pull-request-checklist)
- [**Thanks for Contributing!**](#thanks-for-contributing)

## Code of Conduct

Please note that this project is released with a [**Contributor Code of Conduct**](CODE_OF_CONDUCT.md). By participating in
this project, you agree to abide by its terms.


## Contributing Guide 101

If you're ready to dive into the codebase and submit new features or bug fixes, this section is for you.
 Build scripts or Shell scripts
These guides assume that you have a basic understanding of Git, GitHub, and C development tools.

---

### Prerequisites

Please look at the [**building**](docs/building.md) section of the documentation to ensure that you have installed all
the necessary tools and dependencies to build the project. To submit a pull request, you must have a **GitHub** account.

Additionally, you would need these tools to run the checks:
- clang-format
- clang-tidy

There are no explicit version requirements; however, you should ensure that every configuration works with your current tool version.

Please make sure all the above tools are locatable from your shell for the scripts to work properly.

---

### Contributing in a Nutshell

Here are the steps to contribute to the project:
1. If you didn't have write access to the repository, fork the repository to your own account.
2. Create a new branch from `main`.
3. Implement your feature or fix in your branch.
4. Open a Pull Request (PR) back into `main`.
5. The maintainer will review your changes and provide feedback.

---

### Branching

We recommend naming a branch to be descriptive of the feature or bug fix you are working on, and each branch should have a single purpose.

---

### Pull Request Checklist

To ensure that your pull request is pass the checks, please ensure that you have completed the following:
- [ ] You have signed the Developer Certificate of Origin (DCO) for all your commits.
- [ ] You have run clang-format and clang-tidy on your code, and it passes all checks.
- [ ] All files should be in lowercase when applicable. Preferably snake_case. 

---

## Thanks for Contributing!

Thanks for contributing to KURO! We look forward to your contributions.
