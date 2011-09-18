#!/bin/sh

# dirname(__FILE__)
PWD="$(dirname $(readlink $0 || echo $0))"

HOSTNAME=${1-localhost}

# forward SC udp ports; responses to UDP bcast will be UDP unicast
${PWD}/udp-proxy 6111 $HOSTNAME
${PWD}/udp-proxy 6112 $HOSTNAME
