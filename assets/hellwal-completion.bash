#!/usr/bin/env bash

_hellwal_completion() {
    local cur prev opts
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"

    opts="--image -i --dark -d --light -l --quiet -q --script -s --random -r \
--template-folder -f --output -o --theme -t --theme-folder -k --help -h"

    case "${prev}" in
        --image|-i|--template-folder|-f|--output|-o|--theme|-t|--theme-folder|-k)
            # Complete file paths
            COMPREPLY=( $(compgen -f -- "${cur}") )
            return 0
            ;;
        *)
            # Complete options
            COMPREPLY=( $(compgen -W "${opts}" -- "${cur}") )
            return 0
            ;;
    esac
}

complete -F _hellwal_completion ./hellwal
