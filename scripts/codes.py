#!/usr/bin/env python3

import sys
import re

print(repr([int(re.sub(r'\D', '', x)) / 255.0 for x in sys.argv[1:]]))
