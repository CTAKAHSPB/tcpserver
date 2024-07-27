#!/bin/bash

SERVER_PORT=8080
MESSAGE="test message"

# start a server
echo Starting a server

SCRIPT_PATH="$(dirname -- "${BASH_SOURCE[0]}")"
$SCRIPT_PATH/../server &
server_id=$!
sleep 1

# running a client
$SCRIPT_PATH/../client "$MESSAGE" | grep "From server: $MESSAGE"
echo_received=$?

# kill a server
echo Killing a server
kill -9 $server_id

if [ $echo_received -eq 0 ]; then
	echo Echo was received from a server
	exit 0
else
	echo Failed to receive echo responce from a server
	exit 1
fi
