#!/bin/sh

PROJECTNAME=client_locater
PROJECTDIR=$HOME/Projects/c/$PROJECTNAME

# Run debug executable
valgrind --leak-check=full --show-leak-kinds=all --log-file="log.valg" $PROJECTDIR/bin/debug/$PROJECTNAME $*
