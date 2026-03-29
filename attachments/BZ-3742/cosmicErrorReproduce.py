#! /usr/bin/env python

import os
import os.path
from string import Template

DIR_PREFIX   = 'dir'
DIR_DEPTH    = 2**3 # 8
MAX_NUM_FILE = 2**8 # 256

IDL_TEMPLATE_STR = '''\
#ifndef ${COMP_NAME}
#define ${COMP_NAME}

#include "Components.idl"

component ${compName}
{
   attribute long myAttr;
};

#endif
'''

idlTemplate = Template(IDL_TEMPLATE_STR)

# create file path
curDir  = os.getcwd()
dirPath = curDir
root    = ''
for level in range(DIR_DEPTH):
   dirPath = os.path.join(dirPath, DIR_PREFIX + str(level))
   if not root:
      root = dirPath

# make directories and change working dir to newly created dir
os.makedirs(dirPath)

count = 0
# walk directories and create files
for dirInfo in os.walk(root):
   curRoot = dirInfo[0]
   for fileNum in range(MAX_NUM_FILE // DIR_DEPTH):
      compName = 'MyComp' + str(count)
      count += 1
      fileName = compName + '_comp.idl'
      contents = idlTemplate.substitute(compName=compName, COMP_NAME=compName.upper())
      newFile = open(os.path.join(curRoot, fileName), 'w')
      newFile.write(contents)
      newFile.close()


