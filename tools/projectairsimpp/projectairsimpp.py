"""
Copyright (C) Microsoft Corporation. All rights reserved.
Text preprocessor Markdown extension that implements C-style preprocessing
"""
import argparse
import io
import logging
import os
import pathlib
import re

from enum import Enum

import markdown
import markdown.preprocessors

import expr


# Logger for ProjectAirSimPP
LOGGER_NAME = "ProjectAirSimPP"
log = logging.getLogger(LOGGER_NAME)
log.setLevel(logging.WARNING)

ch = logging.StreamHandler()
ch.setFormatter(logging.Formatter("%(levelname)s: %(name)s%(message)s"))
log.addHandler(ch)


class IncludeNestLimitError(Exception):
    """Exception class when include files count hits the nesting limit"""

    pass


class ProjectAirSimExtension(markdown.Extension):
    """ProjectAirSimPP Markdown extension setup and registration handler"""

    def __init__(self, **kwargs):
        self.config = {
            "include_paths": [
                "",
                "Semi-colon separated list of directories to search for included files.",
            ],
            "log_level": "info",
        }
        super().__init__(**kwargs)

    def extendMarkdown(self, md: markdown.Markdown) -> None:
        self.md = md
        md.registerExtension(self)
        md.preprocessors.register(
            ProjectAirSimPreprocessor(md, include_paths=self.getConfig("include_paths")),
            "projectairsimpp",
            200,
        )


class IfTag:
    """State of an If/ifdef/ifndef/else/elif/elif/elifndef block"""

    class Skip(Enum):
        """Skip action state"""

        NONE = 0  # Not skipping
        BLOCK = 1  # Skipping the current block
        TO_END = 2  # Skipping until matching end tag

    class Tag(Enum):
        """If tag"""

        IF = 0
        IFDEF = 1
        IFNDEF = 2
        ELSE = 3
        ELIF = 4
        ELIFDEF = 5
        ELIFNDEF = 6

    # Mapping from tag ID to string
    map_tag_str = {
        Tag.IF: "if",
        Tag.IFDEF: "ifdef",
        Tag.IFNDEF: "ifndef",
        Tag.ELSE: "else",
        Tag.ELIF: "elif",
        Tag.ELIFDEF: "elifdef",
        Tag.ELIFNDEF: "elifndef",
    }

    @property
    def tag_name(self):
        """Return the tag"""
        return self.map_tag_str[self.tag]

    def __init__(self, iline: int, skip: Skip, tag: Tag = Tag.IF, comment: str = None):
        self.iline = iline  # Line where this tag was encountered
        self.skip = skip  # How the block contents are skipped or not
        self.tag = tag  # The tag ID
        self.comment = (
            comment  # The variable name or expression associated with the tag
        )


class IfTagState:
    """Manages overall if/ifdef/etc. block state"""

    @property
    def is_empty(self):
        """Returns whether the stack is empty"""
        return len(self.stack_if_tag) < 1

    @property
    def is_skipping(self):
        """Return whether the current block is being skipped"""
        return self.skip_count > 0

    @property
    def is_outer_skipping(self):
        """Returns whether block skip is because of a stack entry other than the innermost"""
        return (self.skip_count > 0) and not self.innermost.is_skipping

    @property
    def innermost(self):
        """Returns the innermost stack entry"""
        return self.stack_if_tag[-1]

    def __init__(self):
        self.stack_if_tag: list[IfTag] = []  # Stack of block states
        self.skip_count = 0  # How many blocks on the stack are skipping their contents

    def pop(self):
        """Removes and returns te innermost entry"""
        if self.innermost.skip != IfTag.Skip.NONE:
            self.skip_count -= 1
        if self.skip_count < 0:
            raise RuntimeError("self.skip_count < 0")

        return self.stack_if_tag.pop()

    def push(
        self,
        iline: int,
        is_skipping: bool,
        tag: IfTag.Tag = IfTag.Tag.IF,
        comment: str = None,
    ):
        """Adds a new innermost entry"""
        self.stack_if_tag.append(
            IfTag(
                iline=iline,
                skip=IfTag.Skip.BLOCK if is_skipping else IfTag.Skip.NONE,
                tag=tag,
                comment=comment,
            )
        )
        if is_skipping:
            self.skip_count += 1

    def set(
        self,
        is_skipping: bool,
        tag: IfTag.Tag = None,
        iline: int = None,
        comment: str = None,
    ):
        """Updates the properties of the innermost entry on the stack.

        Arguments:
            is_skipping - If True, enables skipping; if False, disables skipping
            tag - (Optional) Sets the new tag for the current entry
            iline - (Optional) Sets the new source line for the current entry
            comment - (Optional) Sets the new comment for the current entry
        """
        if_tag = self.stack_if_tag[-1]
        if if_tag.skip != IfTag.Skip.NONE:
            self.skip_count -= 1

        if tag:
            if_tag.tag = tag
        if iline:
            if_tag.iline = iline
        if comment is not None:
            if_tag.comment = comment

        # Update tag's skip state--if skipping now and was not skipping, it can now be set to not skipping
        if if_tag.skip == IfTag.Skip.NONE:
            if is_skipping:
                if_tag.skip = IfTag.Skip.TO_END
        elif if_tag.skip == IfTag.Skip.BLOCK:
            if not is_skipping:
                if_tag.slip = IfTag.Skip.NONE

        if if_tag.skip != IfTag.Skip.NONE:
            self.skip_count += 1


class ProjectAirSimPreprocessor(markdown.preprocessors.Preprocessor):
    """ProjectAirSimPP text preprocessor that support {#...#}-style tags for conditional text inclusion, text insertion from files, and programmatic text creation"""

    re_tag_start = re.compile("\{#")

    class IncludedFile:
        """Information about a file being included"""

        def __init__(self, filename: str, line_including: int):
            self.filename = filename  # Name of file that was included
            self.line_including = line_including  # Line containing include tag in the file (the previous IncludedFile entry) that included this file
            self.line_start = 0  # Line in the aggregate lines array where the insertion of this file started

    class ListLineSource:
        """Text line source from a list of text lines"""

        def __init__(self, lines):
            self.lines = lines  # List of text lines
            self.iline_next = 0  # Index of the next line to return
            self.cline = len(lines)  # Total number of lines in the list

        def get_file_name(self):
            """Returns the associated filename, if any"""
            return None

        def get_log_context(self):
            """Returns the associated line context appropriate for a log entry, if any"""
            return f"{self.lines[0]}"

        def get_log_line_cur(self):
            """Returns the current line number where 1 is the first line"""
            return self.iline_next

        def get_next(self):
            """Returns the next source line"""
            line = None
            if self.iline_next < self.cline:
                line = self.lines[self.iline_next]
                self.iline_next += 1

            return line

    class FileLineSource:
        """Text line source from a file"""

        def __init__(self, filename: str, file: io.IOBase):
            self.file = file  # Handle to the file
            self.filename = filename  # Name of file
            self.iline_cur = 0  # Index of the next line to be returned

        def get_file_name(self):
            """Returns the associated filename, if any"""
            return self.filename

        def get_log_context(self):
            """Returns the associated line context appropriate for a log entry, if any"""
            return f"{self.filename}"

        def get_log_line_cur(self):
            """Returns the current line number where 1 is the first line"""
            return self.iline_cur

        def get_next(self):
            """Returns the next source line"""
            line = self.file.readline()
            if line is not None:
                if len(line) < 1:
                    line = None
                else:
                    self.iline_cur += 1
                    if line[-1] == "\n":
                        line = line[:-1]

            return line

    def __init__(self, *args, **kwargs):
        self.include_paths = None  # Directories to search for included files
        self.include_nest_max = 25  # Maximum number of nested included files

        if "include_nest_limit" in kwargs:
            if kwargs["include_nest_limit"]:
                self.include_nest_max = int(kwargs["include_nest_limit"])
            del kwargs["include_nest_limit"]

        if "include_paths" in kwargs:
            if kwargs["include_paths"]:
                self.include_paths = kwargs["include_paths"].split(";")
            del kwargs["include_paths"]

        # Set logger to the specified logging severity level
        if "log_level" in kwargs:
            if kwargs["log_level"]:
                log_level = kwargs["log_level"].lower()

                if log_level == "debug":
                    log.setLevel(logging.DEBUG)
                elif log_level == "info":
                    log.setLevel(logging.INFO)
                elif log_level == "warning":
                    log.setLevel(logging.WARNING)
                elif log_level == "error":
                    log.setLevel(logging.ERROR)
                else:
                    log.error(
                        'config parameter "log_level" value "{log_level}" is not "info", "warning", "error", or "debug"'
                    )
            del kwargs["log_level"]

        super().__init__(*args, **kwargs)

    def run(self, lines):
        """Runs the text lines through the preprocessor

        Arguments:
            lines - List of lines

        Returns:
            (Return) List of lines after preprocessing
        """
        return self.process_line_source(self.ListLineSource(lines))

    def process_line_source(self, line_source):
        """Preprocess a source of text lines

        Arguments:
            line_source - Source of text lines

        Returns:
            (Return) List of lines after preprocessing
        """
        self.log_fn_last = None  # Last logging output function used
        self.log_level_last = 100  # Severify level of the last log message
        self.log_source_last = (
            None  # Line source that was active for the last log message
        )
        self.log_line_last = -1  # Source line index of the last log message
        self.stack_line_source = []  # Stack of active line sources
        self.variables = expr.Variables()  # Variables
        self.variable_scope_global = expr.VariableScope()  # Global variables
        self.variable_scope_local = None  # Inheritable variables
        self.variable_scope_local_only = None  # Non-inherited variables
        self.variable_scope_parent = None  # Local scope of parent document
        self.expr_evaluator = expr.ExprEvaluator(self.variables, self.include_paths)

        self.variables.push_scope(self.variable_scope_global)

        return self._process_lines(line_source)

    def _check_else_ok(self, if_tag_state: IfTagState, tag_name) -> bool:
        """Returns whether an else* directive is valid at this point

        Arguments:
            if_tag_state - Current if tag state
            tag_name - Name of the else* directive

        Returns:
            (Return)  True if an else* directive is valid here, False otherwise
        """
        is_ok = not if_tag_state.is_empty
        if not is_ok:
            self._log_error(
                f"{tag_name} tag was not preceeded by a matching if, elif, or elifdef tag"
            )
        else:
            if_top = if_tag_state.innermost
            tag_active = if_top.tag
            is_ok = tag_active != IfTag.Tag.ELSE
            if not is_ok:
                self._log_error(
                    f"{tag_name} tag was preceeded by else tag and not a matching if, elif, or elifdef tag"
                )

        return is_ok

    def _eval_expr(self, expr: str, is_arg_list: bool = False):
        """Evaluates an expression

        Arguments:
            expr - Expression string
            is_arg_list - If True, allow comma-separated expressions

        Returns:
            (Return)  If is_arg_list is true, a list of values from the
                      evaluating the comma-separated expressions; if false the
                      value from evaluating the expression
        """
        return self.expr_evaluator.evaluate(str_expr=expr, is_arg_list=is_arg_list)

    def _find_tag(self, line: str):
        """Locate the next next tag in the text line, if any

        Arguments:
            line - Text line to search

        Returns:
            (Return)  (First character, character after last character) of the tag,
                      or (-1, -1) if there's no tag found
        """
        ich_tag_start = -1
        ich_tag_end = -1

        match = self.re_tag_start.search(line)
        if match:
            ich_start = match.end(0)

            is_escaped = False
            is_maybe_tag_end = False
            quote_delimiter = False

            # Locate end of tag ignoring characters that are escaped or quoted
            for ich in range(ich_start, len(line)):
                ch = line[ich]
                if is_escaped:
                    is_escaped = False
                elif ch == "\\":
                    is_escaped = True
                    is_maybe_tag_end = False
                elif quote_delimiter:
                    if ch == quote_delimiter:
                        quote_delimiter = False
                else:
                    if is_maybe_tag_end:
                        if ch == "}":
                            # Found the end of the tag
                            ich_tag_start = ich_start - 2
                            ich_tag_end = ich + 1
                            break

                        is_maybe_tag_end = False

                    if ch == "#":
                        is_maybe_tag_end = True
                    elif (ch == '"') or (ch == "'"):
                        quote_delimiter = ch

            if ich_tag_start >= ich_tag_end:
                self._log_warning('Tag start "{#" found, but not matching tag end "#}"')

        return ich_tag_start, ich_tag_end

    def _get_log_line_cur(self):
        """Return the line number appropriate for a log entry"""
        return self.stack_line_source[-1].get_log_line_cur()

    def _log_expr_error(self, message: str, e: Exception, str_expr: str, line=True):
        """Create a log entry for an exception from evaluating an expression with
        additional entries to give the location context.

        Arguments:
            message - Log message
            e - Exception object to log
            str_expr - The expression
            line - (Optional) Line number containing the expression,
                    True to use the current line source line number,
                    False to omit the line number from the log entry
        """
        log_level = log.getEffectiveLevel()
        if log_level <= logging.ERROR:
            self._log_with_context(message, line, log.error)

            if hasattr(e, "ich_first"):
                self._log_without_context(f"  where:  {str_expr}", line, log.error)
                message = "          "
                if e.ich_first >= 0:
                    if e.ich_first > 0:
                        message += " " * e.ich_first
                    message += "^"
                if hasattr(e, "ich_last"):
                    dch = e.ich_last - e.ich_first - 1
                    if dch >= 0:
                        if dch > 0:
                            message += "-" * dch
                        message += "^"
                self._log_without_context(message, line, log.error)

            self.log_level_last = logging.ERROR

    def _log_debug(self, message: str, line=True):
        """Log a debug-severity message.  If needed log entries to give
        context for the line location are also added.

        Argument:
            message - Log entry message
            line - (Optional) Line number containing the expression,
                    True to use the current line source line number,
                    False to omit the line number from the log entry
        """
        log_level = log.getEffectiveLevel()
        if log_level <= logging.DEBUG:
            self._log_with_context(message, line, log.debug)
            self.log_level_last = logging.DEBUG

    def _log_error(self, message: str, line=True):
        """Log an error-severity message.  If needed log entries to give
        context for the line location are also added.

        Argument:
            message - Log entry message
            line - (Optional) Line number containing the expression,
                    True to use the current line source line number,
                    False to omit the line number from the log entry
        """
        log_level = log.getEffectiveLevel()
        if log_level <= logging.ERROR:
            self._log_with_context(message, line, log.error)
            self.log_level_last = logging.ERROR

    def _log_info(self, message: str, line=True):
        """Log an informational-severity message.  If needed log entries to give
        context for the line location are also added.

        Argument:
            message - Log entry message
            line - (Optional) Line number containing the expression,
                    True to use the current line source line number,
                    False to omit the line number from the log entry
        """
        log_level = log.getEffectiveLevel()
        if log_level <= logging.INFO:
            self._log_with_context(message, line, log.info)
            self.log_level_last = logging.INFO

    def _log_warning(self, message: str, line=True):
        """Log a warning-severity message.  If needed log entries to give
        context for the line location are also added.

        Argument:
            message - Log entry message
            line - (Optional) Line number containing the expression,
                    True to use the current line source line number,
                    False to omit the line number from the log entry
        """
        log_level = log.getEffectiveLevel()
        if log_level <= logging.WARNING:
            self._log_with_context(message, line, log.warning)
            self.log_level_last = logging.WARNING

    def _log_with_context(self, message: str, line, fn_log):
        """Log a message.  If needed log entries to give
        context for the line location are also added.

        Argument:
            message - Log entry message
            line - Line number containing the expression,
                    True to use the current line source line number,
                    False to omit the line number from the log entry
            fn_log - Function to create the log entry
        """
        log_source_changed = self.log_source_last != self.stack_line_source[-1]
        log_context_changed = log_source_changed or (
            self.log_line_last != self.log_source_last.get_log_line_cur()
        )

        if log_context_changed:
            csource = len(self.stack_line_source)
            if csource < 1:
                self.log_source_last = None
                self.log_line_last = 0

            elif log_source_changed or (fn_log != self.log_fn_last):
                is_first_context = True
                isource = 0 if log_source_changed else (csource - 1)
                iline_of_previous = 0
                while isource < csource:
                    line_source = self.stack_line_source[isource]

                    filename = line_source.get_file_name()
                    context_message = "In the " if is_first_context else f"included "
                    if not filename:
                        context_message += f'document beginning with "{line_source.get_log_context()}":'
                    else:
                        context_message += f'file "{line_source.get_log_context()}":'

                    if is_first_context:
                        fn_log(f": " + context_message)
                        is_first_context = False
                        iline_of_previous = line_source.get_log_line_cur()
                    else:
                        fn_log(f":->  (line {iline_of_previous}) {context_message}")
                        iline_of_previous = line_source.get_log_line_cur()

                    isource += 1

                self.log_fn_last = fn_log
                self.log_source_last = self.stack_line_source[-1]
                self.log_line_last = self.log_source_last.get_log_line_cur()

        self._log_without_context(message, line, fn_log)

    def _log_without_context(self, message: str, line, fn_log):
        """Log a message.  If needed log entries to give
        context for the line location are also added.

        Argument:
            message - Log entry message
            line - Line number containing the expression,
                    True to use the current line source line number,
                    False to omit the line number from the log entry
            fn_log - Function to create the log entry
        """
        if isinstance(line, bool):
            line = (
                self.stack_line_source[-1].get_log_line_cur()
                if (self.stack_line_source and line)
                else None
            )
        fn_log(f" -  (line {line}) {message}" if line else f"    {message}")

    def _open_include_file(self, filename: str, access: str):
        """Search for the specified file in the file path lists and open it

        Arguments:
            filename - Name of file to open
            access - Python-style file open access specifier
            ls - LexSym of the filename (for error reporting)

        Returns:
            (return) File object if file found, None otherwise
        """
        if not self.include_paths:
            return open(filename, access)
        else:
            file = None
            for include_path in self.include_paths:
                path_file = os.path.abspath(
                    pathlib.PurePath(include_path).joinpath(filename)
                )
                try:
                    self._log_debug(f"Trying to open file {path_file}")
                    file = open(path_file, access)
                    break
                except Exception as e:
                    self._log_debug(f"File {path_file}: {e}")

            if file is None:
                raise FileNotFoundError(
                    f'Can\'t open include file "{filename}" from the include paths'
                )

            return file

    def _process_lines(self, line_source):
        """Process the text lines from the line source

        Arguments:
            line_source - Source to text lines

        Returns:
            (Return) Lines of preprocessed text lines
        """
        lines_processed = []
        if_tag_state = IfTagState()  # if-else-endif tag section state

        self.stack_line_source.append(line_source)

        # Hide parent document's local-only scope
        if self.variable_scope_local_only is not None:
            is_hidden_local_only_parent_sav = self.variable_scope_local_only.is_hidden
            self.variable_scope_local_only.is_hidden = True

        # Save parent document variables scopes
        variable_scope_local_only_sav = self.variable_scope_local_only
        variable_scope_parent_sav = self.variable_scope_parent
        self.variable_scope_parent = self.variable_scope_local

        # Install our own document variables scopes
        self.variable_scope_local = (
            expr.VariableScope()
        )  # Local variables inheritable by included files
        self.variable_scope_local_only = (
            expr.VariableScope()
        )  # Local variables not inheritable by included files
        self.variables.push_scope(self.variable_scope_local)
        self.variables.push_scope(self.variable_scope_local_only)

        try:
            while True:
                line = line_source.get_next()
                if line is None:
                    break  # End of source

                if not line and not if_tag_state.is_skipping:
                    # Output blank lines
                    lines_processed.append("")
                    continue

                text_pending = None  # Buffer for processed text waiting to be output

                while line:
                    # Get the next tag
                    ich_start, ich_end = self._find_tag(line)
                    if ich_start >= ich_end:
                        if if_tag_state.is_skipping:
                            line = None
                        break

                    # Process the tag
                    if (ich_start > 0) and not if_tag_state.is_skipping:
                        # Move text before the tag to the pending buffer
                        if text_pending:
                            text_pending = text_pending + line[:ich_start]
                        else:
                            text_pending = line[:ich_start]
                    text_after = line[ich_end:]
                    lines_replace = self._process_tag(
                        line[ich_start:ich_end], if_tag_state
                    )

                    # Replace the tag with the lines returned; the lines returned are not processed again.
                    # If the tag was the entire remainder of the line, drop the line
                    if lines_replace:
                        clines_replace = len(lines_replace)
                        if clines_replace == 1:
                            # Insert replacement text
                            if text_pending:
                                text_pending = text_pending + lines_replace[0]
                            else:
                                text_pending = lines_replace[0]
                        elif clines_replace > 1:
                            # Append replacement lines to output, prepending and appending text surrounding tag to first and last replacement lines
                            iline_insert_begin = 0
                            iline_insert_end = len(lines_replace)

                            # If tag started in the middle of the line, output the first replacement line appended to the text pending
                            if text_pending:
                                lines_processed.append(text_pending + lines_replace[0])
                                text_pending = None
                                iline_insert_begin = 1

                            # If tag ended in the middle of the line, make the last replacement line the pending text
                            if text_after:
                                text_pending = lines_replace[-1]
                                iline_insert_end -= 1

                            # Add the replacement lines to the output lines
                            lines_processed += lines_replace[
                                iline_insert_begin:iline_insert_end
                            ]

                    # Continue processing with the text after the tag, if any
                    line = text_after

                # Output results, if any
                if line:
                    if text_pending:
                        text_pending = text_pending + line
                    else:
                        text_pending = line

                if text_pending is not None:
                    lines_processed.append(text_pending)

            # Check to see if we stil have open if/else/elif tags
            while not if_tag_state.is_empty:
                iftag = if_tag_state.innermost
                self._log_error(
                    f"{iftag.tag_name} tag is missing matching endif tag", iftag.iline
                )

                for tag in if_tag_state.stack_if_tag:
                    self._log_debug(f"Tag = {tag.tag}, iline = {tag.iline}")

                if_tag_state.pop()

        finally:
            self.stack_line_source.pop()

            # Restore variable stores
            self.variables.pop_scope()  # Local-only variables
            self.variables.pop_scope()  # Local variables

            self.variable_scope_local = self.variable_scope_parent
            self.variable_scope_parent = variable_scope_parent_sav
            self.variable_scope_local_only = variable_scope_local_only_sav
            if self.variable_scope_local_only is not None:
                self.variable_scope_local_only.is_hidden = (
                    is_hidden_local_only_parent_sav
                )

        return lines_processed

    def _process_tag(self, tag: str, if_tag_state: IfTagState):
        """Process the tag

        Arguments:
            tag - The tag to process
            if_tag_state - If-tag state

        Returns:
            (Return) - Text string or list of strings to replace the tag or None
        """
        lines_replace = None
        command = tag[2:-2].strip()

        self._log_debug(
            f'Processing command "{command}" (is_skipping = {if_tag_state.is_skipping})'
        )

        ich = command.find(" ")
        if ich > 0:
            tag_name = command[:ich]
            args = command[ich:].lstrip()
        else:
            tag_name = command
            args = ""

        if tag_name == "if":
            if not args:
                self._log_error(f"{tag_name} tag requires an expression")
            else:
                try:
                    is_skipping = (
                        True
                        if if_tag_state.is_skipping
                        else not bool(self._eval_expr(args))
                    )
                except Exception as e:
                    self._log_expr_error(
                        f"{tag_name} tag expression is not valid: {e}", e, args
                    )
                    is_skipping = True

                if_tag_state.push(
                    iline=self._get_log_line_cur(),
                    is_skipping=is_skipping,
                    comment=args,
                )
                self._log_debug(
                    f"{tag_name} block is {'being skipped' if if_tag_state.is_skipping else 'enabled'}"
                )

        elif tag_name == "ifdef":
            if not args:
                self._log_error(f"if{tag_name}def tag requires a variable name")
            else:
                try:
                    self.expr_evaluator.validate_variable_name(args)
                    is_skipping = (
                        True if if_tag_state.is_skipping else args not in self.variables
                    )
                except Exception as e:
                    self._log_error(
                        f'{tag_name} tag variable name "{args}" is not valid: {e}'
                    )
                    is_skipping = True

                if_tag_state.push(
                    tag=IfTag.Tag.IFDEF,
                    iline=self._get_log_line_cur(),
                    is_skipping=is_skipping,
                    comment=args,
                )
                self._log_debug(
                    f"{tag_name} block is {'being skipped' if if_tag_state.is_skipping else 'enabled'}"
                )

        elif tag_name == "ifndef":
            if not args:
                self._log_error(f"{tag_name} tag requires a variable name")
            else:
                try:
                    self.expr_evaluator.validate_variable_name(args)
                    is_skipping = (
                        True if if_tag_state.is_skipping else args in self.variables
                    )
                except Exception as e:
                    self._log_error(
                        f'{tag_name} tag variable name "{args}" is not valid: {e}'
                    )
                    is_skipping = True

                if_tag_state.push(
                    tag=IfTag.Tag.IFNDEF,
                    iline=self._get_log_line_cur(),
                    is_skipping=is_skipping,
                    comment=args,
                )
                self._log_debug(
                    f"{tag_name} block is {'being skipped' if if_tag_state.is_skipping else 'enabled'}"
                )

        elif tag_name == "else":
            # Ignore args, which can be used a comment
            if self._check_else_ok(if_tag_state, tag_name):
                if args:
                    if args != if_tag_state.innermost.comment:
                        self._log_warning(
                            "{tag_name} tag comment does not match if/elif/ifdef/ifndef/elifdef/elifndef tag expression"
                        )

                if_tag_state.set(
                    tag=IfTag.Tag.ELSE,
                    iline=self._get_log_line_cur(),
                    is_skipping=not if_tag_state.is_skipping,
                )
                self._log_debug(
                    f"{tag_name} block is {'being skipped' if if_tag_state.is_skipping else 'enabled'}"
                )

        elif tag_name == "elif":
            if self._check_else_ok(if_tag_state, tag_name):
                try:
                    is_skipping = (
                        True
                        if (if_tag_state.innermost.skip == IfTag.Skip.BLOCK)
                        else not bool(self._eval_expr(args))
                    )
                except Exception as e:
                    self._log_expr_error(
                        f"{tag_name} tag expression is not valid: {e}", e, args
                    )
                    is_skipping = True

                if_tag_state.set(
                    tag=IfTag.Tag.ELIF,
                    iline=self._get_log_line_cur(),
                    is_skipping=is_skipping,
                    comment=args,
                )
                self._log_debug(
                    f"{tag_name} block is {'being skipped' if if_tag_state.is_skipping else 'enabled'}"
                )

        elif tag_name == "elifdef":
            if self._check_else_ok(if_tag_state, tag_name):
                try:
                    self.expr_evaluator.validate_variable_name(args)
                    is_skipping = (
                        True
                        if if_tag_state.innermost.skip == IfTag.Skip.BLOCK
                        else args not in self.variables
                    )
                except Exception as e:
                    self._log_error(
                        f'{tag_name} tag variable name "{args}" is not valid: {e}'
                    )
                    is_skipping = True

                if_tag_state.set(
                    tag=IfTag.Tag.ELIFDEF,
                    iline=self._get_log_line_cur(),
                    is_skipping=is_skipping,
                    command=args,
                )
                self._log_debug(
                    f"{tag_name} block is {'being skipped' if if_tag_state.is_skipping else 'enabled'}"
                )

        elif tag_name == "elifndef":
            if self._check_else_ok(if_tag_state, tag_name):
                try:
                    self.expr_evaluator.validate_variable_name(args)
                    is_skipping = (
                        True
                        if if_tag_state.innermost.skip == IfTag.Skip.BLOCK
                        else args in self.variables
                    )
                except Exception as e:
                    self._log_error(
                        f"{tag_name} tag variable {args} name is not valid: {e}"
                    )
                    is_skipping = True

                if_tag_state.set(
                    tag=IfTag.Tag.ELIFNDEF,
                    iline=self._get_log_line_cur(),
                    is_skipping=is_skipping,
                    command=args,
                )
                self._log_debug(
                    f"{tag_name} block is {'being skipped' if if_tag_state.is_skipping else 'enabled'}"
                )

        elif tag_name == "endif":
            if if_tag_state.is_empty:
                self._log_error(
                    f"{tag_name} tag was not preceeded by a matching if, else, elif, or elifdef tag"
                )
            else:
                if args:
                    if args != if_tag_state.innermost.comment:
                        self._log_warning(
                            "{tag_name} tag comment does not match if/elif/ifdef/ifndef/elifdef/elifndef tag expression"
                        )

                self._log_debug(
                    f"{tag_name} ending {if_tag_state.innermost.tag_name} block at line {if_tag_state.innermost.iline}"
                )
                if_tag_state.pop()

        elif not if_tag_state.is_skipping:
            if tag_name == "include":
                lines_replace = self._process_tag_include(args)

            elif tag_name == "define":
                lines_replace = self._process_tag_define(
                    "define", args, self.variable_scope_global
                )

            elif tag_name == "export":
                lines_replace = self._process_tag_define(
                    "export",
                    args,
                    self.variable_scope_parent
                    if self.variable_scope_parent is not None
                    else self.variable_scope_local,
                )

            elif tag_name == "log":
                lines_replace = self._process_tag_log(args)

            elif tag_name == "print":
                lines_replace = self._process_tag_print(args)

            elif tag_name == "set":
                lines_replace = self._process_tag_define(
                    "set", args, self.variable_scope_local
                )

            elif tag_name == "setlocal":
                lines_replace = self._process_tag_define(
                    "setlocal", args, self.variable_scope_local_only
                )

            elif tag_name == "undef":
                lines_replace = self._process_tag_undef(args)

            else:
                self._log_warning(f'skipping unknown tag "{tag}"', True)

        return lines_replace

    def _process_tag_define(
        self, tag: str, args: str, variable_scope: expr.VariableScope
    ):
        """Process the variable-setting tag

        Arguments:
            tag - Directive being processed
            args - Arguments to the directive
            variable_scope - Scope to contain the new variable

        Returns:
            (Return)  Returns None (to indicate no text replaces tag)
        """
        if not args:
            self._log_error(f"{tag} tag requires variable name")
        else:
            # Get the variable name
            ich = args.find(" ")
            if ich < 1:
                variable_name = args
                variable_value = 1
            else:
                variable_name = args[:ich]

                # Skip whitespace
                cch = len(args)
                while (ich < cch) and args[ich].isspace():
                    ich += 1

                # Remove optional equals sign
                if args[ich] == "=":
                    ich += 1

                try:
                    variable_value = self._eval_expr(args[ich:])
                except Exception as e:
                    self._log_expr_error(
                        f"{tag} tag expression error: {e}", e, args[ich:]
                    )
                    variable_value = 0

            try:
                self.expr_evaluator.validate_variable_name(variable_name)
            except NameError as e:
                self._log_error(
                    f'{tag} tag variable "{variable_name}" not is not valid: {e}'
                )
                variable_name = None

            if variable_name:
                if isinstance(variable_value, str):
                    self._log_debug(
                        f'{tag} variable "{variable_name}" = "{variable_value}"'
                    )
                else:
                    self._log_debug(
                        f'{tag} variable "{variable_name}" = {variable_value}'
                    )
                variable_scope[variable_name] = variable_value

        return None

    def _process_tag_include(self, args: str):
        """Process the file include directive

        Arguments:
            args - Arguments to the directive

        Returns:
            (Return)  List of text lines to replace the tag
        """
        lines_replace = None

        try:
            # Try processing argument as an expression
            filename = str(self._eval_expr(args))
        except:
            # Not a valid expression?  Try using directly as a filename.
            filename = args.strip()

        if len(self.stack_line_source) >= self.include_nest_max:
            self._log_error(
                f'can\'t include file "{filename}"--exceeded include nesting limit of {self.include_nest_max}'
            )
            raise IncludeNestLimitError(
                f'can\'t include file "{filename}"--exceeded nesting limit of {self.include_nest_max}'
            )

        self._log_debug(f'including file "{filename}"')

        # Process the included file
        try:
            with self._open_include_file(filename, "r") as file_include:
                file_line_source = self.FileLineSource(filename, file_include)

                lines_replace = self._process_lines(file_line_source)

        except FileNotFoundError as e:
            self._log_error(str(e))

        return lines_replace

    def _process_tag_log(self, args):
        """Process the file log directive

        Arguments:
            args - Arguments to the directive

        Returns:
            (Return)  List of text lines to replace the tag
        """
        lines_replace = None
        ich_message = args.find(" ")
        if ich_message < 0:
            self._log_error(f"log tag requires message severity and message")

        severity = args[:ich_message].lower()
        str_expr = args[ich_message:].strip()
        if (
            (severity != "info")
            and (severity != "warning")
            and (severity != "error")
            and (severity != "fatal")
        ):
            self._log_error(
                f'log tag severify argument must be "info", "warning", "error" or "fatal"'
            )
        else:
            str_message = ""
            try:
                list_value = self._eval_expr(str_expr, is_arg_list=True)
                for value in list_value:
                    str_message += str(value)

            except Exception as e:
                if severity == "fatal":
                    self._log_expr_error(
                        f"log tag fatal severify expression error: {e}", e, args
                    )
                    raise RuntimeError(
                        f'log tag fatal severity expression "{str_expr}" error: {e}'
                    )
                self._log_expr_error(f"log tag expression error: {e}", e, args)

            if str_message:
                if severity == "info":
                    self._log_info(str_message)
                elif severity == "warning":
                    self._log_warning(str_message)
                elif severity == "error":
                    self._log_error(str_message)
                elif severity == "fatal":
                    self._log_error(str_message)
                    raise RuntimeError(str_message)

        return lines_replace

    def _process_tag_print(self, args):
        """Process the file print directive

        Arguments:
            args - Arguments to the directive

        Returns:
            (Return)  List of text lines to replace the tag
        """
        lines_replace = None
        if len(args) < 1:
            lines_replace = "\n"
        else:
            try:
                list_value = self._eval_expr(args, is_arg_list=True)
                str_result = ""
                for value in list_value:
                    str_result += str(value)
                lines_replace = [str_result]
            except Exception as e:
                self._log_expr_error(f"print tag expression error: {e}", e, args)

        return lines_replace

    def _process_tag_undef(self, args):
        """Process the variable-unsetting tag

        Arguments:
            args - Arguments to the directive

        Returns:
            (Return)  Returns None (to indicate no text replaces tag)
        """
        if len(args) < 1:
            self._log_error(f"undef tag requires variable name")
        try:
            self.expr_evaluator.validate_variable_name(args)

            try:
                self.variables.remove(args)
                self._log_debug(f"undefining variable {args}")
            except NameError:
                pass
        except Exception as e:
            self._log_error(f'undef tag variable name "{args}" is not valid: {e}')

        return None


def makeExtension(**kwargs):  # pragma: no cover
    """Entry point for ProjectAirSimPP Markdown extension"""
    return ProjectAirSimExtension(**kwargs)


def preprocess_file(
    filename_input: str,
    filename_output: str,
    log_level: str="warning",
    include_paths=None,
    include_nest_limit=25,
):
    """Process the specified input file to the specified output file.

    Arguments:
        filename_input - Name of the input file
        filename_output - Name of the output file or None to write preprocessed output to standard output
        log_level - Logger minimum message severity level ("error", "warning", "info", or "debug")
        include_path - List of directory pathnames to search for include files with relative pathnames
        include_next_limit - Maximum allowed depth of nested include files
    """
    projectairsim_processor = ProjectAirSimPreprocessor(
        include_nest_limit=include_nest_limit,
        include_paths=include_paths,
        log_level=log_level,
    )
    with open(filename_input, "r") as file_input:
        file_line_source = ProjectAirSimPreprocessor.FileLineSource(filename_input, file_input)
        lines = projectairsim_processor.process_line_source(file_line_source)

    if filename_output:
        with open(filename_output, "w") as file_output:
            for line in lines:
                file_output.write(line)
                file_output.write("\n")
    else:
        for line in lines:
            print(line)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='ProjectAirSim text preprocessor.  Tags embedded in the input file such as "{# print "Hello" #}" perform actions such as conditional text inclusion, programmatically-created text, insertion of text from files, evaluation of expressions, and more.'
    )
    parser.add_argument(
        "inputfile",
        help=("the file to preprocess"),
        type=str,
        default=None,
    )
    parser.add_argument(
        "--includenestlimit",
        help=("maximum number of nested included files at any one time"),
        type=int,
        default=25,
    )
    parser.add_argument(
        "--includepaths",
        help=("semicolon-separated list of directories to search for included files"),
        type=str,
        default=None,
    )
    parser.add_argument(
        "--outputfile",
        help=("write the output to this file instead of standard output"),
        type=str,
        default=None,
    )
    parser.add_argument(
        "--loglevel",
        help=(
            'log output level: "info", "warning", "error", or "debug" (default "warning".)'
        ),
        type=str,
        default="warning",
    )
    args = parser.parse_args()

    try:
        preprocess_file(
            filename_input=args.inputfile,
            filename_output=args.outfile,
            log_level=args.loglevel,
            include_paths=args.includepaths,
            include_nest_limit=args.includenestlimit,
        )

    except FileNotFoundError as e:
        log.error(f": Can't open input file {args.inputfile}: {e}")
