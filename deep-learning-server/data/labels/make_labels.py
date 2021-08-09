# AUTHORS

# The Veracruz Development Team.

# COPYRIGHT AND LICENSING

# See the `LICENSE_MIT.markdown` file in the Veracruz deep learning server 
# example repository root directory for copyright and licensing information.

#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import string
import pipes

def make_labels(s):
    l = string.printable
    for word in l:
        if word == ' ':
            os.system('convert -fill black -background white -bordercolor white -pointsize %d label:"\ " data/labels/32_%d.png'%(s,s/12-1))
        if word == '@':
            os.system('convert -fill black -background white -bordercolor white -pointsize %d label:"\@" data/labels/64_%d.png'%(s,s/12-1))
        elif word == '\\':
            os.system('convert -fill black -background white -bordercolor white -pointsize %d label:"\\\\\\\\" data/labels/92_%d.png'%(s,s/12-1))
        elif ord(word) in [9,10,11,12,13,14]:
            pass
        else:
            os.system("convert -fill black -background white -bordercolor white -pointsize %d label:%s \"data/labels/%d_%d.png\""%(s,pipes.quote(word), ord(word),s/12-1))

for i in [12,24,36,48,60,72,84,96]:
    make_labels(i)