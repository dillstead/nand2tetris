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
    self.fin = open(input_fname)

def has_more_commands(self):
    """Checks to see if more commands are available.
    
    Returns
    True if there are more commands, False if not.
    """
    line = self.fin.readline()
    while line != "":
        commands = line.split()
        # Ignore blank lines and comments.
        if len(commands) > 0 and commands[0] != '/':
            self.command = commands[0]
            return True
        line = self.fin.readline()
    self.fin.close()
    return False

def advance(self):
    """Breaks down the command into its constituent parts.
    """
    if self.command[0] == '@':
        self.command_type = A_COMMAND
        _parse_a_command()
        x
    elif self.command[1] == '(':
        self.command_type = L_COMMAND
        _parse_l_command()
    else
        self.command_type = C_COMMAND
        _parse_c_command()

def symbol(self):
    return self.symbol

def dest(self):
    return self.dest

def comp(self):
    return self.comp

def jmp(self):
    return self.jmp


def _parse_a_command(self):
    # @symbol
    self.symbol = self.command[1:]

def _parse_l_command(self):
    # (symbol)
    self.symbol = self.command[1:-1]

def _parse_c_command(self):
    # dest=comp;jmp
    # Either dest or jmp fields may be empty.
    # If dest is empty the "=" is ommited.
    # If jmp is empty the ";" is ommited.
    comp_start = self.command.find('=')
    if comp_start != -1:
        self.dest = self.command[:comp_start]
        comp_start += 1
    else:
        self.dest = ""
    comp_end = self.command.rfind(';')
    if comp_end != -1:
        self.comp = self.command[comp_start:comp_end]
        self.jmp = self.command[comp_end + 1:]
    else:
        self.comp = self.command[comp_start:]
        self.jmp = ""
