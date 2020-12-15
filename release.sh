#!/bin/bash

PRJNAME=libqhr

if [ $# -lt 2 ]
then
    echo "Missing version and output directory"
    exit 1
fi

VERSION=$1
shift
OUTPUTDIR=$1
shift

RUNNPM=1
SIGNTARBALL=1

if [ -z $VERSION ]
then
    echo "Missing version";
    exit 1;
fi

if [ -z "$OUTPUTDIR" ]
then
    echo "Missing output directory";
    exit 2;
fi

while [ -n "$1" ]
do
    case $1 in
        --no-signing) SIGNTARBALL=0;;
    esac
    shift
done

TMPDIR=$(mktemp -d)

DIR="${TMPDIR}/${PRJNAME}-${VERSION}"

mkdir $DIR

for SRCDIR in cmake dox QHR tests
do
    cp -r $SRCDIR $DIR
done

for SRCFILE in CMakeLists.txt COPYING COPYING.LESSER README.md
do
    cp $SRCFILE $DIR
done

pushd $TMPDIR
rm -f "${OUTPUTDIR}/${PRJNAME}-${VERSION}.tar.xz"
# rm -f "${OUTPUTDIR}/${PRJNAME}-${VERSION}.tar.gz"
tar -cJf "${OUTPUTDIR}/${PRJNAME}-${VERSION}.tar.xz" ${PRJNAME}-${VERSION}
# tar -czf "${OUTPUTDIR}/${PRJNAME}-${VERSION}.tar.gz" ${PRJNAME}-${VERSION}
rm -rf ${PRJNAME}-${VERSION}
popd

if [ $SIGNTARBALL -gt 0 ]
then
    gpg --armor --detach-sign --yes --default-key 6607CA3F41B25F45 --output "${OUTPUTDIR}/${PRJNAME}-${VERSION}.tar.xz.sig" "${OUTPUTDIR}/${PRJNAME}-${VERSION}.tar.xz"
#     gpg --armor --detach-sign --yes --default-key 6607CA3F41B25F45 --output "${OUTPUTDIR}/${PRJNAME}-${VERSION}.tar.gz.sig" "${OUTPUTDIR}/${PRJNAME}-${VERSION}.tar.gz"
fi
exit 0
