"""Breaks each assembly command into its underlying components
(fields and symbols).

Public interface
has_more_commands()
advance()
command_type()
symbol()
dest()
comp()
jmp()
"""

class Parser:
    A_COMMAND = 0
    C_COMMAND = 1
    L_COMMAND = 2

    def __init__(self, input_fname):
        """Constructor.
        
        Arguments
        input_fname (string) - Name of input file containing assembly code.
        """
        self._fin = open(input_fname, "r")

    def has_more_commands(self):
        """Checks to see if more commands are available.
        
        Returns
        True if there are more commands, False if not.
        """
        line = self._fin.readline()
        while line != "":
            commands = line.split()
            # Ignore blank lines and comments.
            if len(commands) > 0 and commands[0][0] != '/':
                self._command = commands[0]
                return True
            line = self._fin.readline()
        self._fin.close()
        return False

    def advance(self):
        """Breaks down the command into its constituent parts.
        """
        if self._command[0] == '@':
            self._command_type = Parser.A_COMMAND
            self._parse_a_command()
        elif self._command[1] == '(':
            self._command_type = Parser.L_COMMAND
            self._parse_l_command()
        else:
            self._command_type = Parser.C_COMMAND
            self._parse_c_command()

    def command_type(self):
        return self._command_type

    def symbol(self):
        return self._symbol

    def dest(self):
        return self._dest

    def comp(self):
        return self._comp

    def jmp(self):
        return self._jmp

    def _parse_a_command(self):
        # @symbol
        self._symbol = self._command[1:]

    def _parse_l_command(self):
        # (symbol)
        self._symbol = self._command[1:-1]

    def _parse_c_command(self):
        # dest=comp;jmp
        # Either dest or jmp fields may be empty.
        # If dest is empty the "=" is ommited.
        # If jmp is empty the ";" is ommited.
        comp_start = self._command.find('=')
        if comp_start != -1:
            self._dest = self._command[:comp_start]
            comp_start += 1
        else:
            self._dest = ""
            comp_start = 0
        comp_end = self._command.rfind(';')
        if comp_end != -1:
            self._comp = self._command[comp_start:comp_end]
            self._jmp = self._command[comp_end + 1:]
        else:
            self._comp = self._command[comp_start:]
            self._jmp = ""
