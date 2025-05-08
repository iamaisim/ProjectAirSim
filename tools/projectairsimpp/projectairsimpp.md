# ProjectAirSimPP Text Preprocessor

The ProjectAirSimPP text preprocessor is a Markdown preprocessor extension and a stand-alone text preprocessor Python script.  It supports C-like preprocessor directives and expression syntax.

Although created as a Markdown preprocessor, it can preprocess any type of text file, not just text files in Markdown format.

## Installing

To install ProjectAirSimPP, navigate to the directory containing the `projectairsimpp` directory and install with `pip`:

``` bash
pip install projectairsimpp
```

To install `projectairsimpp` "in-place" so that ProjectAirSimPP runs directly from the `projectairsimpp` directory, give the "-e" flag:

``` bash
pip install -e projectairsimpp
```

## Using ProjectAirSimPP in mkdocs

To use ProjectAirSimPP as a Markdown extension in `mkdocs`, add `projectairsimpp` as a entry under the `markdown-extensions` section:

```yml
[...]
markdown-extensions:
    [...]
    - projectairsimpp:
        include_paths: "templates;docs/includes"
        log_level: "warning"
[...]
```

ProjectAirSimPP accepts the following config parameters:
| Parameter | Description |
| --- | --- |
| `include_nest_limit` | Number specifying the maximum number of nested `include` files at any one time.  (Default 25.)
| `include_paths` | String containing a semicolon-separated list of directories to search for the file in an `include` directive. |
| `log_level` | String specifying the logging message severity level: `"info"`, `"warning"`, `"error"`, or `"debug"`.  Only messages at or above this level will appear. (Default `warning`.) |

## Running ProjectAirSimPP standalone

When run stand-alone, the basic ProjectAirSimPP usage is:

`python -m projectairsimpp` [`--includenestlimit` <i>limit</i>] [`--includepaths` <i>pathlist</i>] [`--outputfile` <i>filename</i>] [`--loglevel` <i>level</i>] <i>inputfile</i>

where:

| Argument | Description
| --- | --- |
| <i>inputfile</i> | The file to preprocess. |
| <code>&#x2011;&#x2011;includenestlimit</code> <i>count</i> | Number specifying the maximum number of nested `include` files at any one time.  (Default 25.)
| <code>&#x2011;&#x2011;includepaths</code> <i>pathlist<i> | Semicolon-separated list of directories to search for the file in an `include` directive. |
| <code>&#x2011;&#x2011;outputfile</code> <i>filename</i> | Where to write the preprocessed output.  If not specified, the output is written to standard output. |
| <code>&#x2011;&#x2011;loglevel</code> <i>level</i> | Set the logging message severify level where <i>level</i> is `info`, `warning`, `error`, or `debug`.  Only messages at or above <i>level</i> will appear. (Default `warning`.) |

## Tags and directives

ProjectAirSimPP operates on "tags" embedded in the source document which contains a directive and any arguments.  A tag has the form:

<p style="margin-left: 18pt"><code>{#</code> <i>directive</i> [<i>args</i>...] <code>#}</code></p>

The whitespace between the "`{#`" and "`#}`" delimiters and the tag contents are optional.  Tags may appear anywhere within a line of text and an unlimited number of tags may appear per line.  A tag cannot be split across lines.

Tags are replaced with the text resulting from processing the directive, if any.  The result from a tag is not re-evaluated so if a tag generates what looks like another tag, that result is just written to the output. For instance, the tag:

<p style="margin-left: 18pt">{# print "{# print 42 #}" #}</p>

inserts "{# print 42 #}" into the output, not "42".  Note that the `include` directive may appear to violate this rule but does not--see [include directive](#include_directive) below.

### Example Tag Usage
_Example 1_: Inserting today's data into a document.  Let's say that the input document contains the following:

    This date was last updated on {# print datetime("%B %d, %Y") #} by the daily build pipeline.

Assuming the date was 4/1/2022, this sentence would appear like the following in the output document:

    This date was last updated on April 01, 2022 by the daily build pipeline.

_Example 2_: Inserting text from another file.  Here the text to be inserted is contained in another file and we want to insert that text directly into the output:

    The client version for this release is {# print readline("client_version.txt") #} which should also be the version of the executable.

If the file `client_version.txt` begins with this line:

    v4.1.3

then the above text will appear like this in output document:

    The client version for this release is v4.1.3 which should also be the version of the executable.

_Example 3_: Conditional inclusion of text.  Here there two blocks of text but only one is to appear in the output document depending on a variable set in an included file.  Let's say the input document contains this:

    {# include "document_edition.md" #}
    {# if documentEdition == "internal" #}
    Building 7 doesn't exist.
    {# elif documentEdition == "new_hire" #}
    Free pony rides are available in building 7.
    {# endif #}

If the file `document_edition.md` contains the following:

    {# set documentEdition = "new_hire" #}

then the output document will contain:

    Free pony rides are available in building 7.

However if the file `document_edition.md` contains the following:

    {# set documentEdition = "internal" #}

then the output document will contain:

    Building 7 doesn't exist.

Finally if the file `document_edition.md` contains the following:

    {# set documentEdition = "public" #}

then not will be written to the output document will be empty.

### Conditional directives

The ProjectAirSimPP conditional directives include or exclude text and are similar to the C preprocessor conditional directives.  A conditional block has the following form:
* Starts with an `if`, `ifdef` or `ifndef` directive.
* Optionally followed by multiple `elif`, `elifdef`, `elifndef` directives.
* Includes an optional `else` directive.
* Ends with an `endif` directive.

Only one region within a conditional block is "active", the region belonging to the first directive that evaluates to true.  Text (including tags) within the active region is evaluated further and text inside the nonactive regions are discarded.

Conditional blocks may be nested.  `else` and `endif` directives are associatd with the innermost unterminated `if*` or `else*` directive.  Here's an example of nested blocks:

    {# if ARCHITECTURE == "x86" }
    {# ifdef DEBUG #}
    {# print "32-bit debug" #}
    {# else #}
    {# print "32-bit release" #}
    {# endif #}
    {# elif ARCHITECTURE == "x64" }
    {# ifdef DEBUG #}
    {# print "64-bit debug" #}
    {# else #}
    {# print "64-bit release" #}
    {# endif #}
    {# endif #}

If the value of variable `ARCHITECTURE` is "x64" and the value of variable `DEBUG` is `true`, the string "64-bit debug" is sent to the output document.

Conditional blocks may not span files.  For instance, a document may not contain an `ifdef` directive without a matching `endif` directive and instead `include` a file containing the `endif` directive.  ProjectAirSimPP will issue an error for the `endif` without a matching `if*` directive in the included file and an error for the `ifdef` without a matching `endif` in the parent document.  Note that including files (via `include` directives) within conditional blocks is perfectly OK, like this:

    {# if ARCHITECTURE == 'x86' #}
    {# include x86_info.md #}
    {# else #}
    {# include generic_info.md #}
    {# endif #}

ProjectAirSimPP supports the following conditional `if` directives:

| Directive | Arguments | Description |
| --- | --- | --- |
| `if` |  <i>expression</i> | Region is active if the expression evaulates to `true`.  See [Expressions](#expressions), below.
| `ifdef` | <i>variable_name</i> | Region is active if the variable is defined. |
| `ifndef` | <i>variable_name</i> | Region is active if the variable is not defined. |


The `if` directives also have `elif` versions that may follow an `if` directive or another `elif` directive:

| Directive | Arguments | Description |
| --- | --- | --- |
| `elif` |  <i>expression</i> | Region is active if the expression evaulates to `true`.  See [Expressions](#expressions), below.
| `elifdef` | <i>variable_name</i> | Region is active if the variable is defined. |
| `elifndef` | <i>variable_name</i> | Region is active if the variable is not defined. |

The remaining two directives may only appear once after the `if` directive or after any `elif` directives of a conditional block.  Note that nested conditional blocks are supported.

| Directive | Arguments | Description |
| --- | --- | --- |
| `else` | [<i>if_expression</i>] | Region is active if the preceeding `if` or `elif` directive is not.  This directive may only appear once as the last conditional directive of a conditional block before `endif`. |
| `endif` | [<i>if_expression</i>] | Ends the conditional block. |

The optional <i>if_expression</i> can be given to help indicate the matching `if*` directive.  However when one is given, TulloPP will issue a warning if it does not  match the expression of the correponding `if*`/`elif*` directive.

### Directives for Variables
A variable is an identifier representing a value.  Once defined, a variable can be used instead of literal values in expressions  (see [Expressions](#Expressions), below).

A variable name follows these rules:
* Starts with a letter or underscore ("_")
* Continues with one or more letters, underscores, or numeric characters
* Is no more than 256 characters long

The variable scope where a variable is defined determines the variable's visibility:
* Global scope - The variable is visible everywhere.
* Local scope - The variable is visible in the file setting the variable, in a file included by that file, or in a nested included file.
* File scope - The variable is visible only in the file setting the variable and not visible anywhere else, including in any included files.

Variable scopes are arranged in a stack with the global scope at the bottom and the file scope of the current file at the top.  Local scopes are on the stack just below the corresponding file scopes.  When a file is included (with the `include` directive) a new local scope and new file scope are pushed onto the stack.  When the included file is finished processing, its local and file scopes are popped off the stack.

Variables with the same name may exist in different  rscopes simultaneously.  When a variable is referenced the scope stack is searched starting with the most recent file scope at the top and ending with the global scope at the bottom.  The first matching entry found is used.

The variable-related directives are as follows:

| Directive | Arguments | Description |
| --- | --- | --- |
| `define` | <i>variable</i> [`=` <i>expr</i>] | Set the variable in the global scope to the value of the expression. |
| `export` | <i>variable</i> [`=` <i>expr</i>] | Set the variable in the local scope of the parent of the current file to the value of the expression. |
| `set` | <i>variable</i> [`=` <i>expr</i>] | Set the variable in the current file's local scope to the value of the expression. |
| `setlocal` | <i>variable</i> [`=` <i>expr</i>] | Set the variable in the current file's file scope to the value of the expression. |
| `undef` | <i>variable</i>  | Removes the variable from the first visible scope found containing it. |

If the optional expression is not given, the variable is set to 1.

### Print Directive
The `print` directive outputs text that is inserted in place of the tag.  This is the primary way programmatically generated text is inserted into the document.

| Directive | Arguments | Description |
| --- | --- | --- |
| `print` | [<i>expr</i>[`,` <i>expr</i>...]] | Evaluates the expression(s) and replaces the tag with the result. |

If no expressions are given, the `print` directive inserts a blank line.  When multiple comma-separated expressions are listed, their string values are output in the same order after each other without separators.  Note that a `print` directive can't be used to generate tags that are evaluated further--the output of the `print` directive is not reevaluated and a tag output by a `print` directive is simply inserted into the output document.

### Include Directive
The `include` directive inserts the contents of another file into the current document.  Unlike the `print` directive, any tags in the included file are processed before inserting the contents into the output.

| Directive | Arguments | Description |
| --- | --- | --- |
| `include` | string <i>filename_expr</i> | Preprocess the contents of the file and insert into the output document. |

<i>filename_expr</i> is a name of a file (with or without quotes) or an expression that evaluates to string.  Note that a variable with an identical name as the file will cause the variable's value to be used instead of the intended filename; however, filenames with a dot extension (like "main.md") aren't valid variable names and so avoid this issue.

For example:

    {# include client_version.txt #}
    {# include "client_version.txt" #}

    {# set CLIENT_VERSION_FILE = "client_version.txt" #}
    {# include CLIENT_VERSION_FILE #}

    {# include concat("client_version", ".txt") #}

All four examples include the file "client_version.txt".  The first specifies the filename directly while the other three specifies the filename using expressions.

Local and files variable scopes are pushed onto the scope stack for the included file while it is being preprocessed.  The included file may `include` other files, including itself, up to the `include_nest_maximum` parameter limit.

### Log Directive
The `log` directive writes a message to the logger output.

| Directive | Arguments | Description |
| --- | --- | --- |
| `log` | <i>severity</i>, <i>expr</i> | Writes the string value of the expression to the log with the specified sevrity level which must be `info`, `warning`, `error`, or `fatal`.  `fatal` will also raise a runtime exception, halting further processing of the document. |

If needed, the log output is automatically preceded by additional output that help indicate in which document and where in that document the log directive was invoked.

When ProjectAirSimPP is run stand-alone, the logger output is sent to the console before the document output, if any.  When ProjectAirSimPP is run as a preprocessor in `mkdocs`, the logger output destination is determined by `mkdocs`.

### Expressions
ProjectAirSimPP expressions are similar to C language expressions.  When an expression is evaluated it will result in one of four different types of values:

* Floating-point number
* Integer number
* Boolean value
* Character string

Generally, there is little distinction between float-point and integer numbers and they are handled in the same way.

When an expression is evaluated the variables, operators, and function calls are repeatly evaluated and replaced by their results until a literal value remains.  The result should be a single value--multiple values would generate an error.

#### Type conversion
If an operator or function argument requires a value of a specific type, the value is converted using the following rules.  For conversion, floating-point and integer numbers are treated the same.

| Original Type | Target Type | Resulting Value |
| ------------- | ----------- | --------------- |
| Number | Boolean | 0 becomes `false`, all other values become `true` |
| Number | String | The value is formatted using the "%g" C printf-style formatting specification. |
| Boolean | Number | `true` becomes 1, `false` becomes 0 |
| Boolean | String | `false` becomes `"false"`, `true` becomes `"true"`. |
| String | Number | String is parsed using the "%g" C scanf-style format, 0.0 if the string is not a valid floating-point number string. |
| String | Boolean | Empty string becomes `false`, anything else becomes `true`. |


#### Literals
ProjectAirSimPP recognizes the following literal values:
| Literal Type | Format | Description |
| ------------ | ------ | ----------- |
| Boolean | `true` or `false` | Boolean value.
| Number | i[`.`f] | Numeric value where *i* is one or more decimal digits for the integer part and *f* is one or more decimal digits for the fractional part. |
| String | `"`[c...]`"` | C-style character string including character escaping using the backslash character ("\\") and standard character escape sequences. |

#### Variables

Variables may be used anywhere a literal value can be used, but variables must be defined or set before they are evaluated.

The format for variable names is described above in the section [Directives for Variables](#directives_for_variables).  When a variable name is evaluated, the variable scopes are searched for the name starting at the top of the variable scope stack which contains the file scope of the current file.  The value of the first entry found replaces the variable name.  If the variable is not defined, an error is logged and a value of 0 is used.

#### Operators
Operators perform basic operations in an expression.  The following operators are supported:

| Operator | Type | Precedence | Description |
| -------- | ---- | ---------- | ----------- |
| `-` | Unary | 1 | For numeric and boolean values, negates the value on the right-hand side.  For strings, returns an empty string. |
| `+` | Unary | 1 | Similar to negation operator but essentially has no effect and just returns the right-hand value. |
| `!` | Unary | 1 | Converts right-hand value to a boolean value and returns the negated value. |
| `+` | Binary | 3 | For numeric values, adds the two values.  For strings, returns the concatenated string.  For booleans, returns the logical-OR. |
| `-` | Binary | 3 | For numeric values, subtracts the right value from the left.  For strings, removes the first instance of the right string from the left.  For booleans, returns whether the right is implied by the left. |
| `*` | Binary | 2 | For numeric values, multiplies the two values.  For boolean values, returns the logical-AND.  If one value is a string, repeats the string the number of times indicated by the other (numeric) value. |
| `/` | Binary | 2 | For numeric values, divides the left value by the right.  For booleans, returns the logical-XOR. For strings, returns an empty string. |
| `%` | Binary | 2 | For numeric values, return the modulus of the left value divided by the right.  For booleans, returns false.  For strings, returns an empty string. |
| `<`, `<=`, `==`, `>`, `>=` | Binary | 4 | C-style comparison operators.  Both sides are converted to boolean values first. |
| `\|\|` | Binary | 5 | Shortcut OR.  If the left side value is true, the result is true without evaluating the right side.  If the left side is false, the result is the boolean value of the right side. |
| `&&` | Binary | 5 | Shortcut AND.  If the left sidevalue is false, the result is false without evaluating the right side.  If the left side is true, the result is the boolean value of the right side. |
| `? :` | Ternary | 6 | If the left boolean value is true, returns the value of the middle value and the right value is not evaluated.  If the left boolean expression is false, returns the value of the right expression and the middle expression is not evaluated.  For instance, "(PLATFORM == "x64") ? 64 : 32" returns 64 if PLATFORM is equal to "x64", 32 otherwise. |

The operator percedence in the table indicates which operator is evaluated first when a subexpression would be input to two operators.  A lower precedence number is higher precedence and the operator with lower precedence number would be evaluated first.  If two operators have the same precedence, generally the operator on the left is evaluated first.  The conditional ternary operator is slightly different: when the ternary operators are nested, inner ternary operators have precedence.

Parenthesis may be used to group parts of an expression and override oeprator precedence.  The grouped subexpression is evaluated before being used as part of another subexpression.

_Example 1_: Two operators with different precedences.  In the expression:

    1 + 5 * 4

If evaluating the expression left to right, the `+` operator would be evaluated first and the expression equals 1 + 5 * 4 = 6 * 4 = 24.  However the `*` operator has higher precedence than the `+` operator and should be evaluated first, giving the correct answer of 1 + 5 * 4 = 1 + 20 = 21.

_Example 2_: Two operator with the same precedence.  In the expression:

    5 % 2 * 2

The `%` and `*` operators have the same precedence.  If the `*` operator was evaluated first, the result would be 5 % 2 * 2 = 5 % 4 = 1.  However since both operators have the same precedence, the operator are evaluated left-to-right, thus the `%` operator is evaluated first giving the result of 5 % 2 * 2 = 1 * 2 = 2.

_Example 3_: Using parentheses to override precedence.  In the expression:

    (1 + 5) * 4

Again the `*` operator has higher precedence than the `+` operator, but here the parentheses delimits the `+` subexpression.  The parenthetical subexpression has higher precedence and must be evaluated first before the `*` operator so the result is (1 + 5) * 4 = 6 * 4 = 24.

#### Functions
In an expression functions can be invoked to perform useful operation such as string manipulation, generating date and time strings, and even reading content from files.  To invoke a function, give the function name followed an open-parenthesis, arguments (if any), and a close-parenthesis.  For instance:

```
datetime("%Y%m%d")
```
would return the current date as a string in YYYYMMDD format.  Variable names given as arguments are replaced by their values as usual except for the `defined()` function which takes the variable name directly to determine whether the variable is defined.

  The following functions are available:

| Name | Parameters | Returns | Description |
| ---- | ---------- | ------- | ----------- |
| `bool` | any | boolean | Returns the argument converted tnto a boolean value. |
| `capitalize` | string | string | Returns a string where the first letter of the first word is upper case and all remaining characters are lower case. |
| `ceil` | number | number | Returns the next higher integer value if the parameter is not an integer. |
| `compactws` | string | string | Repeated whitespace characters in the string are replaced by a single whitespace character. |
| `concat` | string, ... | string | The arguments are assembled in the order given into a single string. |
| `datetime` | [string] | string | Return the current date and time as a string.  The optional parameter is a strftime() format string.  If the parameter is not specified, "%c" is used.  |
| `defined` | <i>variable_name</i> or string <i>variable_name</i> | boolean | Return true if the variable is defined, false otherwise.  Note that the variable name may be given as a variable name or as a literal string with the variable name (e.g., 'defined(PLATFORM)' or 'defined("PLATFORM")') but not as an expression. |
| `field` | string <i>source</i>, string <i>delimiter</i>, int <i>index</i> | string | Divides the source string into fields at occurances of the delimiter string, and returns the field at the specified index where 0 is the first field.  The delimiter string is not part of the fields. |
| `field_count` | string <i>source</i>, string <i>delimiter</i> | int | Returns the number of fields when the source is separated into fields at each occurance of the delimiter string. |
| `find` | string <i>source</i>, string <i>target</i> | int | Returns the index of the first occurance of the target string in the source string with 0 being the index of the first character. |
| `float` | any | float | Returns the argument converted into a floating-point value. |
| `floor` | number | float | If the argument is not an integer value, returns the next lower integer value. |
| `format` | string <i>format</i>, [any <i>insert</i>...] | string | Generates a string based on the Python-style format string.  Values of the insertion markers in the format string are supplied by corresponding insert parameters. |
| `int` | any | int | Returns the argument converted into an integer value.  The fractional part of floating-point values are dropped, moving towards zero. |
| `len` | string | int | Returns the number of characters in the string. |
| `lower` | string | string | Returns a string with all upper-case characters converted to lower case. |
| `readfile` | string <i>filename</i> | string | Returns the contents of the file as a string. |
| `readfileline` | string <i>filename</i> | string | Returns the contents of the file up to but not including the first line terminator as a string. |
| `regex` | string <i>expr</i>, string <i>source</i> | string | Returns the string matching the Python-style regular expression <i>expr</i> from the source string.  If no match is found, an empty string is returned. |
| `str` | any | string | Returns the argument converted into a string. |
| `strip` | string | string | Returns the string with any leading or trailing whitespace are removed. |
| `substr` | string <i>source</i>, int <i>start</i> [, int <i>end</i>] | string | Returns the portion of source string starting at the character at index start (first is 0), and ending just before the character at index <i>end</i>.  If the optional <i>end</i> argument is not given, the portion through the end of the source string is returned. |
| `translate` | string <i>source</i>, string <i>from</i>, string <i>to</i> | string | Returns a string where each character in the source string that's in the <i>from</i> string is replaced by the corresponding character in the <i>to</i> string.|
| `upper` | string | string | Returns a string with all lower-case characters converted to upper case. |


---

Copyright (C) Microsoft Corporation.  All rights reserved.
