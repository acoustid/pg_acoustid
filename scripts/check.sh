#!/usr/bin/env bash
cd $(dirname $0)/..
make && sudo make install && (sudo make installcheck PGUSER=postgres || cat regression.diffs)
