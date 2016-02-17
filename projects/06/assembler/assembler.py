import sys
import os
from symbol_table import SymbolTable
from parser import Parser

def _init_symbol_table(symbol_table)
    """Initializes the symbol table with predefined symbols.

    Arguments
    symbol_table (SymbolTable) - Symbol table to initialize.
    """

    symbol_table.add_entry("SP", 0)
    symbol_table.add_entry("LCL", 1)
    symbol_table.add_entry("ARG", 2)
    symbol_table.add_entry("THIS", 3)
    symbol_table.add_entry("THAT", 4)
    symbol_table.add_entry("RO", 0)
    symbol_table.add_entry("R1", 1)
    symbol_table.add_entry("R2", 2)
    symbol_table.add_entry("R3", 3)
    symbol_table.add_entry("R4", 4)
    symbol_table.add_entry("R5", 5)
    symbol_table.add_entry("R6", 6)
    symbol_table.add_entry("R7", 7)
    symbol_table.add_entry("R8", 8)
    symbol_table.add_entry("R9", 9)
    symbol_table.add_entry("R10", 10)
    symbol_table.add_entry("R11", 11)
    symbol_table.add_entry("R12", 12)
    symbol_table.add_entry("R13", 13)
    symbol_table.add_entry("R14", 14)
    symbol_table.add_entry("R15", 15)
    symbol_table.add_entry("SCREEN", 16384)
    symbol_table.add_entry("KEYBOARD", 24576)

def _first_pass(input_fname, symbol_table):
    """Add all of the labels to the symbol table so they are
    available for the second pass.

    Arguments
    input_fname (string) - Name of the input file containing assembly code.
    symbol_table (SymbolTable) - Symbol table.
    """

    # Labels start at ROM address 0.
    rom_address = 0
    parser = Parser(input_fname)
    while parser.has_more_commands():
        parser.advance()
        if parser.command_type() == Parser.L_COMMAND:
            symbol_table.add_entry(parser.symbol(), rom_address)
        else:
            rom_address += 1

def _second_pass(input_fname, symbol_table, output_fname):
    """Go through each line of assembly code and generate machine code.

    Arguments
    input_fname (string) - Name of the input file containing assembly code.
    symbol_table (SymbolTable) - Symbol table.
    output_fname (string) - Output file name of a file to write machine code to.
    """

    # Variable symbols start at RAM address 16.
    ram_address = 16
    fout = open(input_fname, "w")
    parser = Parser(input_fname)
    while parser.has_more_commands():
        parser.advance()
        if parser.command_type() == Parser.A_COMMAND:
            symbol = parser.symbol()
            try:
                # If the symbol is a decimal number convert it to a binary string.
                fout.write("0" + Code.to_binary(int(symbol)) + "\n")                
            except ValueError:
                # The symbol isn't a decimal number thus it must either be in the symbol table or it needs to be
                # assigned a value.
                if !symbol_table.contains(symbol):
                    symbol_table.add_entry(symbol, ram_address)
                    ram_address += 1
                fout.write("0" + Code.to_binary(symbol_table.get_address(symbol)) + "\n")
        elif parser.command_type() == Parser.C_COMMAND:
            fout.write("111" + Code.dest(parser.dest()) + Code.comp(parser.comp()) + Code.jmp(parser.jmp()) + "\n")
        # Ignore L_COMMAND, they have been looked at in the first pass.
    fout.close()

def main(input_fname):
    """Asssembles the code.

    Arguments
    input_fname (string) - Name of the input file containing assembly code.
    """

    symbol_table = SymbolTable()
    _init_symbol_table(symbol_table)
    _first_pass(input_fname, symbol_table)
    output_fname = os.path.splitext(input_fname)[1] + ".hack"
    _second_pass(input_fname, symbol_table, output_fname)

if __name__ == '__main__': 
    main(sys.argv[1])
