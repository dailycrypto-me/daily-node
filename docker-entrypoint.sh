#!/bin/bash

export DAILY_CONF_PATH=${DAILY_CONF_PATH:=/root/.daily/conf_daily.json}
export DAILY_PERSISTENT_PATH=${DAILY_PERSISTENT_PATH:=/root/.daily}
export DAILY_COPY_COREDUMPS=${DAILY_COPY_COREDUMPS:=true}
export DAILY_SLEEP_DIAGNOSE=${DAILY_SLEEP_DIAGNOSE:=false}

FLAGS=""
if [[ -z "${HOSTNAME}" ]]; then
  echo "HOSTNAME is not set."
else
  INDEX=${HOSTNAME##*-}

  ADVERTISED_IP_NAME="ADVERTISED_IP_$INDEX"
  ADVERTISED_IP=${!ADVERTISED_IP_NAME}
  if [[ -z "${ADVERTISED_IP}" ]]; then
    echo "ADVERTISED_IP is not set."
  else
    if [ "$ADVERTISED_IP" = "auto" ]; then
      ADVERTISED_IP=$(curl icanhazip.com 2>/dev/null)
    fi
    FLAGS="--public-ip ${ADVERTISED_IP}"
  fi

  ADVERTISED_PORT_NAME="ADVERTISED_PORT_$INDEX"
  ADVERTISED_PORT=${!ADVERTISED_PORT_NAME}
  if [[ -z "${ADVERTISED_PORT}" ]]; then
    echo "ADVERTISED_PORT is not set."
  else
    FLAGS="$FLAGS --port ${ADVERTISED_PORT}"
  fi
fi

case $1 in

  daily-bootnode)
    echo "Starting daily-bootnode..."
    daily-bootnode $FLAGS "${@:2}"
    ;;

  dailyd)
    echo "Starting dailyd..."
    dailyd $FLAGS "${@:2}"
    ;;

  join)
    echo "Starting dailyd..."
    dailyd $FLAGS \
            --config $DAILY_CONF_PATH \
            --chain-id $2

    ;;

  single)
	  echo "Starting dailyd..."
    dailyd $FLAGS \
            --config $DAILY_CONF_PATH

    ;;
  exec)
    exec "${@:2}"
    ;;

  *)
    echo "You should choose between:"
    echo "daily-bootnode, dailyd, single, join {NAMED_NETWOTK}"
    ;;

esac

# Hack to copy coredumps on  K8s (gke) current /proc/sys/kernel/core_pattern
if [ "$DAILY_COPY_COREDUMPS" = true ] ; then
    echo "Copying dump (if any) to $DAILY_PERSISTENT_PATH"
    find / -maxdepth 1 -type f -name '*core*' -exec cp -v "{}" $DAILY_PERSISTENT_PATH  \;
fi

# Hack to sleep forever so devs can diagnose the pod on k8s
# We should not set Liveness/Readiness for this to work
if [ "$DAILY_SLEEP_DIAGNOSE" = true ] ; then
    echo "Sleeping forever for diagnosis"
    while true
    do
        echo "Crashed. Waiting on diagnosis..."
        sleep 300
    done
fi
