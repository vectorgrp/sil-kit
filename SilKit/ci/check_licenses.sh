#! /usr/bin/env sh

# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

# Helper function to get the file list for the reuse tool
assemble_file_string()
{
    tmp=$(mktemp)
    filestring=""
    printf "%s\n" "$1" > "$tmp"
    while read line;
    do
        tmpString="$(echo "$line" | cut -d ':' -f 1)"
        filestring="$filestring $tmpString"
    done < "$tmp"

    echo $filestring
}

# Yes this is weird that we single out some directories and the include "."
# But the regex filters only work if more than one path is provided
result="$(licensecheck --check='\.c.*$|\.h.*$|\.py|CMakeLists.txt|\.sh' \
    --ignore='ThirdParty|.git|.clang-format' \
    --recursive -- ./SilKit/* ./Demos/* ./Utilities/* .)"

numfiles=$(echo "$result" | wc -l)
echo "Scanned $numfiles files"

violations=$(echo "$result" | grep "UNKNOWN")
if [ -n "$violations" ] ; then
    numviolations=$(echo "$violations" | wc -l)
    echo "FAILURE: Found $numviolations files without proper license info!"
    echo "$violations"
    mfiles="$(assemble_file_string "$violations")"
    reuseMessage=$(cat << EOM
You can add the correct SPDX header to these files by running:
reuse addheader --copyright="Vector Informatik GmbH" --license="MIT" -- $mfiles
EOM
)

    echo "$reuseMessage"
    exit 1
else
    echo "SUCCESS: All files have proper license info!"
    exit 0
fi
