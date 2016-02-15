"""Maps symbolic lables to number addresses in RAM and ROM.

Public interface
add_entry(symbol, address)
contains(symbol)
get_address(symbol)
"""

class Symbol_Table:

def __init__(self):
    """Constructor
    """

    self.address_map = {}

def add_entry(self, symbol, address):
    """Adds a entry to the symbol table.

    Arguments
    symbol (string) - Symbol to add.
    address (int) - Address to map the symbol to.
    """
    
    self.address_map[symbol] = address;

def contains(self, symbol):
    """Checks to see if a symbol already contains a mapping.

    Arguments
    symbol (string) - Symbol to check.

    Returns
    True if the symbol contains a mapping, False if it doesn't.
    """

    return self.address_map.has_key(symbol)

def get_address(self, symbol):
    """Returns the addreses that the symbol maps to.  Note the
    symbol must exist in the table before this is called.

    Arguments
    symbol (string) - Symbol whose adddress to retreive.

    Returns
    The address that the symbol maps to.
    """

    return self.address_map[symbol]
