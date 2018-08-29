#!/bin/bash
###########################################################################
#    astyle.sh
#    ---------------------
#    Date                 : August 2008
#    Copyright            : (C) 2008 by Juergen E. Fischer
#    Email                : jef at norbit dot de
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

if ! hash astyle 2>/dev/null; then
  echo "astyle not found - please install astyle on your system" >&2
  exit 1
fi

if ! type -p flip >/dev/null; then
  flip() {
    :
  }
fi

set -e

MY_ARTISTIC_STYLE_OPTIONS=" \
--preserve-date             \
--indent-preprocessor       \
--convert-tabs              \
--indent=spaces=2           \
--indent-classes            \
--indent-labels             \
--indent-namespaces         \
--indent-switches           \
--max-instatement-indent=40 \
--min-conditional-indent=-1 \
--suffix=none               \
--break-after-logical       \
--style=allman              \
--align-pointer=name        \
--align-reference=name      \
--keep-one-line-statements  \
--keep-one-line-blocks      \
--pad-paren-in              \
--pad-oper                  \
--unpad-paren               \
--pad-header"

for f in "$@"; do
  case "$f" in
        *.cpp|*.h|*.c|*.h|*.cxx|*.hxx|*.c++|*.h++|*.cc|*.hh|*.C|*.H|*.hpp)
                cmd="astyle $MY_ARTISTIC_STYLE_OPTIONS"
                ;;

        *.ui|*.qgm|*.txt|*.t2t|*.sip|resources/context_help/*)
                cmd=:
                ;;

        *.py)
                cmd="perl -i.prepare -pe 's/[\r\t ]+$//;'"
                ;;

        *)
                echo -ne "$f skipped $elcr"
                continue
                ;;
        esac

  if ! [ -f "$f" ]; then
    echo "$f not found" >&2
    continue
        fi

  flip -ub "$f"
  #qgsloggermig.pl "$f"
        eval "$cmd '$f'"
done
