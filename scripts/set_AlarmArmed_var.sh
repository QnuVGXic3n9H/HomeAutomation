#!/usr/bin/env bash
echo Setting AlarmArmed=$1
curl -X PUT -H "Content-Type: application/json" -d "{\"key\":\"AlarmArmed\",\"value\":\"$1\"}" "http://admin:admin@localhost:8443/mc/rest/variables"