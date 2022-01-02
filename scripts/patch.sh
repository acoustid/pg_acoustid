#!/usr/bin/env bash
set -eux
cd $(dirname $0)/..
(cd expected && patch -p$(pwd | awk -F/ '{ print NF }')) < regression.diffs
