#!/usr/bin/env python3

import os, sys, string

# Locate the bundled module both in the build tree (../lib) and after a normal
# CMake installation (../share/lpmd/python), without requiring PYTHONPATH.
_script_dir = os.path.dirname(os.path.realpath(__file__))
for _module_dir in (
    os.path.normpath(os.path.join(_script_dir, '..', 'lib')),
    os.path.normpath(os.path.join(_script_dir, '..', 'share', 'lpmd', 'python')),
):
    if os.path.isdir(os.path.join(_module_dir, 'lpplotter')):
        sys.path.insert(0, _module_dir)
        break
from lpplotter import *
from lpplotter.makeframes import *
from lpplotter.povscene import *
from lpplotter.parselpmd2 import *


#
# VERSION 0.1
#

syntax = { 'input': ('file', 'start=', 'end=', 'each='),
           'antialias': ('value',),
           'radius':('value',),
           'box': ('radius','color',),
           'cameraLocation': ('position',),
           'cameraUp': ('position',),
           'cameraLookat': ('position',),
           'cameraLight':('value',),
           'extraLight':('value',),
           'cameraAngle':('value',),
           'cameraRotate':('poslook','rotvect','value',),
	   'startrotate':('value',),
	   'endrotate':('value',),
           'plane1':('color', 'transparency',),
           'plane2':('color', 'transparency',),
           'plane3':('color', 'transparency',),
           'plane4':('color', 'transparency',),
           'plane5':('color', 'transparency',),
           'plane6':('color', 'transparency',),
           'atomcolor':('color',),
           'camera':('value',),
           'startframes':('value',),
           'finalframes':('value',),
           'background': ('color',),
           'size': ('size',),
           'format': ('value',),
           'povfiles': ('value',),
           'warning': ('value',),
           'logo': ('type', 'expression=',),
           'movie': ('value', 'file', 'format'),
           'color': ('type', 'tag', 'expression='), }

allow_multiple = ('color',)

#
#
#

def ignorable(x): return x.strip() == '' or x.strip().startswith('#')

def parsetoken(x):
    if x.lower() in ('true', 'false'): return x.lower() == 'true'
    if x.startswith('<') and x.endswith('>'): return tuple(parsetoken(z.strip()) for z in x[1:-1].split(','))
    if not all((z in string.digits+'.+-eE') for z in x): return x
    try: return eval(x)
    except: return x

def substitute(x, d): return (x if x not in d else d[x])

def any_contains(seq, value):
    for x in seq:
        if value in x: return True
    return False

def protect_quotes(txt):
    new_txt, quoted_flag = list(), False
    for i in range(len(txt)):
        if txt[i] == '\"': quoted_flag = not quoted_flag
        if quoted_flag and txt[i] in string.whitespace: new_txt.append('\0')
        elif txt[i] != '\"': new_txt.append(txt[i])
    return ''.join(new_txt)

def unprotect_quotes(txt):
    new_txt = list()
    for i in range(len(txt)):
        if txt[i] == '\0': new_txt.append(' ')
        else: new_txt.append(txt[i])
    return ''.join(new_txt)

def protect_brackets(txt, openbrac, closebrac):
    if not openbrac in txt and not closebrac in txt: return (txt, {})
    cnt, d, p0, newtxt = 0, {}, 0, ''
    while openbrac in txt[p0:]:
        p1 = txt.find(openbrac, p0)
        newtxt += txt[p0:p1]
        p2 = txt.find(closebrac, p1) 
        newtxt += '%'+str(cnt)
        d['%'+str(cnt)] = openbrac+txt[p1+1:p2]+closebrac
        p0, cnt = (p2+1), cnt+1
    return (newtxt+txt[p2+1:], d)

class SyntaxError(Exception):

    def __init__(self, line):
        self.line = line 

    def __str__(self): return '[Error] Syntax error, \'%s\'' % self.line

class ControlParser(dict):

    def __init__(self, control=None):
        if control != None: self.Parse(control)

    def Parse(self, control):
        for cmd in allow_multiple: self[cmd] = list()
        for line in open(control):
            if ignorable(line): continue
            line = line.strip()
            # Protect quotes 
            line = protect_quotes(line)
            # Protect angle brackets
            line, d = protect_brackets(line, '<', '>')
            lspl = [parsetoken(unprotect_quotes(substitute(x, d)).strip()) for x in line.split()]
            cmd, args, options = lspl[0], lspl[1:], dict()
            if cmd not in syntax: raise SyntaxError(line)
            if cmd not in allow_multiple and cmd in list(self.keys()): raise SyntaxError(line)
            if not any_contains(syntax[cmd], '=') and len(args) != len(syntax[cmd]): 
               raise SyntaxError(line)
            for i, x in enumerate(syntax[cmd]): 
                if i < len(args): options[x] = args[i]
                else: options[x] = None
            if cmd in list(self.keys()): self[cmd].append(options)
            else: self[cmd] = options

    def Show(self):
        for k in list(self.keys()): print(k, self[k])

#
#
#
if __name__ == '__main__':
 if len(sys.argv) < 2: sys.exit('Usage: lpmd-plotter <control-file>')
 c = ControlParser(sys.argv[1])
 RunRender(c)
