"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
ProjectAirSimPP's evaluator for C-style expressions
"""
import datetime
import math
import pathlib
import re
import typing

from lexer import LexerFromList, LexerFromString, LexKind, LexSym, OpInfo


class VariableScope(dict):
    """Mapping from variable name (str) to an object"""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.is_hidden = (
            False  # If true, variables in this store are skipped during a search
        )


class Variables:
    """Manages setting, getting, and removing variables"""

    def __init__(self):
        self._variable_scopes = (
            []
        )  # Array of variable stores that are searched in right-to-left order

    def __contains__(self, variable_name):
        for variable_scope in reversed(self._variable_scopes):
            if not variable_scope.is_hidden and (variable_name in variable_scope):
                return True

        return False

    def __getitem__(self, variable_name):
        for variable_scope in reversed(self._variable_scopes):
            if not variable_scope.is_hidden and (variable_name in variable_scope):
                return variable_scope[variable_name]

        raise NameError(f"Variable {variable_name} not found")

    def __setitem__(self, variable_name, variable_value):
        self._variable_scopes[-1][variable_name] = variable_value

    def pop_scope(self) -> VariableScope:
        """Remove and return the top variable scope"""
        return self._variable_scopes.pop()

    def push_scope(self, variable_scope: VariableScope):
        """Push a scope onto the top of the scope stack

        Arguments:
            variable_scope - Scope to push
        """
        self._variable_scopes.append(variable_scope)

    def remove(self, variable_name: str):
        """Remove the first instance of a variable

        The scope stack is searched top down and the variable removed
        from the first scope that contains it.

        Arguments:
            variable_name - Name of variable to remove
        """
        for variable_scope in reversed(self._variable_scopes):
            if variable_name in variable_scope:
                del variable_scope[variable_name]

        raise NameError(f"Variable {variable_name} not found")


class ExprEvaluator:
    """Evaluates in-fix expressions"""

    class StackFuncOp:
        """Manages function and operator"""

        # The following functions want their arguments raw (unevaluated as expressions)
        list_functions_wanting_raw_args = ["defined"]

        class FuncOpEntry:
            def __init__(self, lex_sym: LexSym, wants_raw_args: bool):
                self.lex_sym = lex_sym
                self.wants_raw_args = wants_raw_args

        @property
        def innermost_is_accumulating(self):
            """Returns whether the innermost funcop is accumulating symbols"""
            funcop_outermost = self.get_wants_raw_args_outermost(self.stack_funcop)
            return (funcop_outermost is not None) and (
                funcop_outermost == self.stack_funcop[-1]
            )

        @property
        def top_lex_sym(self):
            """Returns the funcop on top of the stack"""
            if self.stack_funcop:
                return self.stack_funcop[-1].lex_sym

            return None

        @property
        def wants_raw_args(self):
            """Returns whether an entry on the stack is accumulating symbols"""
            return self.wants_raw_args_effective(self.stack_funcop)

        @property
        def bottommost_wants_raw_args(self):
            """Returns the bottommost funcop that is accumulating symbols"""
            if len(self.stack_funcop) < 1:
                return False
            ls_outer = self.get_wants_raw_args_outermost(self.stack_funcop)
            return (ls_outer is not None) and (ls_outer != self.stack_funcop[-1])

        def __init__(self):
            self.stack_funcop = (
                []
            )  # Stack of functions or operators whose subexpressions are not yet completed

        def __getitem__(self, i):
            return self.stack_funcop[i].lex_sym

        def __len__(self):
            return len(self.stack_funcop)

        def push(self, lex_sym: LexSym):
            """Pushes a funcop onto the stack

            Arguments:
                lex_sym - FuncOp to push
            """
            if lex_sym.lex_kind == LexKind.FUNCTION_CALL:
                wants_raw_args = lex_sym.value in self.list_functions_wanting_raw_args
            elif lex_sym.lex_kind == LexKind.OPERATOR:
                wants_raw_args = OpInfo.wants_raw_args(lex_sym.value)
            else:
                wants_raw_args = False

            self.stack_funcop.append(self.FuncOpEntry(lex_sym, wants_raw_args))

        def pop(self):
            """Pops the top funcop from the stack"""
            return self.stack_funcop.pop().lex_sym

        @staticmethod
        def get_wants_raw_args_outermost(stack: typing.List[FuncOpEntry]):
            """Return the bottommost entry in the stack wanting raw arguments

            This method searchs the stack from the bottom-up for the first entry
            wanting raw arguments.

            Arguments:
                stack - FuncOp stack to search

            Returns:
                (return) - Stack entry wanting raw arguments
            """
            for funcop in stack:
                if funcop.wants_raw_args:
                    return funcop

            return None

        @staticmethod
        def wants_raw_args_effective(stack: typing.List[FuncOpEntry]):
            """Returns whether a entry on the stack wants raw arguments

            Arguments:
                stack -FuncOp stack to search

            Returns:
                (return) - True if a stack entry wants raw arguments
            """
            return __class__.get_wants_raw_args_outermost(stack) is not None

    def __init__(self, variables: Variables, file_paths: typing.List[str] = None):
        self.variables = variables  # Manager of variables
        self.file_paths = file_paths  # Array of directories to be searched for file by readfile and readfileline functions

    def evaluate(self, str_expr, is_arg_list: bool = False):
        """Evaluate an expression

        Arguments:
            str_expr - String with expression to evaluate
            is_arg_list - If true, the string contains multiple comma-separated expressions

        Returns:
            (return) - If is_arg_list is false, a single value resulting from evaluating the expression
                       If is_arg_list is true, a list of values resulting from evaluating the expressions
        """
        try:
            self.lexer = LexerFromString(str_expr)
            result = self._evaluate_core(is_arg_list)
        finally:
            self.lexer = None

        return result

    def validate_variable_name(self, variable_name: str):
        """Returns whether the string is a valid variable name

        Arguments:
            variable_name - Variable name to verify

        Returns:
            (return) - true if string is a valid variable name, false otherwise
        """
        if len(variable_name) > 256:
            raise ValueError(f"variables name cannot be longer than 255 characters")

        if (variable_name.lower() == "true") or (variable_name.lower() == "false"):
            raise ValueError(
                f'variables name cannot be reserved keywords "true" or "false"'
            )

        ch = variable_name[0]
        if (ch != "_") and not ch.isalpha():
            raise ValueError(
                f"Variable name can't begin with '{ch}', only a letter or underscore '_'"
            )

        for ch in variable_name[1:]:
            if (ch != "_") and not ch.isalnum():
                raise ValueError(
                    f"Variable name can't contain '{ch}', only letters, digits, or underscore '_'"
                )

    def _convert_value(self, value, target):
        """Converts the value to a type compatible with the target value

        Arguments:
            value - Value to convert, if needed
            target - Value with which we want value to be compatible

        Returns:
            (return) - Value converted to type compatible with target
        """
        if isinstance(value, type(target)):
            return value
        elif isinstance(target, str):
            return str(value)
        elif isinstance(target, bool):
            return bool(value)
        elif isinstance(target, int):
            if isinstance(value, float):
                return value
            elif isinstance(value, bool):
                return 1 if value else 0
            else:
                return int(value)
        elif isinstance(target, float):
            if isinstance(value, int):
                return value
            elif isinstance(value, bool):
                return 1 if value else 0
            else:
                return float(value)

        raise ValueError(
            f'Can\'t convert value: target type "{type(target)}" is not supported'
        )

    def _evaluate_core(self, is_arg_list: bool = False):
        """Evaluate the expression from the current lexer

        Arguments:
            is_arg_list - If true, the expression contains multiple comma-separated subexpressions

        Returns:
            (return) - If is_arg_list is false, a single value resulting from evaluating the expression
                       If is_arg_list is true, a list of values resulting from evaluating the expressions
        """
        stack = []
        error_ret = None
        saw_eof = False
        stack_funcop = (
            self.StackFuncOp()
        )  # Value indicates whether arguments should be not processed as expressions

        while not saw_eof:
            evaluate_again = True
            lex_sym = self.lexer.get_next_lex_sym()

            while not saw_eof and evaluate_again:
                evaluate_again = False

                if lex_sym.lex_kind == LexKind.EOF:
                    saw_eof = True
                    break

                # If symbol is an operator, detect unary operators and change symbol type
                if lex_sym.lex_kind == LexKind.OPERATOR:
                    must_be_unary = len(stack) < 1
                    if not must_be_unary:
                        lex_kind = stack[-1].lex_kind
                        must_be_unary = (
                            (lex_kind == LexKind.PAREN_OPEN)
                            or (lex_kind == LexKind.OPERATOR)
                            or (lex_kind == LexKind.FUNCTION_CALL)
                            or (lex_kind == LexKind.COMMA)
                        )

                    if must_be_unary:
                        # Operator can't be binary, only unary
                        if (lex_sym.value == "+") or (lex_sym.value == "-"):
                            # Change symbol type to unary
                            lex_sym.lex_kind = LexKind.OPERATOR_UNARY
                        elif len(stack) < 1:
                            self._raise_error(
                                SyntaxError,
                                f'Operator "{lex_sym.value}" can\'t be at the beginning of an expression',
                                lex_sym,
                            )
                        else:
                            self._raise_error(
                                SyntaxError,
                                f"Operator \"{lex_sym.value}\" can't immediately follow '{stack[-1].value}'",
                                lex_sym,
                            )

                # If an open-parenthesis is active and we're accumulating raw args, just accumulate the symbol
                if (
                    (len(stack_funcop) > 0)
                    and (stack_funcop.top_lex_sym.value == "(")
                    and stack_funcop.wants_raw_args
                ):
                    if lex_sym.value == "(":
                        stack_funcop.push(lex_sym)
                    elif lex_sym.value == ")":
                        stack_funcop.pop()

                    stack.append(lex_sym)
                    continue

                # Process the symbol
                if lex_sym.lex_kind == LexKind.CHAR:
                    if lex_sym.value.isprintable():
                        self._raise_error(
                            SyntaxError, f"Invalid character '{lex_sym.value}'", lex_sym
                        )
                    else:
                        self._raise_error(
                            SyntaxError,
                            "Invalid character {0:#x}".format(int(lex_sym.value)),
                            lex_sym,
                        )

                elif lex_sym.lex_kind == LexKind.COMMA:
                    if (len(stack_funcop) < 1) and not is_arg_list:
                        self._raise_error(SyntaxError, "Invalid character ','", lex_sym)
                    elif (stack[-1].lex_kind == LexKind.COMMA) or (
                        stack[-1].lex_kind == LexKind.FUNCTION_CALL
                    ):
                        self._raise_error(
                            SyntaxError, f"Function argument can't be empty", lex_sym
                        )
                    else:
                        stack = self._resolve_pending_subexpressions(
                            stack, stack_funcop, OpInfo.PRECEDENCE_LOWEST
                        )
                        stack.append(lex_sym)

                elif lex_sym.lex_kind == LexKind.FUNCTION_CALL:
                    self._verify_subexpression_start_ok(
                        stack, f"Call to function {lex_sym.value}", lex_sym
                    )
                    stack_funcop.push(lex_sym)
                    stack.append(lex_sym)

                elif lex_sym.lex_kind == LexKind.ID:
                    self._verify_subexpression_start_ok(
                        stack, f'The variable "{lex_sym.value}"', lex_sym
                    )

                    if stack_funcop.wants_raw_args:
                        stack.append(lex_sym)
                    else:
                        if lex_sym.value not in self.variables:
                            self._raise_error(
                                NameError,
                                f'Variable "{lex_sym.value}" is not defined',
                                lex_sym,
                            )

                        lex_sym = LexSym(
                            LexKind.VALUE,
                            self.variables[lex_sym.value],
                            lex_sym.ich_start,
                            lex_sym.ich_end,
                        )
                        evaluate_again = True

                elif lex_sym.lex_kind == LexKind.OPERATOR:
                    if len(stack) < 1:
                        self._raise_error(
                            SyntaxError,
                            f'Operator "{lex_sym.value}" can\'t be at the beginning of an expression',
                            lex_sym,
                        )
                    elif not stack_funcop.wants_raw_args and (
                        stack[-1].lex_kind != LexKind.VALUE
                    ):
                        self._raise_error(
                            SyntaxError,
                            f"Operator \"{lex_sym.value}\" can't immediately follow '{stack[-1].value}'",
                            lex_sym,
                        )
                    elif lex_sym.value == ":":
                        # Second part of ternary operator--locate matching first part
                        ls_first_part = None
                        while len(stack_funcop) > 0:
                            ls = stack_funcop.pop()
                            if ls.value == "?":
                                ls_first_part = ls
                                break

                        # Verify ternary operator
                        if ls_first_part is None:
                            self._raise_error(
                                SyntaxError,
                                f"Operator \"{lex_sym.value}\" missing first part of ternary operator, '?'",
                                lex_sym,
                            )

                        else:
                            # Evaluate the first part of the ternary operator if it was the outermost op wanting raw args
                            wants_raw_args = stack_funcop.wants_raw_args
                            if not wants_raw_args:
                                stack_funcop.push(ls)
                                stack = self._resolve_pending_subexpressions(
                                    stack,
                                    stack_funcop,
                                    OpInfo.get_precedence(lex_sym.value),
                                )

                            stack_funcop.push(lex_sym)
                            stack.append(lex_sym)

                    else:
                        precedence_new = OpInfo.get_precedence(lex_sym.value)
                        if (
                            (len(stack_funcop) > 0)
                            and (stack_funcop.top_lex_sym.lex_kind == LexKind.OPERATOR)
                            and OpInfo.is_higher_priority(
                                stack_funcop.top_lex_sym.value, precedence_new
                            )
                        ):
                            # Apply any pending operators with higher or equal priority
                            stack = self._resolve_pending_subexpressions(
                                stack, stack_funcop, precedence_new
                            )

                        if not stack_funcop.wants_raw_args or (lex_sym.value == "?"):
                            stack_funcop.push(lex_sym)

                        stack.append(lex_sym)

                elif lex_sym.lex_kind == LexKind.OPERATOR_UNARY:
                    stack_funcop.push(lex_sym)
                    stack.append(lex_sym)

                elif lex_sym.lex_kind == LexKind.PAREN_OPEN:
                    self._verify_subexpression_start_ok(
                        stack, "An open-parenthesis", lex_sym
                    )
                    stack_funcop.push(lex_sym)
                    stack.append(lex_sym)

                elif lex_sym.lex_kind == LexKind.PAREN_CLOSE:
                    # End of grouped expression or function call?
                    ientry_subexpr_start = -1
                    ls_open_paren = None
                    for i in reversed(range(0, len(stack_funcop))):
                        ls = stack_funcop[i]
                        if ls.lex_kind == LexKind.FUNCTION_CALL:
                            ls_open_paren = ls
                            break
                        elif ls.lex_kind == LexKind.PAREN_OPEN:
                            ls_open_paren = ls
                            break

                    # Get index of function call or open-parenthesis
                    if ls_open_paren:
                        for i in reversed(range(0, len(stack))):
                            if stack[i] == ls_open_paren:
                                if ls_open_paren.lex_kind == LexKind.FUNCTION_CALL:
                                    ientry_subexpr_start = i
                                    break
                                else:
                                    ientry_subexpr_start = i
                                    break

                    if ientry_subexpr_start < 0:
                        self._raise_error(
                            SyntaxError,
                            "Closing parenthesis is missing matching opening parenthesis",
                            lex_sym,
                        )

                    # Evaluate any pending subexpression (while top funcop isn't the open-parenthesis or function call)
                    if stack_funcop.top_lex_sym != ls_open_paren:
                        stack = self._resolve_pending_subexpressions(
                            stack, stack_funcop, OpInfo.PRECEDENCE_LOWEST
                        )

                    # Process the subexpression closed by the close-parenthesis
                    if ls_open_paren.lex_kind == LexKind.PAREN_OPEN:
                        # Verify the subexpression was resolved to one value
                        if len(stack) != (ientry_subexpr_start + 2):
                            self._raise_error(
                                SyntaxError, f"Subexpression syntax is invalid", lex_sym
                            )

                        # Remove the open-parenthesis from the funcop stack
                        stack_funcop.pop()

                        # Remove the subexpression value and open-parenthesis from the stack
                        lex_sym = stack.pop()
                        stack.pop()
                        evaluate_again = True

                    # Process the function call or subexpression
                    else:
                        # End of function arguments
                        if stack[-1].lex_kind == LexKind.COMMA:
                            self._raise_error(
                                SyntaxError,
                                f"Function argument can't be empty",
                                lex_sym,
                            )

                        # If an enclosing funcop is accumulating symbols, remove the function from the funcop stack
                        if stack_funcop.bottommost_wants_raw_args:
                            stack_funcop.pop()

                        # Evaluate the function
                        else:
                            stack_sub = stack[ientry_subexpr_start + 1 :]
                            stack = stack[:ientry_subexpr_start]
                            stack_sub = self._resolve_pending_subexpressions(
                                stack_sub, stack_funcop, OpInfo.PRECEDENCE_LOWEST
                            )

                            # Get function and arguments
                            function_args = self._retrieve_arg_list(stack_sub)

                            # Remove the function from the funcop stack
                            stack_funcop.pop()

                            # Invoke the function
                            lex_sym = self._invoke_function(
                                ls_open_paren, lex_sym, function_args
                            )
                            evaluate_again = True

                elif lex_sym.lex_kind == LexKind.VALUE:
                    self._verify_subexpression_start_ok(stack, "A value", lex_sym)
                    if not stack_funcop.wants_raw_args:
                        if (len(stack) > 0) and (
                            stack[-1].lex_kind == LexKind.OPERATOR_UNARY
                        ):
                            ls_operator = stack.pop()
                            lex_sym = LexSym(
                                LexKind.VALUE,
                                self._resolve_unary_operator(ls_operator, lex_sym),
                                ls_operator.ich_start,
                                lex_sym.ich_end,
                            )
                            evaluate_again = True

                    if not evaluate_again:
                        stack.append(lex_sym)

        if len(stack) == 0:
            raise RuntimeError("Stack is empty after evaluating expression")

        # Evaluate any pending subexpressions
        stack = self._resolve_pending_subexpressions(
            stack, stack_funcop, OpInfo.PRECEDENCE_LOWEST
        )

        for ls in stack:
            lex_kind = ls.lex_kind
            if lex_kind == LexKind.OPERATOR_UNARY:
                self._raise_error(
                    SyntaxError,
                    f"Incomplete expression (argument to unary operator is missing)",
                    ls,
                )
            elif lex_kind == LexKind.TERNARY:
                self._raise_error(
                    SyntaxError,
                    f"Incomplete expression (second part of ternary operator is missing)",
                    ls,
                )

        # Return expression result
        if is_arg_list:
            # Return an argument list
            list_ls = self._retrieve_arg_list(stack)
            list_value = []
            for ls in list_ls:
                list_value.append(ls.value)

            return list_value
        else:
            # Return a single value
            if len(stack) != 1:
                self._raise_error(SyntaxError, f"Incomplete expression", stack[-1])

            return stack[0].value

    def _evaluate_from_list(
        self, list_lex_sym: typing.List[LexSym], str_src: str, is_arg_list: bool = False
    ):
        """Evaluate an expression as a list of lexical symbols

        Arguments:
            list_lex_sym - List of lexical symbols to evaluate
            str_src - String containing the source referenced by the lexsyms
            is_arg_list - If true, the string contains multiple comma-separated expressions

        Returns:
            (return) - If is_arg_list is false, a single value resulting from evaluating the expression
                       If is_arg_list is true, a list of values resulting from evaluating the expressions
        """
        lexer_sav = self.lexer
        try:
            self.lexer = LexerFromList(list_lex_sym, str_src)
            result = self._evaluate_core(is_arg_list)
        finally:
            self.lexer = lexer_sav

        return result

    def _invoke_function(self, ls_function, ls_function_terminator, args):
        """Invoke the specified expression function

        Arguments:
            ls_function - Lexsym with the function to invoke
            ls_function_terminator - Lexsym that terminated the function's arguments

        Returns:
            (return) - Value returned by the function
        """
        value_ret = None
        cargs = len(args)
        ich_start = ls_function.ich_start
        ich_end = ls_function_terminator.ich_start

        try:
            function_name = ls_function.value
            if function_name == "bool":
                if cargs != 1:
                    self._raise_error_function_carg(
                        function_name, "1 argument", cargs, ich_start, ich_end
                    )

                value_ret = bool(args[0].value)

            elif function_name == "capitalize":
                if cargs != 1:
                    self._raise_error_function_carg(
                        function_name, "1 argument", cargs, ich_start, ich_end
                    )

                value_ret = str(args[0].value).capitalize()

            elif function_name == "ceil":
                if cargs != 1:
                    self._raise_error_function_carg(
                        function_name, "1 argument", cargs, ich_start, ich_end
                    )
                value_ret = math.ceil(args[0].value)

            elif function_name == "compactws":
                if cargs != 1:
                    self._raise_error_function_carg(
                        function_name, "1 argument", cargs, ich_start, ich_end
                    )

                value_ret = ""
                saw_ws_last = False
                for ch in str(args[0].value):
                    if not ch.isspace():
                        value_ret += ch
                        saw_ws_last = False
                    elif not saw_ws_last:
                        value_ret += ch
                        saw_ws_last = True

            elif function_name == "concat":
                value_ret = ""
                for arg in args:
                    value_ret += str(arg.value)

            elif function_name == "datetime":
                if cargs > 1:
                    self._raise_error_function_carg(
                        function_name, "at most 1 argument", cargs, ich_start, ich_end
                    )

                value_ret = datetime.datetime.today().strftime(
                    "%c" if cargs == 0 else str(args[0].value)
                )

            elif function_name == "defined":
                if (cargs == 1) and (args[0].lex_kind == LexKind.LEXSYM_LIST):
                    args = args[0].value
                    cargs = len(args)
                if cargs != 1:
                    self._raise_error_function_carg(
                        function_name, "1 argument", cargs, ich_start, ich_end
                    )
                ls_variable = args[0]
                if (ls_variable.lex_kind != LexKind.ID) and (
                    (ls_variable.lex_kind != LexKind.VALUE)
                    or not isinstance(ls_variable.value, str)
                ):
                    self._raise_error(
                        TypeError,
                        f'Function "{function_name}" argument must be a variable name or a string',
                        ls_variable,
                    )

                variable_name = ls_variable.value
                self.validate_variable_name(variable_name)
                value_ret = variable_name in self.variables

            elif function_name == "field":
                if cargs != 3:
                    self._raise_error_function_carg(
                        function_name, "3 arguments", cargs, ich_start, ich_end
                    )

                fields = str(args[0].value).split(args[1].value)
                index = int(args[2].value)
                value_ret = fields[index] if (index < len(fields)) else ""

            elif function_name == "field_count":
                if cargs != 2:
                    self._raise_error_function_carg(
                        function_name, "2 arguments", cargs, ich_start, ich_end
                    )

                fields = str(args[0].value).split(args[1].value)
                value_ret = len(fields)

            elif function_name == "find":
                if cargs != 2:
                    self._raise_error_function_carg(
                        function_name, "2 arguments", cargs, ich_start, ich_end
                    )

                str_source = str(args[0].value)
                str_find = str(args[1].value)
                if not isinstance(str_source, str) and not isinstance(str_find, str):
                    self._raise_error_ich(
                        TypeError,
                        f'Function "{function_name}" accepts only a strings for the first and second arguments',
                        ich_start,
                        ich_end,
                    )

                value_ret = str_source.find(str_find)

            elif function_name == "float":
                if cargs != 1:
                    self._raise_error_function_carg(
                        function_name, "1 argument", cargs, ich_start, ich_end
                    )

                value_ret = float(args[0].value)

            elif function_name == "floor":
                if cargs != 1:
                    self._raise_error_function_carg(
                        function_name, "1 argument", cargs, ich_start, ich_end
                    )

                value_ret = math.floor(args[0].value)

            elif function_name == "format":
                if cargs < 1:
                    self._raise_error_function_carg(
                        function_name, "at least 1 argument", cargs, ich_start, ich_end
                    )

                value_ret = str(args[0].value).format(*[arg.value for arg in args[1:]])

            elif function_name == "int":
                if cargs != 1:
                    self._raise_error_function_carg(
                        function_name, "1 argument", cargs, ich_start, ich_end
                    )

                value_ret = int(args[0].value)

            elif function_name == "len":
                if cargs != 1:
                    self._raise_error_function_carg(
                        function_name, "1 argument", cargs, ich_start, ich_end
                    )

                value_ret = len(args[0].value)

            elif function_name == "lower":
                if cargs != 1:
                    self._raise_error_function_carg(
                        function_name, "1 argument", cargs, ich_start, ich_end
                    )

                value_ret = str(args[0].value).lower()

            elif function_name == "readfile":
                if cargs != 1:
                    self._raise_error_function_carg(
                        function_name, "1 argument", cargs, ich_start, ich_end
                    )

                filename = args[0].value
                if not isinstance(filename, str):
                    self._raise_error_ich(
                        TypeError,
                        f'Function "{function_name}" accepts only a string as the first argument',
                        ich_start,
                        ich_end,
                    )

                list_str_lines = []
                try:
                    with self._open_from_file_paths(
                        filename,
                        "r",
                        LexSym(
                            LexKind.FUNCTION_CALL, ls_function.value, ich_start, ich_end
                        ),
                    ) as file_input:
                        list_str_lines = file_input.readlines()
                except Exception as e:
                    self._raise_error_ich(
                        TypeError,
                        f'Function "{function_name}" can\'t open file "{filename}": {e}',
                        ich_start,
                        ich_end,
                    )

                value_ret = ""
                for str_line in list_str_lines:
                    value_ret += str_line

            elif function_name == "readfileline":
                if cargs != 1:
                    self._raise_error_function_carg(
                        function_name, "1 argument", cargs, ich_start, ich_end
                    )

                filename = args[0].value
                if not isinstance(filename, str):
                    self._raise_error_ich(
                        TypeError,
                        f'Function "{function_name}" accepts only a string as the first argument',
                        ich_start,
                        ich_end,
                    )

                value_ret = ""
                try:
                    with self._open_from_file_paths(
                        filename,
                        "r",
                        LexSym(
                            LexKind.FUNCTION_CALL, ls_function.value, ich_start, ich_end
                        ),
                    ) as file_input:
                        value_ret = file_input.readline()
                except Exception as e:
                    self._raise_error_ich(
                        TypeError,
                        f'Function "{function_name}" can\'t open file "{filename}": {e}',
                        ich_start,
                        ich_end,
                    )

                if value_ret and (value_ret[-1] == "\n"):
                    value_ret = value_ret[:-1]

            elif function_name == "regex":
                if cargs != 2:
                    self._raise_error_function_carg(
                        function_name, "2 arguments", cargs, ich_start, ich_end
                    )

                value_ret = ""
                match = re.compile(str(args[0].value)).search(str(args[1].value))
                if match:
                    value_ret = match.group(0)

            elif function_name == "strip":
                if cargs != 1:
                    self._raise_error_function_carg(
                        function_name, "1 argument", cargs, ich_start, ich_end
                    )

                value_ret = str(args[0].value).strip()

            elif function_name == "substr":
                if (cargs < 2) or (cargs > 3):
                    self._raise_error_function_carg(
                        function_name, "2 or 3 arguments", cargs, ich_start, ich_end
                    )

                str_value = args[0].value
                if not isinstance(str_value, str):
                    self._raise_error_ich(
                        TypeError,
                        f'Function "{function_name}" accepts only a string as the first argument',
                        ich_start,
                        ich_end,
                    )
                if not isinstance(args[1].value, int) or (
                    (cargs == 3) and not isinstance(args[2].value, int)
                ):
                    self._raise_error_ich(
                        TypeError,
                        f'Function "{function_name}" accepts only integers as the second or third argument',
                        ich_start,
                        ich_end,
                    )

                ich_value_first = args[1].value
                if cargs == 2:
                    value_ret = str_value[ich_value_first:]
                else:
                    value_ret = str_value[ich_value_first : args[2].value]

            elif function_name == "str":
                if cargs != 1:
                    self._raise_error_function_carg(
                        function_name, "1 argument", cargs, ich_start, ich_end
                    )

                value_ret = str(args[0].value)

            elif function_name == "str":
                if cargs != 1:
                    self._raise_error_function_carg(
                        function_name, "1 argument", cargs, ich_start, ich_end
                    )

                value_ret = str(args[0].value)

            elif function_name == "translate":
                if cargs != 3:
                    self._raise_error_function_carg(
                        function_name, "3 arguments", cargs, ich_start, ich_end
                    )

                str_source = args[0].value
                str_from = args[1].value
                str_to = args[2].value
                if not isinstance(str_from, str) or not isinstance(str_to, str):
                    self._raise_error_ich(
                        TypeError,
                        f'Function "{function_name}" second and third arguments must be strings',
                        ich_start,
                        ich_end,
                    )

                if len(str_from) != len(str_to):
                    self._raise_error_ich(
                        TypeError,
                        f'Function "{function_name}" second and third string arguments must be the same length',
                        args[1].ich_start,
                        args[2].ich_end,
                    )

                value_ret = str(str_source).translate(str.maketrans(str_from, str_to))

            elif function_name == "upper":
                if cargs != 1:
                    self._raise_error_function_carg(
                        function_name, "1 argument", cargs, ich_start, ich_end
                    )

                value_ret = str(args[0].value).upper()

            if value_ret is None:
                self._raise_error(
                    NameError, f'Function "{function_name}" is unknown', ls_function
                )

            return LexSym(LexKind.VALUE, value_ret, ich_start, ich_end)

        except Exception as e:
            # Add position of function in string
            e.ich_start = ich_start + 1
            e.ich_end = ich_end + 1
            raise e

    def _open_from_file_paths(self, filename: str, access: str, ls: LexSym):
        """Search for the specified file in the file path lists and open it

        Arguments:
            filename - Name of file to open
            access - Python-style file open access specifier
            ls - LexSym of the filename (for error reporting)

        Returns:
            (return) File object if file found, None otherwise
        """
        if not self.file_paths:
            return open(filename, access)
        else:
            file = None
            for include_path in self.file_paths:
                path_file = pathlib.PurePath(include_path).joinpath(filename)
                try:
                    file = open(path_file, access)
                    break
                except Exception as e:
                    pass

            if file is None:
                self._raise_error(
                    FileNotFoundError,
                    f'Can\'t open file "{filename}" from the file paths',
                    ls,
                )

            return file

    def _print_funcop_stack(self, funcop_stack):
        """Print the funcop stack"""
        print("FuncOp Stack:")
        for i in reversed(range(0, len(funcop_stack))):
            print(f"  {i}: {funcop_stack[i]}")

    def _print_stack(self, stack):
        """Print the lexsym stack"""
        print("Stack:")
        for i in reversed(range(0, len(stack))):
            print(f"  {i}: {stack[i]}")

    def _print_stacks(self, stack, funcop_stack):
        """Print the lexym and funcop stacks"""
        self._print_stack(stack)
        self._print_funcop_stack(funcop_stack)

    def _raise_error(self, cls_error, message, lexsym):
        """Raise the specified exception with context

        Arguments:
            cls_error - Error class to raise
            message - Message to attach to exception
            lexsym - LexSym locating the exception in the expression source
        """
        self._raise_error_ich(
            cls_error, message, lexsym.ich_start, lexsym.ich_end
        )

    def _raise_error_function_carg(
        self,
        function_name: str,
        carg_expected: str,
        carg_provided: int,
        ich_start: int,
        ich_end: int,
    ):
        """Raise an exception for the wrong number of arguments to a function

        Arguments:
            function_name - Name of the function
            carg_expected - Number of expected arguments
            carg_provided - Number of actual arguments
            ich_start - Index of first character of arguments
            inc_end - Index of last character of arguments
        """
        self._raise_error_ich(
            SyntaxError,
            f'Function "{function_name}" takes {carg_expected}, {carg_provided} given',
            ich_start,
            ich_end,
        )

    def _raise_error_ich(self, cls_error, message, ich_start, ich_end):
        """Raise an exception with context

        Arguments:
            cls_error - Exception class to raise
            message - Message to attach to exception
            ich_start - Index of first character in expression source
            ich_end - Index of one past last character in expression source or None
        """
        ich_first = ich_start
        ich_first_msg = ich_first + 1
        if ich_end > ich_first_msg:
            message = f"(cols {ich_first_msg} - {ich_end}): " + message
        else:
            ich_last = None
            message = f"(col {ich_first_msg}): " + message

        err = cls_error(message)
        err.ich_first = ich_first

        if ich_end is not None:
            err.ich_last = ich_end - 1

        raise err

    def _resolve_binary_operator(self, ls_left: LexSym, operator: str, ls_right: LexSym):
        """Evaluate a binary operator

        Argument:
            ls_left - Left argument
            operator - Operator
            ls_right - Right argument

        Returns:
            (return) Operator's value
        """
        left_value = ls_left.value
        right_value = ls_right.value
        if operator == "-":
            if isinstance(left_value, str) and isinstance(right_value, str):
                # Remove the first instance of the second string from the first
                ich = left_value.find(right_value)
                if ich < 0:
                    value_ret = left_value
                else:
                    cch_right = len(right_value)
                    if ich == 0:
                        value_ret = left_value[cch_right:]
                    elif (ich + cch_right) >= len(left_value):
                        value_ret = left_value[:-cch_right]
                    else:
                        value_ret = left_value[:ich] + left_value[ich + cch_right :]
            else:
                value_ret = left_value - right_value
        elif operator == "+":
            if isinstance(left_value, bool):
                value_ret = left_value or bool(right_value)
            elif isinstance(left_value, str):
                value_ret = left_value + str(right_value)
            elif isinstance(right_value, str):
                value_ret = str(left_value()) + right_value
            else:
                value_ret = left_value + right_value
        elif operator == "*":
            if isinstance(left_value, bool):
                value_ret = left_value and bool(right_value)
            elif isinstance(left_value, str):
                value_ret = left_value * int(right_value)
            elif isinstance(right_value, str):
                value_ret = right_value * int(left_value)
            else:
                value_ret = left_value * self._convert_value(right_value, left_value)
        elif operator == "/":
            if isinstance(left_value, bool):
                value_ret = left_value ^ bool(right_value)
            elif isinstance(left_value, str):
                value_ret = ""
            elif isinstance(right_value, str):
                value_ret = left_value / self._convert_value(right_value, left_value)
            else:
                value_ret = left_value / right_value
        elif operator == "%":
            if isinstance(left_value, bool):
                value_ret = false
            elif isinstance(left_value, str):
                value_ret = ""
            elif isinstance(right_value, str):
                value_ret = left_value % self._convert_value(right_value, left_value)
            else:
                value_ret = left_value % right_value
        elif operator == "<":
            value_ret = left_value < right_value
        elif operator == "<=":
            value_ret = left_value <= right_value
        elif operator == ">":
            value_ret = left_value > right_value
        elif operator == ">=":
            value_ret = left_value >= right_value
        elif operator == "==":
            value_ret = left_value == right_value
        elif operator == "||":
            if bool(left_value):
                value_ret = True
            elif ls_right.lex_kind != LexKind.LEXSYM_LIST:
                self._raise_error_ich(
                    RuntimeError,
                    f"Logical operator '{operator}' right value is not a list of LexSym",
                    ls_left.ich_start,
                    ls_right.ich_end,
                )
            else:
                if ls_right.lex_kind != LexKind.LEXSYM_LIST:
                    self._raise_error_ich(
                        RuntimeError,
                        f"Logical operator '{operator}' right value is not a list of LexSym",
                        ls_left.ich_start,
                        ls_right.ich_end,
                    )
                value_ret = self._evaluate_from_list(right_value, self.lexer.str_source)
        elif operator == "&&":
            if not bool(left_value):
                value_ret = False
            elif ls_right.lex_kind != LexKind.LEXSYM_LIST:
                self._raise_error_ich(
                    RuntimeError,
                    f"Logical operator '{operator}' right value is not a list of LexSym",
                    ls_left.ich_start,
                    ls_right.ich_end,
                )
            else:
                if ls_right.lex_kind != LexKind.LEXSYM_LIST:
                    self._raise_error_ich(
                        RuntimeError,
                        f"Logical operator '{operator}' right value is not a list of LexSym",
                        ls_left.ich_start,
                        ls_right.ich_end,
                    )
                value_ret = self._evaluate_from_list(right_value, self.lexer.str_source)
        elif operator == "?":
            if ls_right.lex_kind != LexKind.LEXSYM_LIST:
                self._raise_error_ich(
                    RuntimeError,
                    f"Ternary operator '{operator}' right value is not a list of LexSym",
                    ls_left.ich_start,
                    ls_right.ich_end,
                )
            return LexSym(
                LexKind.TERNARY,
                right_value if left_value else None,
                ls_left.ich_start,
                ls_right.ich_end,
            )
        elif operator == ":":
            return self._resolve_ternary_operator(ls_left, ls_right)
        else:
            self._raise_error_ich(
                RuntimeError,
                f"Binary operator '{operator}' is not handled",
                ls_left.ich_start,
                ls_right.ich_end,
            )

        return LexSym(LexKind.VALUE, value_ret, ls_left.ich_start, ls_right.ich_end)

    def _resolve_pending_subexpressions(
        self, stack, stack_funcop: StackFuncOp, min_precedence_level: int
    ):
        """Repeatedly evaluate subexpressions at the end of the stack until
        the operator at the top of the stack has a lower precedence or is a
        right-associative operator of equal precedence (the operator of
        the specified precendence would need to be evaluated first.)

        Argument:
            ls_left - Left argument
            operator - Operator
            ls_right - Right argument

        Returns:
            (return) Operator's value
        """
        if (len(stack) < 2) or (len(stack_funcop) < 1):
            return stack

        ls_value = None
        isym = len(stack) - 1

        if stack_funcop.wants_raw_args:
            ls_funcop = stack_funcop.top_lex_sym
            if not OpInfo.is_higher_priority(ls_funcop.value, min_precedence_level):
                return stack
        else:
            ls_value = stack[isym]

            # If the most recent operator doesn't want raw arguments, then the top of the stack must be a value;
            # any individual LexSym that could be a value would have been already evaluated to one
            if ls_value.lex_kind != LexKind.VALUE:
                return stack

            isym -= 1

        while isym >= 0:
            # Get the operator and the LexSym list if the operator wants raw arguments
            while True:
                ls = stack[isym]

                if not stack_funcop.wants_raw_args:
                    break
                elif ls == ls_funcop:
                    stack_funcop.pop()

                    if stack_funcop.wants_raw_args:
                        # This operator is part of the LexSym list of an earlier operator--keep going
                        ls_funcop = stack_funcop.top_lex_sym
                        if not OpInfo.is_higher_priority(
                            ls_funcop.value, min_precedence_level
                        ):
                            return stack
                    else:
                        # Evaluate this operator
                        stack_funcop.push(ls)
                        list_lexsym = stack[isym + 1 :]
                        ls_value = LexSym(
                            LexKind.LEXSYM_LIST,
                            list_lexsym,
                            list_lexsym[0].ich_start,
                            list_lexsym[-1].ich_end,
                        )
                        break

                isym -= 1

            if ls.lex_kind == LexKind.OPERATOR_UNARY:
                stack_funcop.pop()
                ls_value = LexSym(
                    LexKind.VALUE,
                    self._resolve_unary_operator(ls.value, ls_value.value),
                    ls_value_left.ich_start,
                    ls_value.ich_end,
                )

            elif ls.lex_kind == LexKind.OPERATOR:
                if isym < 1:
                    self._raise_error(
                        RuntimeError,
                        "Encountered OPERATOR but VALUE for left-side value of binary operator is missing",
                        ls,
                    )

                # Stop evaluating when we encounter an operator of lower precedence or right-associative operator
                # of equal precedence meaning the next operator would need to be evaluated first
                if not OpInfo.is_higher_priority(ls.value, min_precedence_level):
                    isym += 1
                    break

                stack_funcop.pop()
                isym -= 1
                ls_value_left = stack[isym]
                if ls.value == ":":
                    if ls_value_left.lex_kind != LexKind.TERNARY:
                        self._raise_error(
                            RuntimeError,
                            f'Encountered lexical symbol {ls_value_left.lex_kind} (value "{ls_value_left.value}"), but expected TERNARY for left-side value of binary operator "{ls.value}"',
                            ls_value_left,
                        )
                elif ls_value_left.lex_kind != LexKind.VALUE:
                    self._raise_error(
                        RuntimeError,
                        f'Encountered lexical symbol {ls_value_left.lex_kind} (value "{ls_value_left.value}"), but expected VALUE for left-side value of binary operator "{ls.value}"',
                        ls_value_left,
                    )

                ls_value = self._resolve_binary_operator(
                    ls_value_left, ls.value, ls_value
                )

            else:
                # Not an operator so not part of subexpression
                break

            isym -= 1

        # Replace the subexpression symbols with the subexpression result
        stack = stack[: isym + 1]
        stack.append(ls_value)
        return stack

    def _resolve_ternary_operator(self, ls_left: LexSym, ls_right: LexSym):
        """Evaluate the ternary operator

        This function produces the final result of the ternary operator.  The
        first part of the ternary operator has already been evaluated and
        pass in as ls_left while the right-most value is passed-in as ls_right.

        Arguments:
            ls_left - Result of first part of ternary operator
            ls_right - Right-hand argument to the ternary operator
        """
        if ls_left.lex_kind != LexKind.TERNARY:
            self._raise_error_ich(
                SyntaxError,
                f"Ternary operator is missing first part, '?'",
                ls_left.ich_start,
                ls_right.ich_end,
            )
        if ls_right.lex_kind != LexKind.LEXSYM_LIST:
            self._raise_error_ich(
                RuntimeError,
                f"Ternary operator second part ':' right value is not a list of LexSym",
                ls_left.ich_start,
                ls_right.ich_end,
            )

        if (ls_left.value is not None) and not isinstance(ls_left.value, list):
            self._raise_error_ich(
                RuntimeError,
                f"\"True part\" value of '?:' operator is not a list of LexSym objects",
                ls_left.ich_start,
                ls_right.ich_end,
            )
        if not isinstance(ls_right.value, list):
            self._raise_error_ich(
                RuntimeError,
                f"\"False part\" value of '?:' operator is not a list of LexSym objects",
                ls_left.ich_start,
                ls_right.ich_end,
            )

        # Evaluate "then" or "else" value depending on result from "?" operator
        list_ls = ls_left.value if (ls_left.value is not None) else ls_right.value
        return LexSym(
            LexKind.VALUE,
            self._evaluate_from_list(list_ls, self.lexer.str_source),
            ls_left.ich_start,
            ls_right.ich_end,
        )

    def _resolve_unary_operator(self, ls_operator, ls_value):
        """Evaluate a unary operator

        Arguments:
            ls_operator - The unary operator
            ls_value - The argument to the operator

        Returns:
            (return) Operator's value
        """
        operator = ls_operator.value
        if operator == "!":
            return not bool(ls_value.value)
        elif operator == "-":
            if isinstance(ls_value.value, bool):
                return not ls_value.value
            elif isinstance(ls_value.value, str):
                return ""
            else:
                return -ls_value.value
        elif operator == "+":
            return ls_value.value
        else:
            raise self._raise_error(
                RuntimeError, f"'{operator}' is not handled", ls_operator
            )

    def _retrieve_arg_list(self, stack):
        """Convert a stack of comma-separated function arguments
        to a list

        Arguments:
            stack - Stack to convert

        Returns:
            (Return) List of arguments
        """
        list_args = []
        ientry = 0
        ientry_last = len(stack) - 1
        if ientry <= ientry_last:
            list_ls = []

            while True:
                end_of_stack = ientry == ientry_last
                ls = stack[ientry]
                lex_kind = ls.lex_kind

                if lex_kind != LexKind.COMMA:
                    list_ls.append(ls)

                if end_of_stack or (lex_kind == LexKind.COMMA):
                    if len(list_ls) == 1:
                        list_args.append(list_ls[0])
                        list_ls = []
                    else:
                        list_args.append(
                            LexSym(
                                LexKind.LEXSYM_LIST,
                                list_ls,
                                list_ls[0].ich_start,
                                list_ls[-1].ich_end,
                            )
                        )
                        list_ls = []

                if end_of_stack:
                    break

                ientry += 1

        return list_args

    def _verify_subexpression_start_ok(self, stack: typing.List[LexSym], item_name: str, lex_symbol: LexSym):
        """Returns whether a subexpression can be started given the current
        state of lexsym stack.

        Arguments:
            stack - LexSym stack
            item_name - Name of lex_symbol that would be starting a subexpression
            lex_symbol - LexSym that would be starting a subexpression

        Returns:
            (Return) True if a subexpression can be started, False otherwise
        """
        if len(stack) > 0:
            lex_top = stack[-1]
            lex_kind_top = lex_top.lex_kind
            if (
                (lex_kind_top != LexKind.PAREN_OPEN)
                and (lex_kind_top != LexKind.OPERATOR)
                and (lex_kind_top != LexKind.OPERATOR_UNARY)
                and (lex_kind_top != LexKind.FUNCTION_CALL)
                and (lex_kind_top != LexKind.COMMA)
            ):
                self._raise_error(
                    SyntaxError,
                    f'{item_name} can\'t immediately follow "{self.lexer.get_string(lex_top)}"',
                    lex_symbol,
                )
