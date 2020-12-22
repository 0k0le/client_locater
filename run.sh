#!/bin/sh

PROJECTNAME=client_locater
PROJECTDIR=$HOME/Projects/c/$PROJECTNAME

# Run debug executable
$PROJECTDIR/bin/release/$PROJECTNAME $*
