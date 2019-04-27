#!/bin/bash

[ -n "$CGAL_DEBUG_TRAVIS" ] && set -x
DONE=0
brew update
brew install --only-dependencies cgal
exit 0

