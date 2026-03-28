#!/bin/bash

#  ./compile-all.sh > compile.log 2>&1 &


# Global environment variables

# need more swap for compiling cidlc
swapon /dev/hdb1
# Log
export LOG="`pwd`/ACE.log"
echo "" >> ${LOG}
echo "" >> ${LOG}
echo "" >> ${LOG}
echo "------> BEGIN `date`" >> ${LOG}
echo "" >> ${LOG}
echo "LOG      = ${LOG}" >> ${LOG}
# Using ccache
export PATH="/usr/bin:/bin"
#export PATH="/usr/local/bin:${PATH}"
echo "PATH     = ${PATH}" >> ${LOG}
# Compiler Flags
export CFLAGS="-O3 -mcpu=i686 -pipe"
echo "CFLAGS   = ${CFLAGS}" >> ${LOG}
# Library features
export FEATURES="-features ssl=1,qos=1,rapi=1,cidlc=1,exceptions=1"
echo "FEATURES = ${FEATURES}" >> ${LOG}
# Make options
export MAKEOPTS="-j3 static_libs=1 ssl=1 rapi=1 cidl=1 exceptions=1"
echo "MAKEOPTS = ${MAKEOPTS}" >> ${LOG}
# mwc files for generating Makefiles
ACE_MWC="ACE.mwc"
TAO_MWC="TAOACE.mwc"
CIAO_MWC="CIAOTAO.mwc"
CIDLC_MWC="CIDLC.mwc"

echo "ACE_MWC   = ${ACE_MWC}" >> ${LOG}
echo "TAO_MWC   = ${TAO_MWC}" >> ${LOG}
echo "CIAO_MWC  = ${CIAO_MWC}" >> ${LOG}
echo "CIDLC_MWC = ${CIDLC_MWC}" >> ${LOG}

# Get ACE+TAO+CIAO sources via CVS
#export CVSROOT=":pserver:anonymous@anoncvs.ociweb.com:/cvs"
#echo "CVSROOT = ${CVSROOT}" >> ${LOG}

# Loggin into the server, just press <Enter>  when prompted for the password.
#test -d ACE_wrappers || cvs login
# Making your first checkout*:
#test -d ACE_wrappers || ((cvs -z 9 checkout ACE_wrappers && cd ACE_wrappers) && cvs -z 9 checkout MPC)

# Make sure you have xerces-c and boost installed (emerge xerces-c boost openssl perl) !!
echo 'Make sure you have the BOOST, OPENSSL and the  XERCES-C libraries and PERL installed !'

# Get Cidlc: for now it is compiled directly rather then downloaded !
#CIDLCVERSION="0.4.3"
##CIDLC="cidlc-${CIDLCVERSION}"
#CIDLC="cidlc"
#CIDLCSOURCE="http://www.dre.vanderbilt.edu/~isisbuilds/CIDLC/gnu-linux/${CIDLC}.bz2"
## CIDLCSOURCE="http://www.dre.vanderbilt.edu/cidlc/binary/gnu-linux/i386/${CIDLC}.bz2"
#echo "CIDLCSOURCE     = ${CIDLCSOURCE}"  >> ${LOG}
#test -f "${CIDLC}" || ( (wget ${CIDLCSOURCE} && bunzip2 "${CIDLC}.bz2") || echo "ERROR: downloading cidlc compiler" )  >> ${LOG}
#echo "Got cidl compiler: finished"  >> ${LOG}

# get Utility library
UTILITY="Utility-1.2.2"
UTILITYSOURCE="http://www.dre.vanderbilt.edu/cidlc/prerequisites/${UTILITY}.tar.bz2"
#UTILITYSOURCE="http://www.dre.vanderbilt.edu/cidlc/prerequisites/Utility-1.2.2.tar.bz2"
echo "UTILITYSOURCE     = ${UTILITYSOURCE}"  >> ${LOG}
test -d ${UTILITY} || ( (wget ${UTILITYSOURCE} && tar -xvjf "${UTILITY}.tar.bz2") || echo "ERROR: downloading Utility lib" )  >> ${LOG} 
export UTILITY_ROOT="`pwd`/${UTILITY}"
echo "UTILITY_ROOT = ${UTILITY_ROOT}" >> ${LOG}
export BOOST_ROOT="/usr"
export BOOST_LIB="/usr/lib"
export BOOST_INCLUDE="/usr/include/boost"

echo "BOOST_ROOT    = ${BOOST_ROOT}" >> ${LOG}
echo "BOOST_LIB     = ${BOOST_LIB}" >> ${LOG}
echo "BOOST_INCLUDE = ${BOOST_INCLUDE}" >> ${LOG}

# CVS update
cd ACE_wrappers
#cvs -z 9 update -d || exit 1
#echo "Updated ACE_wrappers from CVS"  >> ${LOG}
#cd MPC
#cvs -z 9 update -d || exit 1
#echo "Updated MPC from CVS"  >> ${LOG}
#cd ..

# Evironment variables
export ACE_ROOT="`pwd`"
export TAO_ROOT="${ACE_ROOT}/TAO"
export CIAO_ROOT="${TAO_ROOT}/CIAO"
export LD_LIBRARY_PATH="${ACE_ROOT}/lib"

echo "ACE_ROOT        = ${ACE_ROOT}" >> ${LOG}
echo "TAO_ROOT        = ${TAO_ROOT}" >> ${LOG}
echo "CIAO_ROOT       = ${CIAO_ROOT}" >> ${LOG}
echo "LD_LIBRARY_PATH = ${LD_LIBRARY_PATH}" >> ${LOG}

# Cidlc is compiled and not downloaded for now !
#cp -vf ${ACE_ROOT}/../${CIDLC} ${CIAO_ROOT}/bin/cidlc || (echo "ERROR: copying  ${CIAO_ROOT}/bin/cidlc"  >> ${LOG})
#chmod 755  ${CIAO_ROOT}/bin/cidlc || (echo "ERROR: chmod 755  ${CIAO_ROOT}/bin/cidlc"  >> ${LOG})
#echo "Copied cidl compiler: finished"  >> ${LOG}
## Cleaning up all GNUmakefiles and libs since make realclean does not work properly for now


# Making clean
cd "${ACE_ROOT}"
make realclean || (echo "ACE: ERROR making realclean" >> ${LOG})
echo "ACE: making realclean: finished" >>  ${LOG}
cd ${ACE_ROOT}/apps/gperf
make realclean || (echo "gperf: ERROR making realclean" >> ${LOG})
echo "gperf: making realclean: finished" >>  ${LOG}
cd  ${TAO_ROOT}
make realclean || (echo "TAO: ERROR making realclean" >> ${LOG})
echo "TAO: making realclean: finished" >>  ${LOG}
cd  ${CIAO_ROOT}/CIDLC
make realclean || (echo "CIDLC: ERROR making realclean" >> ${LOG})
echo "CIDLC: making realclean: finished" >>  ${LOG}
cd  ${CIAO_ROOT}
make realclean || (echo "CIAO: ERROR making realclean" >> ${LOG})
echo "CIAO: making realclean: finished" >>  ${LOG}
# for now remove some files 'by hand' cause make realclean seems to be broken
find . -name "*GNUmakefile*" -o -name "*.o" -o -name "*.so.*" -o -name "*.so" -o -name "*.a" -exec rm -vf {} \;
rm -vf ${ACE_ROOT}/lib/*

# Platform specific configuration
cd "${ACE_ROOT}/ace"
cp config-linux.h config.h
cd "${ACE_ROOT}/include/makeinclude"
sed -e "s:-O3:${CFLAGS}:" platform_linux.GNU >platform_macros.GNU
# make switches
sed -i "s/ssl.*$/ssl            = 1/g" ${ACE_ROOT}/bin/MakeProjectCreator/config/global.features
sed -i "s/qos.*$/qos            = 1/g" ${ACE_ROOT}/bin/MakeProjectCreator/config/global.features
sed -i "s/rapi.*$/rapi          = 1/g" ${ACE_ROOT}/bin/MakeProjectCreator/config/global.features
sed -i "s/cidl.*$/cidl          = 1/g" ${ACE_ROOT}/bin/MakeProjectCreator/config/global.features

echo "Finished Platform specific configuration"  >> ${LOG}



## Making ACE ##
cd "${ACE_ROOT}"
echo "---> Making ACE: `date`"  >> ${LOG}

# Generate the Makefiles
${ACE_ROOT}/bin/mwc.pl ${FEATURES}  ${ACE_MWC} ||  (echo "ACE: ERROR generating Makefiles with \"${FEATURES}\" for ${ACE_MWC}"  >> ${LOG})
echo "ACE: generated Makefiles with \"${FEATURES}\" for ${ACE_MWC}: finished"  >> ${LOG}

make ${MAKEOPTS} depend || (echo "ACE: ERROR making depend" >> ${LOG})
echo "ACE: making depend: finished" >>  ${LOG}
make ${MAKEOPTS} || (echo "ACE: ERROR making all" >> ${LOG})
echo "ACE: making all: finished" >>  ${LOG}


## Making gperf ##
cd ${ACE_ROOT}/apps/gperf
echo "---> Making gperf: `date`"  >> ${LOG}
make ${MAKEOPTS} depend || (echo "gperf: ERROR making depend" >> ${LOG})
echo "gperf: making depend: finished" >>  ${LOG}
make ${MAKEOPTS} || (echo "gperf: ERROR making all" >> ${LOG})
echo "gperf: making all: finished" >>  ${LOG}


## Making TAO ##
cd  ${TAO_ROOT}
echo "---> Making TAO: `date`"  >> ${LOG}

# Generate the Makefiles
${ACE_ROOT}/bin/mwc.pl ${FEATURES}  ${TAO_MWC} || (echo "TAO: ERROR  generating Makefiles with \"${FEATURES}\" for ${TAO_MWC}" >> ${LOG})
echo "TAO: generated Makefiles with \"${FEATURES}\" for ${TAO_MWC}: finished"  >> ${LOG}

make ${MAKEOPTS} depend || (echo "TAO: ERROR making depend" >> ${LOG})
echo "TAO: making depend: finished" >>  ${LOG}
make ${MAKEOPTS} || (echo "TAO: ERROR making all" >> ${LOG})
echo "TAO: making all: finished" >>  ${LOG}


##  Making CIDLC ##
cd  ${CIAO_ROOT}/CIDLC
echo "---> Making CIDLC: `date`"  >> ${LOG}

# Generate the Makefiles
${ACE_ROOT}/bin/mwc.pl ${FEATURES}  ${CIDLC_MWC} || (echo "CIDLC: ERROR  generating Makefiles with \"${FEATURES}\" for ${CIDLC_MWC}" >> ${LOG})
echo "CIDLC: generated Makefiles with \"${FEATURES}\" for ${CIDLC_MWC}: finished"  >> ${LOG}

make ${MAKEOPTS} depend || (echo "CIDLC: ERROR making depend" >> ${LOG})
echo "CIDLC: making depend: finished" >>  ${LOG}
make ${MAKEOPTS} || (echo "CIDLC: ERROR making all" >> ${LOG})
echo "CIDLC: making all: finished" >>  ${LOG}


##  Making CIAO ##
cd  ${CIAO_ROOT}
echo "---> Making CIAO: `date`"  >> ${LOG}

# Generate the Makefiles
${ACE_ROOT}/bin/mwc.pl ${FEATURES}  ${CIAO_MWC} || (echo "CIAO: ERROR  generating Makefiles with \"${FEATURES}\" for ${CIAO_MWC}" >> ${LOG})
echo "CIAO: generated Makefiles with \"${FEATURES}\" for ${CIAO_MWC}: finished"  >> ${LOG}

make ${MAKEOPTS} depend || (echo "CIAO: ERROR making depend" >> ${LOG})
echo "CIAO: making depend: finished" >>  ${LOG}
make ${MAKEOPTS} || (echo "CIAO: ERROR making all" >> ${LOG})
echo "CIAO: making all: finished" >>  ${LOG}



echo "" >> ${LOG}
echo "" >> ${LOG}
echo "------> FINISHED `date`" >> ${LOG}
exit 0

