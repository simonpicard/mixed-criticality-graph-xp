#!/bin/sh
set -ex

cd campaigns

entrypoint.sh
./configure.sh
./run-campaign.sh
exitpoint.sh
