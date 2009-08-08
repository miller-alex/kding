#!/bin/sh

PROJECT="kding"     # project name
WORKINGDIR=`pwd`    # working directory
PODIR="po"

echo "Preparing rc files"
# we use simple sorting to make sure the lines do not jump around too much from system to system
find . -name '*.rc' -o -name '*.ui' -o -name '*.kcfg' | sort > ${WORKINGDIR}/rcfiles.list
xargs --arg-file=${WORKINGDIR}/rcfiles.list extractrc > ${WORKINGDIR}/rc.cpp
# additional strings for KAboutData
#echo 'i18nc("NAME OF TRANSLATORS","Your names");' >> ${WORKINGDIR}/rc.cpp
#echo 'i18nc("EMAIL OF TRANSLATORS","Your emails");' >> ${WORKINGDIR}/rc.cpp

echo "Extracting messages"
# see above on sorting
find . -name '*.cpp' -o -name '*.h' -o -name '*.c' | sort > ${WORKINGDIR}/infiles.list
echo "rc.cpp" >> ${WORKINGDIR}/infiles.list
xgettext --from-code=UTF-8 -C -kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 -ktr2i18n:1 \
         -kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3 \
         --files-from=infiles.list -D ${WORKINGDIR} -o ${PODIR}/${PROJECT}.pot || { echo "Error while calling xgettext. Aborting."; exit 1; }

echo "Merging translations"
cd ${PODIR}
catalogs=`find . -name '*.po'`
for cat in $catalogs; do
    echo "Processing ${cat}"
    msgmerge -o $cat.new $cat ${PROJECT}.pot
    mv $cat.new $cat
done
cd ${WORKINGDIR}

echo "Cleaning up"
rm rcfiles.list
rm infiles.list
rm rc.cpp
