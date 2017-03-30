#!/usr/bin/env bash
echo Setting AlarmArmed=$1

if [ "$1" = "TRUE" ]; then
    script_name=Set+AlarmArmed+ON.groovy
else
    script_name=Set+AlarmArmed+OFF.groovy
fi

curl "http://admin:admin@localhost:8443/mc/rest/scripts/runNow?bindings=%7B+%7D&script=operations%2F$script_name&scriptBindings=%7B%7D"