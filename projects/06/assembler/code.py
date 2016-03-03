"""Translates assembly mnemonics into binary code.

Public interface
dest(mnemonic)
comp(mnemonic)
jmp(mnemonic)
"""

class Code:

    @staticmethod
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
        else:
            return "111"

    @staticmethod
    def comp(mnemonic):
        """Returns 6-bit binary code for the comp mnemonic.

        Arguments
        mnemonic (string) - Dest mnemonic.
        """
        if mnemonic == "0":
            return "0101010"
        elif mnemonic == "1":
            return "0111111"
        elif mnemonic == "-1":
            return "0111010"
        elif mnemonic == "D":
            return "0001100"
        elif mnemonic == "A":
            return "0110000"
        elif mnemonic == "M":
            return "1110000"
        elif mnemonic == "!D":
            return "0001101"
        elif mnemonic == "!A":
            return "0110001"
        elif mnemonic == "!M":
            return "1110001"
        elif mnemonic == "-D":
            return "0001111"
        elif mnemonic == "-A":
            return "0110011"
        elif mnemonic == "-M":
            return "1110011"
        elif mnemonic == "D+1":
            return "0011111"
        elif mnemonic == "A+1":
            return "0110111"
        elif mnemonic == "M+1":
            return "1110111"
        elif mnemonic == "D-1":
            return "0001110"
        elif mnemonic == "A-1":
            return "0110010"
        elif mnemonic == "M-1":
            return "1110010"
        elif mnemonic == "D+A":
            return "0000010"
        elif mnemonic == "D+M":
            return "1000010"
        elif mnemonic == "D-A":
            return "0010011"
        elif mnemonic == "D-M":
            return "1010011"
        elif mnemonic == "A-D":
            return "0000111"
        elif mnemonic == "M-D":
            return "1000111"
        elif mnemonic == "D&A":
            return "0000000"
        elif mnemonic == "D&M":
            return "1000000"
        elif mnemonic == "D|A":
            return "0010101"
        else: # D|M
            return "1010101"

    @staticmethod
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
        else:
            return "111"

    @staticmethod
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

        binary = ["0" for x in range(15)]
        pos = 14
        while address > 0:
            binary[pos] = str(address % 2)
            address /= 2
            pos -= 1
        return "".join(binary)
