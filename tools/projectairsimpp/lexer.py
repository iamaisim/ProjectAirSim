"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
ProjectAirSimPP's lexical analyzer for C-flavored expressions
"""
import ast
import string
import typing

from abc import abstractmethod
from enum import Enum


class OpInfo:
    """Operator information"""

    class OpEntry:
        def __init__(
            self,
            precedence: int,
            is_right_associative: bool = False,
            want_raw_args: bool = False,
        ):
            self.precedence = precedence  # Operator's precedence (operators with higher values are evaluated before those with lower values)
            self.is_right_associative = is_right_associative  # If this operator is to the left of an operator with the same precedence, the operator on the right is evaluated first
            self.want_raw_args = want_raw_args  # The following operators want their second arguments to be a list of LexSym's and not evaluated as expressions

    PRECEDENCE_LOWEST = (
        -1
    )  # Lowest possible operator precedence, used for comma or closing parenthesis to apply any pending operators
    map_operator_precedence = {
        # Operator symbol: operator precedence (higher has priority)
        "!": OpEntry(6),  # Unary boolean negation
        "*": OpEntry(5),  # Multiplication
        "/": OpEntry(5),  # Division
        "%": OpEntry(5),  # Modulo
        "+": OpEntry(4),  # Addition, also unary '+' whose precedence is highest
        "-": OpEntry(4),  # Subtraction, Also unary '-' whose precedence is highest
        "<": OpEntry(
            3
        ),  # Less-than, also used during lexical analysis to detect the first character of '<='
        ">": OpEntry(
            3
        ),  # Greater-than, also used during lexical analysis to detect the first character of '>='
        "=": OpEntry(
            3
        ),  # Used during lexical analysis to detect the first character of '=='
        "|": OpEntry(
            2
        ),  # Used during lexical analysis to detect the first character of '||'
        "&": OpEntry(
            2
        ),  # Used during lexical analysis to detect the first character of '&&'
        "?": OpEntry(
            1, is_right_associative=True, want_raw_args=True
        ),  # First part of ternary operator
        ":": OpEntry(0, want_raw_args=True),  # Second part of ternary operator
        "<=": OpEntry(3),  # Less-than or equal
        ">=": OpEntry(3),  # Greater-than or equal
        "==": OpEntry(3),  # Equality
        "||": OpEntry(2, want_raw_args=True),  # Logical OR
        "&&": OpEntry(2, want_raw_args=True),  # Logical AND
    }

    @classmethod
    def get_precedence(cls, operator: str):
        """Returns the precedence of the operator

        Arguments:
            operator - The operator to check
        """
        if operator == "(":
            return cls.PRECEDENCE_LOWEST
        elif operator in cls.map_operator_precedence:
            return cls.map_operator_precedence[operator].precedence
        else:
            raise RuntimeError(
                f'OpInfo.get_precedence(): Operator "{operator}" is not in map_operator_precedence'
            )

    @classmethod
    def is_higher_priority(cls, operator: str, precedence: int):
        """
        Returns whether this operator should be evaluated first given the precedence of the next operator.

        Arguments:
            operator - Operator to check
            precedence - Reference precedence
        """
        op_precedence = cls.get_precedence(operator)
        return (op_precedence > precedence) or (
            (op_precedence == precedence) and not cls.is_right_associative(operator)
        )

    @classmethod
    def is_op(cls, operator: str):
        """Return whether the string is an operator

        Arguments:
            operator - Operator string to check
        """
        return operator in cls.map_operator_precedence

    @classmethod
    def is_right_associative(cls, operator: str):
        """Returns whether the operator is right-associative

        Arguments:
            operator - Operator to check
        """
        if operator in cls.map_operator_precedence:
            return cls.map_operator_precedence[operator].is_right_associative
        else:
            raise RuntimeError(
                f'OpInfo.is_right_associative(): Operator "{operator}" is not in map_operator_precedence'
            )

    @classmethod
    def wants_raw_args(cls, operator: str):
        """Returns whether the operator wants raw arguments

        Arguments:
            operator - Operator to check
        """
        if operator in cls.map_operator_precedence:
            return cls.map_operator_precedence[operator].want_raw_args
        else:
            raise RuntimeError(
                f'OpInfo.wants_raw_args(): Operator "{operator}" is not in map_operator_precedence'
            )


class LexKind(Enum):
    """Lexical symbol classifications"""

    EOF = 0  # End of lexical symbols
    ID = 1  # Identifier
    CHAR = 2  # Single character
    VALUE = 3  # String or numeric value
    FUNCTION_CALL = 4  # Function call
    OPERATOR = 5  # Binary or unary operator
    COMMA = 6  # Comma character
    PAREN_OPEN = 7  # Open-parenthesis character
    PAREN_CLOSE = 8  # Close-parenthesis character

    # The following are during evaluation only and never returned by get_next_lex_sym()
    OPERATOR_UNARY = 100  # Unary operator
    LEXSYM_LIST = 101  # List of lexical symbols (for functions/operators wanting unevaluated LexSyms)
    TERNARY = 102  # Result of evaluation of first part of ternary operator; LexSym value is None if condition is false, a list of LexSym of the "then" expression if condition is true


class LexSym:
    """Lexical symbol and context"""

    def __init__(self, lex_kind, value, ich_start: int, ich_end: int):
        self.lex_kind = lex_kind  # Kind of lexical symbol
        self.value = value  # Symbol's string value
        self.ich_start = (
            ich_start  # Index of the first character of the symbol in the source string
        )
        self.ich_end = ich_end  # Index of the first character beyond the symbol in the source string

    def __str__(self):
        return f"lex_kind={self.lex_kind}, value={self.value}, ich {self.ich_start} - {self.ich_end}"


class Lexer:
    """Lexical analysis and symbol provider base class"""

    @property
    def str_source(self):
        """Source string being analyszed"""
        return self._str_source

    def __init__(self, str_source: str):
        self._str_source = str_source  # String source of the expression being analyzed
        self.cch = len(str_source)  # Length of the string source

    @abstractmethod
    def get_next_lex_sym(self):
        """Returns the next lexical symbol"""
        raise NotImplementedError()

    def get_string(self, lex_symbol):
        """Returns the string from the source for the symbol

        Arguments:
            lex_symbol - Symbol whose source string to retrieve
        """
        return (
            ""
            if (lex_symbol.ich_start < 0) or (lex_symbol.ich_end < 0)
            else self.str_source[lex_symbol.ich_start : lex_symbol.ich_end]
        )


class LexerFromList(Lexer):
    """Provides lexical symbols from a list of lexical symbols"""

    def __init__(self, list_lex_sym: typing.List[LexSym], str_source: str):
        super().__init__(str_source)
        self.clex_sym = len(list_lex_sym)  # Number of symbols in the list
        self.ilex_sym = 0  # Index of the next symbol to return
        self.list_lex_sym = list_lex_sym  # List of symbols

        ich_eof = 0 if (len(list_lex_sym) < 1) else list_lex_sym[-1].ich_end
        self.ls_eof = LexSym(LexKind.EOF, None, ich_eof, ich_eof)

    def get_next_lex_sym(self):
        """Returns the next lexical symbol"""
        if self.ilex_sym >= self.clex_sym:
            ls_ret = self.ls_eof
        else:
            ls_ret = self.list_lex_sym[self.ilex_sym]
            self.ilex_sym += 1

        return ls_ret


class LexerFromString(Lexer):
    """Performs lexical analysis upon and provide lexical symbols from a string"""

    # Mapping from escaped single character sequence to character
    map_escape_sequence_char = {
        "a": "\a",
        "b": "\b",
        "f": "\f",
        "n": "\n",
        "r": "\r",
        "t": "\t",
        "v": "\v",
    }

    def __init__(self, str_source: str):
        super().__init__(str_source)
        self.ich = 0  # Index into str_source where to begin looking for the next symbol

    def get_next_lex_sym(self):
        """Return the next lexical symbol"""
        cch = self.cch
        ich = self.ich

        while ich < cch:
            if not self.str_source[ich].isspace():
                break
            ich += 1

        if ich >= cch:
            return LexSym(LexKind.EOF, None, ich, ich)

        ch = self.str_source[ich]
        if ch == "(":
            self.ich = ich + 1
            ls_ret = LexSym(LexKind.PAREN_OPEN, ch, ich, ich + 1)
        elif ch == ")":
            self.ich = ich + 1
            ls_ret = LexSym(LexKind.PAREN_CLOSE, ch, ich, ich + 1)
        elif ch == ",":
            self.ich = ich + 1
            ls_ret = LexSym(LexKind.COMMA, ch, ich, ich + 1)
        elif (ch == '"') or (ch == "'"):
            self.ich = ich
            ls_ret = self._get_next_lex_sym_string()
        elif OpInfo.is_op(ch):
            self.ich = ich
            ls_ret = self._get_next_lex_sym_operator()
        elif ch.isnumeric():
            self.ich = ich
            ls_ret = self._get_next_lex_sym_number()
        elif (ch == "_") or ch.isalpha():
            self.ich = ich
            ls_ret = self._get_next_lex_sym_identifier()
        else:
            self.ich = ich + 1
            ls_ret = LexSym(LexKind.CHAR, ch, ich, ich + 1)

        return ls_ret

    def _get_next_lex_sym_identifier(self):
        """Complete indentifying and return an identifier lexsym"""
        cch = self.cch
        ich = self.ich
        ich_start = ich
        ch = None

        ich += 1
        while ich < cch:
            ch = self.str_source[ich]
            if (ch != "_") and not ch.isalnum():
                break
            ich += 1

        value = self.str_source[ich_start:ich]
        if (ich < cch) and (ch == "("):
            ich += 1
            self.ich = ich
            ls_ret = LexSym(LexKind.FUNCTION_CALL, value, ich_start, ich)
        elif value.lower() == "true":
            self.ich = ich
            ls_ret = LexSym(LexKind.VALUE, True, ich_start, ich)
        elif value.lower() == "false":
            self.ich = ich
            ls_ret = LexSym(LexKind.VALUE, False, ich_start, ich)
        else:
            self.ich = ich
            ls_ret = LexSym(LexKind.ID, value, ich_start, ich)

        return ls_ret

    def _get_next_lex_sym_number(self):
        """Complete indentifying and return a number lexsym"""
        cch = self.cch
        ich = self.ich
        ich_start = ich
        ch = None

        # Parse integer part
        while ich < cch:
            ch = self.str_source[ich]
            if ch == ".":
                break
            elif not ch.isnumeric():
                break

            ich += 1

        # Parse fraction part, if any
        if ch != ".":
            value = int(self.str_source[ich_start:ich])
        else:
            ich += 1
            while ich < cch:
                ch = self.str_source[ich]
                if not ch.isnumeric():
                    break

                ich += 1

            value = float(self.str_source[ich_start:ich])

        self.ich = ich
        return LexSym(LexKind.VALUE, value, ich_start, ich)

    def _get_next_lex_sym_operator(self):
        """Complete indentifying and return an operator lexsym"""
        cch = self.cch
        ich = self.ich
        ich_start = ich
        ch = self.str_source[ich]
        ich += 1

        if ch == "!":
            self.ich = ich
            return LexSym(LexKind.OPERATOR_UNARY, ch, ich_start, ich)
        elif (ch == "<") and (ich < cch) and (self.str_source[ich] == "="):
            ch = "<="
            ich += 1
        elif (ch == ">") and (ich < cch) and (self.str_source[ich] == "="):
            ch = ">="
            ich += 1
        elif (ch == "=") or (ch == "|") or (ch == "&"):
            if (ich < cch) and (self.str_source[ich] == ch):
                ch += ch
                ich += 1
            else:
                self.ich = ich
                return LexSym(LexKind.CHAR, ch, ich_start, ich)

        self.ich = ich
        return LexSym(LexKind.OPERATOR, ch, ich_start, ich)

    def _get_next_lex_sym_string(self):
        """Complete indentifying and return a literal string lexsym"""
        cch = self.cch
        ich = self.ich
        ch_delimiter = self.str_source[ich]
        ich += 1

        ich_start = ich
        escape_ich = 0
        escape_mode = 0
        value = ""
        is_done = False

        while ich < cch:
            ch = self.str_source[ich]

            do_again = True
            while do_again:
                do_again = False
                if escape_mode == 1:
                    if ch in self.map_escape_sequence_char:
                        value += self.map_escape_sequence_char[ch]
                        escape_mode = 0
                    elif (ch == "X") or (ch == "x") or (ch == "U") or (ch == "u"):
                        escape_ich = ich - 1
                        escape_mode = 2  # Hexadecimal value or universal character name
                    elif (ch >= "0") and (ch <= "7"):
                        escape_ich = ich - 1
                        escape_mode = 10  # Octal value
                    else:
                        value += ch
                        escape_mode = 0
                elif escape_mode == 2:  # Hexdecimal value or universal character names
                    if ch not in string.hexdigits:
                        value += ast.literal_eval(
                            '"' + self.str_source[escape_ich:ich] + '"'
                        )
                        escape_mode = 0
                        do_again = True

                elif (escape_mode >= 10) and (escape_mode <= 12):  # Octal value
                    if (ch >= "0") or (ch <= "7"):
                        escape_mode += 1
                    else:
                        escape_mode = 99

                    if escape_mode > 12:
                        value += ast.literal_eval(
                            '"' + self.str_source[escape_ich:ich] + '"'
                        )
                        escape_mode = 0
                        do_again = True

                elif ch == "\\":
                    escape_mode = 1

                elif ch == ch_delimiter:
                    is_done = True

                else:
                    value += ch

            if is_done:
                break

            ich += 1

        self.ich = ich + 1  # Skip closing delimiter
        return LexSym(
            LexKind.VALUE, value, ich_start, ich
        )  # Return string without delimiters
