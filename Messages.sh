#!/bin/sh

cd "$(dirname "$0")"

PROJECT="kding"
PODIR="po"
POT="${PODIR}/${PROJECT}.pot"

# extract version number from KAboutData
VERSION="$(sed -n 's/.*about([^,]*,[^,]*, *"\([0-9.]*\)[-"].*/\1/p' \
	< src/main.cpp)"

XGETTEXT_OPTS="-F --from-code=UTF-8 -C -kde -ci18n
	-ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3
	-ktr2i18n:1  -kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale
	-kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3
	--package-name=${PROJECT} --package-version=${VERSION}"
MSGMERGE_OPTS=''

[ $# -eq 0 ] && set -- x u
extract= mergepo=
while [ $# -gt 0 ]; do
	case "$1" in
		x|-x|extract|--extract)
			extract=y;;
		u|-u|update|--update|m|-m|merge|--merge)
			mergepo="${PODIR}";;
		update=*|--update=*|merge=*|--merge=*)
			mergepo="${1#*=}";;
		U|-U|M|-M)
			shift
			mergepo="$1";;
		-h|--help|help)
			echo \
"${0##*/}: extract translatable strings from sources and update .po files

Arguments:
    x, extract    create messages template ${POT} from sources
    u, update     merge template into all translations (.po files)
    U, update=PO  merge translations under file/dir PO only
Both 'extract' and 'update' commands will be performed if called without
arguments."
			exit;;
		*)	echo "${0##*/}: unrecogized argument: $1" 1>&2
			exit 1;;
	esac
	shift
done

type extractrc xgettext msgmerge > /dev/null || exit 1

# Note: don't run "find ." since that would search .git/, too; use "find *"

if [ "${extract}" = y ]; then
	echo ' * extract messages from resources'
	# filter the output to obtain proper location comments
	# (and simplify double comment prefixes while at it)
	find * \( -name '*.rc' -o -name '*.ui' -o -name '*.kcfg' \) -type f \
		-exec extractrc '{}' + |
		xgettext ${XGETTEXT_OPTS} -o - - |
		sed -e '/^#:/ d' -e 's/^#. i18n: file:/#:/' \
		    -e 's/^#. i18n: ectx:/#. ectx:/' >| "${POT}"

	echo ' * extract messages from source files'
	find * \( -name '*.[ch]' -o -name '*.[ch]pp' \) -type f -exec \
		xgettext -j ${XGETTEXT_OPTS} -o "${POT}" '{}' +
fi

if [ -n "${mergepo}" ]; then
	# merge po files
	find "${PODIR}" -name '*.po' -type f -printf ' * update %p\n' -exec \
		msgmerge -U ${MSGMERGE_OPTS} '{}' "${POT}" \;
fi
