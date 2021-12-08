.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _SOFTWARE_DEVELOPMENT_PROCESS:

Software Development Process
============================

The main goal of the development process is to ensure that only well tested
code is committed to the |master_branch|. This can either be a new feature to
extend the capabilities of the existing implementation or changes that remove
a bug in the existing implementation. For the sake of simplicity these changes
are summarized under the terms of |change_request| (for details see
`here <https://en.wikipedia.org/wiki/Change_request>`__).

The starting point of all work done, is an |issue| describing a bug in the
current implementation or a feature that is missing or needs to be extended.
Based on this |issue| a so called |feature_branch| is created (the name is
the same whether it is bug or feature) by a developer working on the related
issue. This |feature_branch| is pushed to the main (bare) repository managed by
some |git| server. The |git| server triggers the CI pipeline. This pipeline
manages several build servers that then run different integration tasks (jobs)
(building the binary, testing and checking for guideline violations). The
output of a build server is a success matrix based on the run jobs. The status
of the test is shown in the |change_request|.
In parallel the changes that would be introduced to the |master_branch| are
reviewed by developers (developers that did not work on the |feature_branch|)
and comments to the changes are added as needed.

The first requirement for a |feature_branch| getting merged into the
|master_branch| is the success of the CI pipeline and the second requirement is
the approval of the reviewer. If one of both is missing merging the
|feature_branch| is rejected.

This development process is shown in
:numref:`software-development-process-diagram`.

.. graphviz:: software-development-process.dot
    :caption: |foxbms| software development process
    :name: software-development-process-diagram

.. |issue| replace:: ``issue``
.. |change_request| replace:: ``Change Request``
.. |feature_branch| replace:: ``feature`` branch
.. |master_branch| replace:: ``master`` branch
