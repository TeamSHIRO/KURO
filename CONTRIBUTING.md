# Contributing

We welcome contributions from the community!

This is a community-driven project, and we actively welcome contributions. Your involvement is essential to our success.

Whether you're fixing a bug, adding a new feature, or improving documentation, your help is appreciated. Here are some guidelines to get you started:

## Table of Contents

- [**Code of Conduct**](#code-of-conduct)
- [**Contributing Guide 101**](#contributing-guide-101)
    - [**Prerequisites**](#prerequisites)
    - [**Branching**](#branching)
    - [**Contributing in a Nutshell**](#contributing-in-a-nutshell)
    - [**Developer Certificate of Origin**](#developer-certificate-of-origin)
    - [**Pull Request Checklist**](#pull-request-checklist)
- [**Thanks for Contributing!**](#thanks-for-contributing)

## Code of Conduct

Please note that this project is released with a [**Contributor Code of Conduct**](CODE_OF_CONDUCT.md). By participating in
this project, you agree to abide by its terms.


## Contributing Guide 101

If you're ready to dive into the codebase and submit new features or bug fixes, this section is for you.
 Build scripts or Shell scripts
These guides assume that you have a basic understanding of Git, GitHub, and C development tools.

### Prerequisites

Please look at the [**building**](docs/building.md) section of the documentation to ensure that you have installed all
the necessary tools and dependencies to build the project. To submit a pull request, you must have a **GitHub** account.

Additionally, you would need these tools to run the checks:
- clang-tidy

There are no explicit version requirements; however, you should ensure that every configuration works with your current tool version.

Please make sure all the above tools are locatable from your shell for the scripts to work properly.

### Branching

We recommend naming a branch to be descriptive of the feature or bug fix you are working on.

Prefer `kabab-case` for branch names.

### Contributing in a Nutshell

Here are the steps to contribute to the project:
1. If you didn't have write access to the repository, fork the repository to your own account.
2. Create a new branch from `develop` called `feature/<feature-name>`.
3. Implement your feature in your branch.
4. Open a Pull Request (PR) back into `develop`.
5. The maintainer will review your changes and provide feedback.

We use the GitFlow to manage our branches.

The relevant branches are:

- The `main` branch is the latest stable release.
- The `develop` branch is the latest development branch.
- The `feature/<feature-name>` branch is used to implement new features.

For more information, please refer to [GitFlow](https://www.atlassian.com/git/tutorials/comparing-workflows/gitflow-workflow).

### Pull Request Guidelines

When you submit a pull request, please describe the changes you made and the reasoning behind them thoroughly.
The maintainer will review your changes and provide feedback. You might be asked to make changes to your pull request
based on the feedback.

### Developer Certificate of Origin

By contributing to this project, you agree that your contributions will be licensed under the project's license and that
you have the right to make those contributions. Please ensure you sign the Developer Certificate of Origin (DCO) by
adding a "Signed-off-by" line to your commit messages.

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

**Any unsigned commits will be rejected during the PR process.**

> [!TIP]
> You can use `git commit -s` to sign your commits.

### Pull Request Checklist

To ensure that your pull request is pass the checks, please ensure that you have completed the following:
- [ ] You have run clang-tidy on your code, and it passes all checks.
- [ ] You have signed the Developer Certificate of Origin (DCO) for all of your commits.
- [ ] All files should be in the lowercase when applicable. Preferably snake_case.

## Thanks for Contributing!

Thanks for contributing! We look forward to your contributions.
