#!/bin/bash

SERVER_PORT=8080

# start a server
echo Starting a server

SCRIPT_PATH="$(dirname -- "${BASH_SOURCE[0]}")"
$SCRIPT_PATH/../server &
server_id=$!

# check a port is open
nc -z 127.0.0.1 $SERVER_PORT
port_opend=$?

# kill a server
echo Killing a server
kill -9 $server_id

if [ $port_opend -eq 0 ]; then
	echo Port was opened, everything is fine
	exit 0
else
	echo Error! Port was closed
	exit 1
fi
