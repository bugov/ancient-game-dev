#!/bin/bash

function build {
    echo "=========================== BUILD ==============================="
    printf -- "\033[33m[!] Build: $1\033[0m\n"
    clang \
        -DDEBUG \
        -I./inc \
        $1 \
        ./inc/*.c \
        -o a.out \
        -lSDL2 \
        -lSDL2_image \
        -lSDL2_ttf
    ret=$?
    echo "=========================== END BUILD ==========================="
    return $ret
}

function run_test {
    echo "=========================== RUN ================================="
    ./a.out
    ret=$?

    if [[ "$ret" == "0" ]]
    then
        printf -- "\033[32m[+] $1\033[0m\n"
    else
        printf -- "\033[31m[-] $1\033[0m\n"
    fi

    echo "=========================== END RUN ============================="
    return $ret
}

function build_and_test {
    rm a.out
    build $1 && run_test $1
    return $?
}

function command_test {
    build_and_test "./test/test_sdlike.c" \
        && sleep 1 && build_and_test "./test/test_level.c" \
        && printf -- "\033[32m[+] All fine\033[0m\n" \
        || printf -- "\033[31m[-] An error occured\033[0m\n"
}

function command_main {
    rm a.out
    build ./main.c && ./a.out
}

case $1 in
    test)
        command_test
        ;;
    *)
        command_main
        ;;
esac

