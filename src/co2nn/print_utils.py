import sys
import os
from numpy import float64

try:
    TERM_COLUMNS = int(os.popen('stty size 2>/dev/null', 'r').read().split()[1])
    RESET_COLOR = '\033[0m'
    ERROR_COLOR = '\033[91m'
    INFO_COLOR = '\033[92m'
    DEBUG_COLOR = '\033[93m'
    GRAPH_COLOR = '\033[94m'
    STRESS_COLOR = '\033[95m'
except:
    TERM_COLUMNS = 80
    RESET_COLOR = ''
    ERROR_COLOR = ''
    INFO_COLOR = ''
    DEBUG_COLOR = ''
    GRAPH_COLOR = ''
    STRESS_COLOR = ''

TERM_SEPLINE = "=" * TERM_COLUMNS


def print_sep():
    print(GRAPH_COLOR + TERM_SEPLINE + RESET_COLOR, file=sys.stderr)


def print_info(*parts):
    parts = map(str, parts)
    print(GRAPH_COLOR + '>>>>>>>> ' + INFO_COLOR + "".join(parts) + RESET_COLOR, file=sys.stderr)


def print_error(*parts):
    parts = map(str, parts)
    txt = GRAPH_COLOR + '!!!!!!!! ' + ERROR_COLOR + "".join(parts) + RESET_COLOR
    print(txt, file=sys.stderr)

def ff(x: float64) -> str:
    return ('%.20f' % x).rstrip('0').rstrip('.')
