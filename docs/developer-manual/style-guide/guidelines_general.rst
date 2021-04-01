.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _GENERAL_RULES:

General Rules
=============

..
    Comments:
    uFEFF is a Unicode zero-width no-break space character

.. spelling::
    uFEFF
    xEF
    xBB
    xBF


The following rules generally apply and follow the naming schema
``GENERAL:<ongoing-number>``.

.. _rule_general_filenames:

Filenames (``GENERAL:001``)
---------------------------

Generally file naming rules are not necessary, but for reasons
of consistency there are some rules for that within the project.

.. admonition:: Filenames

    Filenames as well as directory names **SHOULD** only consist of lowercase
    alphanumeric characters and may include underscores (``_``), dashes
    (``-``) and dots (``.``). A correct file or directory name matches the
    regular expression ``^[a-z0-9_\-.]*$``. Valid exceptions are e.g.,
    ``README.md`` and ``CHANGELOG.md``.

.. _rule_general_filename_uniqueness:

Filename uniqueness (``GENERAL:002``)
-------------------------------------

Unique filenames help to avoid confusion. Furthermore in case of file includes
or imports non-unique filenames might lead to undesired behavior as wrong files
might be used.

.. admonition:: Filename Uniqueness

    - Filenames **SHOULD BE** be unique. Valid exceptions are e.g.,
      ``README.md`` to explain the content of a directory, or directory-local
      ``.gitignore`` files.
    - Source files **MUST** use unique filenames.

.. _rule_general_text_file_encoding:

Encoding (``GENERAL:003``)
--------------------------

.. admonition:: Encoding

    - All plain text files **SHOULD** use ``UTF-8`` encoding.
    - Files **MAY** use other encodings if these are required by standards or
      other tools. Valid exceptions are

      - ``*.c*`` and ``*.h`` files **MUST** use ``ASCII`` encoding
      - ``*.sym`` and ``*.dbc`` files **MUST** use ``ASCII`` encoding.

.. _rule_general_eof:

End-of-File (``GENERAL:004``)
-----------------------------

.. admonition:: End of File

    All plain text files **MUST** end with a single empty line
    (`POSIX, 3.206 Line <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap03.html#tag_03_206>`_).

.. _rule_general_no_trailing_whitespace:

No trailing whitespace (``GENERAL:005``)
----------------------------------------

Undesired changes in whitespace make diffs more hard to read and are
unnecessary. To reduce this problem, trailing whitespace is not allowed.

.. admonition:: No trailing whitespace

    All plain text files **MUST NOT** add trailing whitespace.

.. _rule_general_indentation:

Indentation (``GENERAL:006``)
-----------------------------

Undesired changes in whitespace make diffs more hard to read and are
unnecessary. To reduce this problem, all indentations **MUST** be of one type.

.. admonition:: Indentation

    - All plain text files **MUST** use spaces for indentation. Use
      4 spaces at a time.
    - Tabs **MUST NOT** be used.

You should set your editor to automatically replace tabs with spaces to ease
development.

.. _further_general_rules:

Further General Rules
---------------------

- Optimize for readability using descriptive names that would be clear even to
  people on a different team.
- Use names that describe the purpose or intent of the object. Do not worry
  about saving horizontal space as it is far more important to make your code
  immediately understandable to a new reader. Minimize the use of abbreviations
  that would likely be unknown to someone outside your project (especially
  acronyms and initialisms). Do not abbreviate by deleting letters within a
  word. As a rule of thumb, an abbreviation is probably ok if it's listed in
  Wikipedia. Generally speaking, descriptiveness should be proportional to the
  name's scope of visibility. For example, ``n`` may be a fine name within a
  5-line function, but within the scope of a file, it's likely too vague.
- For the purposes of the naming rules below, a "word" is anything that you
  would write in English without internal spaces. This includes abbreviations
  and acronyms; e.g., for "camel case" or "Pascal case," in which the first
  letter of each word is capitalized, use a name like ``StartRpc()``, not
  ``StartRPC()``.
- Non-ASCII characters should be rare, and **MUST** use ``UTF-8`` encoding. You
  **SHOULD NOT** hard-code user-facing text in source code, even English, so
  use of non-ASCII characters should be rare. However, in certain cases it is
  appropriate to include such words in your code. For example, if your code
  parses data files from foreign sources, it may be appropriate to hard-code
  the non-ASCII string(s) used in those data files as delimiters. More
  commonly, unit test code (which does not need to be localized) might contain
  non-ASCII strings. In such cases, you should use ``UTF-8``, since that is an
  encoding understood by most tools able to handle more than just ASCII. Hex
  encoding is also ok, and encouraged where it enhances readability - for
  example, "\\xEF\\xBB\\xBF", or, even more simply, u8"\\uFEFF", is the Unicode
  zero-width no-break space character, which would be invisible if included in
  the source as straight ``UTF-8``.
- When referring to files inside the repository the UNIX-style path
  separator **MUST** be used. When referring to files outside the repository
  the platform specific path separator **MUST** be used (e.g.,
  ``src/app/main/main.c`` and ``C:\Users\vulpes``).
- References to functions should be kept as simple and maintainable as
  possible. For example when referring to a function with the declaration
  ``uint8_t ABC_MyFunction(uint8_t a, bool b)`` the reference **SHOULD** only
  use ``ABC_MyFunction()`` without arguments or ellipses. If the context
  requires specific arguments they **MAY** be provided. The ellipses form
  **MUST NOT** be used (see https://en.wikipedia.org/wiki/Ellipsis), e.g., do
  **not** use ``ABC_MyFunction(...)``).
