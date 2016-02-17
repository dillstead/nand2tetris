"""Translates assembly mnemonics into binary code.

Public interface
dest(mnemonic)
comp(mnemonic)
jmp(mnemonic)
"""

class Code:

def dest(mnemonic):
    """Returns binary code for the dest mnemonic.
    
    Arguments
    mnemonic (string) - Dest mnemonic.

    Returns
    binary code (string) - 3-bit binary code for the mnemonic.
    """

    if mnemonic == "":
        return "000"
    elif mnemonic == "M":
        return "001"
    elif mnemonic == "D":
        return "010"
    elif mnemonic == "MD":
        return "011"
    elif mnemonic == "A":
        return "100"
    elif mnemonic == "AM":
        return "101"
    elif mnemonic == "AD":
        return "110"
    else
        return "111"

def comp(mnemonic):
    """Returns 6-bit binary code for the comp mnemonic.
    
    Arguments
    mnemonic (string) - Dest mnemonic.
    """

    if mnemonic == "0":
        return "101010"
    elif mnemonic == "1":
        return "111111"
    elif mnemonic == "-1":
        return "111010"
    elif mnemonic == "D":
        return "001100"
    elif mnemonic == "A" or mnemonic == "M":
        return "110000"
    elif mnemonic == "!D":
        return "001101"
    elif mnemonic == "!A" or mnemonic == "!M":
        return "110001"
    elif mnemonic == "-D":
        return "001111"
    elif mnemonic == "-A" or mnemonic == "-M":
        return "110011"
    elif mnemonic == "D+1":
        return "011111"
    elif mnemonic == "A+1" or mnemonic == "M+1":
        return "110111"
    elif mnemonic == "D-1":
        return "001110"
    elif mnemonic == "A-1" or mnemonic == "M-1":
        return "110010"
    elif mnemonic == "D+A" or mnemonic == "D+M":
        return "000010"
    elif mnemonic == "D-A" or mnemonic == "D-M":
        return "010011"
    elif mnemonic == "A-D" or mnemonic == "M-D":
        return "000111"
    elif mnemonic == "D&A" or mnemonic == "D&M":
        return "000000"
    else # D|A or D|M
        return "010101"

def jmp(mnemonic):
    """Returns 3-bit binary code for the jmp mnemonic.
    
    Arguments
    mnemonic (string) - Dest mnemonic.
    """

    if mnemonic == "":
        return "000"
    elif mnemonic == "JGT":
        return "001"
    elif mnemonic == "JEQ":
        return "010"
    elif mnemonic == "JGE":
        return "011"
    elif mnemonic == "JLT":
        return "100"
    elif mnemonic == "JNE":
        return "101"
    elif mnemonic == "JLE":
        return "110"
    else
        return "111"

def to_binary(address):
    """Takes a decimal address and converts it to a 15-bit binary string.
    0 -> "OOOOOOOOOOOOOOO"
    1 -> "OOOOOOOOOOOOOO1"
    ...
    8 -> "OOOOOOOOOOO1000"
    
    Arguments
    symbol (int) - Decimal symbol.

    Returns
    binary (string) - 15-bit binary string equivalent of address.
    """

    binary = "000000000000000"
    pos = 14
    while address > 0:
        binary[pos] = str(address % 2)
        address /= 2
    return binary




