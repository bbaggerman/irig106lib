""" Call return status """

OK                  =  0    # Everything okey dokey
OPEN_ERROR          =  1    # Fatal problem opening for read or write
OPEN_WARNING        =  2    # Non-fatal problem opening for read or write
EOF                 =  3    # End of file encountered
BOF                 =  4
READ_ERROR          =  5    # Error reading data from file
WRITE_ERROR         =  6    # Error writing data to file
MORE_DATA           =  7
SEEK_ERROR          =  8
WRONG_FILE_MODE     =  9
NOT_OPEN            = 10
ALREADY_OPEN        = 11
BUFFER_TOO_SMALL    = 12
NO_MORE_DATA        = 13
NO_FREE_HANDLES     = 14
INVALID_HANDLE      = 15
TIME_NOT_FOUND      = 16
HEADER_CHKSUM_BAD   = 17
NO_INDEX            = 18
UNSUPPORTED         = 19
BUFFER_OVERRUN      = 20
INDEX_NODE          = 21    # Returned decoded node message
INDEX_ROOT          = 22    # Returned decoded root message
INDEX_ROOT_LINK     = 23    # Returned decoded link to next root (i.e. last root)
INVALID_DATA        = 24    # Packet data is invalid for some reason

def Message (StatusNum):
    if   StatusNum == OK                : return "OK"
    elif StatusNum == OPEN_ERROR        : return "Open Error"
    elif StatusNum == OPEN_WARNING      : return "Open Warning"
    elif StatusNum == EOF               : return "End of File"
    elif StatusNum == BOF               : return "Beginning of File"
    elif StatusNum == READ_ERROR        : return "Read Error"
    elif StatusNum == WRITE_ERROR       : return "Write Error"
    elif StatusNum == MORE_DATA         : return "More Data"
    elif StatusNum == SEEK_ERROR        : return "Seek Error"
    elif StatusNum == WRONG_FILE_MODE   : return "Wrong File Mode"
    elif StatusNum == NOT_OPEN          : return "File Not Open"
    elif StatusNum == ALREADY_OPEN      : return "File Already Open"
    elif StatusNum == BUFFER_TOO_SMALL  : return "Buffer Too Small"
    elif StatusNum == NO_MORE_DATA      : return "No More Data"
    elif StatusNum == NO_FREE_HANDLES   : return "No Free Handles"
    elif StatusNum == INVALID_HANDLE    : return "Invalid Handle"
    elif StatusNum == TIME_NOT_FOUND    : return "Time Not Found"
    elif StatusNum == HEADER_CHKSUM_BAD : return "Header Checksum Bad"
    elif StatusNum == NO_INDEX          : return "No Index"
    elif StatusNum == UNSUPPORTED       : return "Unsupported Operation"
    elif StatusNum == BUFFER_OVERRUN    : return "Buffer Overrun"
    elif StatusNum == INDEX_NODE        : return "Node Index Returned"
    elif StatusNum == INDEX_ROOT        : return "Root Index Returned"
    elif StatusNum == INDEX_ROOT_LINK   : return "Root Index Link"
    elif StatusNum == INVALID_DATA      : return "Invalid Data"
    else                                : return "Undefined"

